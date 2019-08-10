// ITU-T H.222.0(06/2012)
// Information technology �C Generic coding of moving pictures and associated audio information: Systems
// 2.5.3.1 Program stream(p74)

#include <stdio.h>
#include <stdlib.h>
#include "mpeg_ps.h"
#include "mpeg_ps_proto.h"
#include "mpeg_ts_proto.h"
#include "mpeg_pes_proto.h"
#include <assert.h>
#include <string.h>

struct ps_demuxer_t
{
    struct psm_t psm;
    struct psd_t psd;

    struct ps_pack_header_t pkhd;
    struct ps_system_header_t system;

    ps_dumuxer_onpacket onpacket;
	void* param;	
};

static struct pes_t* psm_fetch(struct psm_t* psm, uint8_t sid)
{
    size_t i;
    for (i = 0; i < psm->stream_count; ++i)
    {
        if (psm->streams[i].sid == sid)
            return &psm->streams[i];
    }

    if (psm->stream_count < sizeof(psm->streams) / sizeof(psm->streams[0]))
    {
        // guess stream codec id
        if (0xE0 <= sid && sid <= 0xEF)
            psm->streams[psm->stream_count].codecid = PSI_STREAM_H264;
        else if(0xC0 <= sid && sid <= 0xDF)
            psm->streams[psm->stream_count].codecid = PSI_STREAM_AAC;

        return &psm->streams[psm->stream_count++];
    }

    return NULL;
}

static size_t pes_packet_read(struct ps_demuxer_t *ps, const uint8_t* data, size_t bytes)
{
    size_t i = 0;
    size_t j = 0;
    size_t pes_packet_length;
    struct pes_t* pes;

    // MPEG_program_end_code = 0x000000B9
    for (i = 0; i + 5 < bytes && 0x00 == data[i] && 0x00 == data[i + 1] && 0x01 == data[i + 2]
        && PES_SID_END != data[i + 3]
        && PES_SID_START != data[i + 3];
        i += pes_packet_length + 6) 
    {
        pes_packet_length = (data[i + 4] << 8) | data[i + 5];
        assert(i + 6 + pes_packet_length <= bytes);
        if (i + 6 + pes_packet_length > bytes)
            return 0;

        // stream id
        switch (data[i+3])
        {
        case PES_SID_PSM:
            j = psm_read(&ps->psm, data + i, bytes - i);
            assert(j == pes_packet_length + 6);
            break;

        case PES_SID_PSD:
            j = psd_read(&ps->psd, data + i, bytes - i);
            assert(j == pes_packet_length + 6);
            break;

        case PES_SID_PRIVATE_2:
        case PES_SID_ECM:
        case PES_SID_EMM:
        case PES_SID_DSMCC:
        case PES_SID_H222_E:
            // stream data
            break;

        case PES_SID_PADDING:
            // padding
            break;

		// ffmpeg mpeg.c mpegps_read_pes_header
		//case 0x1c0:
		//case 0x1df:
		//case 0x1e0:
		//case 0x1ef:
		//case 0x1bd:
		//case 0x01fd:
		//	break;

        default:
            pes = psm_fetch(&ps->psm, data[i+3]);
            if (NULL == pes)
                continue;

            assert(PES_SID_END != data[i + 3]);
            j = pes_read_header(pes, data + i, bytes - i);
            if (0 == j) continue;

            pes_packet(&pes->pkt, pes, data + i + j, pes_packet_length + 6 - j, ps->onpacket, ps->param);
        }
    }

    return i;
}

size_t ps_demuxer_input(struct ps_demuxer_t* ps, const uint8_t* data, size_t bytes)
{
	size_t i, n;
	
    for (i = 0; i + 3 < bytes && 0x00 == data[i] && 0x00 == data[i + 1] && 0x01 == data[i + 2]; )
    {
        if (PES_SID_START == data[i + 3])
        {
            i += pack_header_read(&ps->pkhd, data + i, bytes - i);
        }
        else if (PES_SID_SYS == data[i + 3])
        {
            i += system_header_read(&ps->system, data + i, bytes - i);
        }
        else if (PES_SID_END == data[i + 3])
        {
            i += 4;
        }
        else
        {
            n = pes_packet_read(ps, data + i, bytes - i);
            i += n;

            if (0 == n)
                break;
        }
    }

	return i;
}

struct ps_demuxer_t* ps_demuxer_create(ps_dumuxer_onpacket onpacket, void* param)
{
	struct ps_demuxer_t* ps;
	ps = calloc(1, sizeof(struct ps_demuxer_t));
	if(!ps)
		return NULL;

    ps->onpacket = onpacket;
	ps->param = param;
	return ps;
}

int ps_demuxer_destroy(struct ps_demuxer_t* ps)
{
    size_t i;
    struct pes_t* pes;
    for (i = 0; i < ps->psm.stream_count; i++)
    {
        pes = &ps->psm.streams[i];
        if (pes->pkt.data)
            free(pes->pkt.data);
        pes->pkt.data = NULL;
    }

	free(ps);
	return 0;
}
