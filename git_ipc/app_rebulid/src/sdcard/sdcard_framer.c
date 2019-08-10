
#include "sdcard_framer.h"
#include "frank_trace.h"
#include "ezxml.h"

#define kSD_FRAMER_MAGIC "FRANK "
#define kSD_FRAMER_VER "1.0"


static int framer_media_magic(char *result, int xmlHeadLength)
{
	char *const magic = result;
	char *const xmlLengthMSB = result + strlen(kSD_FRAMER_MAGIC);
	char *const xmlLengthLSB = result + strlen(kSD_FRAMER_MAGIC) + 1;

	// add magic
	memcpy(magic, kSD_FRAMER_MAGIC, strlen(kSD_FRAMER_MAGIC));

	// add XML length
	*xmlLengthMSB = xmlHeadLength / 256;
	*xmlLengthLSB = xmlHeadLength % 256;

	return 0;
}

int SDCARD_framer_make(LP_SDCARD_FRAMER framer, char *result, int resultMax)
{
	char *const xmlBuf = result + strlen(kSD_FRAMER_MAGIC) + 2; // with two byte XML length indicator
	int xmlBufMax = resultMax - (xmlBuf - result);
	int xmlLength = 0;
	int framerLength = -1;
	
	if(kSDF_TYPE_VIDEO == framer->type){
		xmlLength = snprintf(xmlBuf, xmlBufMax,
			"<SDCard version=\"%s\">"
				"<frame type=\"video\" dataLength=\"%d\" timestampMS=\"%d\" timeS=\"%d\">"
					"<sessionID>%d</sessionID>"
					"<channelID>%d</channelID>"
					"<channelSubID>%d</channelSubID>"
					"<codec>H.264</codec>"
					"<profile>%s</profile>"
					"<resolutionWidth>%d</resolutionWidth>"
					"<resolutionHeight>%d</resolutionHeight>"
					"<frameRate>%d</frameRate>"
					"<keyFrame>%s</keyFrame>"
				"</frame>"
			"</SDCard>",
			kSD_FRAMER_VER,
			framer->dataLength, // // data length
			framer->timestampMS, // frame timestamp
			framer->timeS,
			framer->video.sessionID,
			framer->video.channelID, framer->video.channelSubID, // video channel ID
			"baseline", // h.264 profile
			framer->video.resolutionWidth, // resolution width
			framer->video.resolutionHeight, // resoulution height
			framer->video.frameRate,
			framer->video.keyFrame? "true" : "false");

		_ASSERT(xmlLength < xmlBufMax, "Framer H264 XML size is too large!");

	}else if(kSDF_TYPE_AUDIO == framer->type){		
		xmlLength = snprintf(xmlBuf, xmlBufMax,
			"<SDCard version=\"%s\">"
				"<frame type=\"audio\" dataLength=\"%d\" timestampMS=\"%d\"  timeS=\"%d\">"
					"<sessionID>%d</sessionID>"
					"<channelID>%d</channelID>"
					"<codec>G.711Alaw</codec>"
					"<sampleRate>%d</sampleRate>"
					"<sampleBitWidth>%d</sampleBitWidth>"
				"</frame>"
			"</SDCard>",
			kSD_FRAMER_VER,
			framer->dataLength, // // data length
			framer->timestampMS, // frame timestamp
			framer->timeS,
			framer->audio.sessionID,
			framer->audio.channelID,
			framer->audio.sampleRate, framer->audio.sampleBitWidth);

		_ASSERT(xmlLength < xmlBufMax, "Framer AUDIO XML size is too large!");

	}else if(kSDF_TYPE_PIC == framer->type){
		xmlLength = snprintf(xmlBuf, xmlBufMax,
			"<SDCard version=\"%s\">"
				"<frame type=\"picture\" dataLength=\"%d\" timestampMS=\"%d\"  timeS=\"%d\">"
					"<sessionID>%d</sessionID>"
					"<channelID>%d</channelID>"
					"<codec>JPEG</codec>"
					"<resolutionWidth>%d</resolutionWidth>"
					"<resolutionHeight>%d</resolutionHeight>"
				"</frame>"
			"</SDCard>",			
			kSD_FRAMER_VER,
			framer->dataLength, // // data length
			framer->timestampMS, // frame timestamp
			framer->timeS,
			framer->video.sessionID,
			framer->video.channelID,
			framer->video.resolutionWidth, // resolution width
			framer->video.resolutionHeight);

		_ASSERT(xmlLength < xmlBufMax, "Framer PICTURE XML size is too large!");

	}else if(kSDF_TYPE_EOF == framer->type){
		xmlLength = snprintf(xmlBuf, xmlBufMax,
			"<SDCard version=\"%s\">"
				"<frame type=\"endOfFile\" dataLength=\"0\" timestampMS=\"%d\"/  timeS=\"%d\">"
			"</SDCard>",
			kSD_FRAMER_VER,
			framer->timestampMS,
			framer->timeS);

		//_TRACE("EOF: %s", xmlBuf);
	}

	if(xmlLength > 0){
		char *const magic = result;
		char *const xmlLengthMSB = result + strlen(kSD_FRAMER_MAGIC);
		char *const xmlLengthLSB = result + strlen(kSD_FRAMER_MAGIC) + 1;

		// add magic
		memcpy(magic, kSD_FRAMER_MAGIC, strlen(kSD_FRAMER_MAGIC));

		// add XML length
		*xmlLengthMSB = xmlLength / 256;
		*xmlLengthLSB = xmlLength % 256;

		//_TRACE("MEDIA XML:\r\n%s", xmlBuf);

		return (xmlBuf - result) + xmlLength;
	}
	
	return -1;
}

int sdcard_framer_parse(char *textXML, int textXMLLength, LP_SDCARD_FRAMER result)
{
	ezxml_t nodeSDCard = ezxml_parse_str(textXML, textXMLLength);
	if(NULL != nodeSDCard){
		ezxml_t nodeFrame = ezxml_child(nodeSDCard, "frame");
		if(NULL != nodeFrame){
			// read attributes
			const char *attrType = ezxml_attr(nodeFrame, "type");
			const char *attrDataLength = ezxml_attr(nodeFrame, "dataLength");
			const char *attrTimeStampMS = ezxml_attr(nodeFrame, "timestampMS");

			if(NULL != attrType && NULL != attrDataLength && NULL != attrTimeStampMS){
				memset(result, 0, sizeof(ST_SDCARD_FRAMER));

				result->dataLength = atoi(attrDataLength);
				result->timestampMS = atoi(attrTimeStampMS);
				
				if(0 == strcmp(attrType, "video")){
					// video framer
					ezxml_t nodeSessionID = ezxml_child(nodeFrame, "sessionID");
					ezxml_t nodeChannelID = ezxml_child(nodeFrame, "channelID");
					ezxml_t nodeChannelSubID = ezxml_child(nodeFrame, "channelSubID");
					ezxml_t nodeCodec = ezxml_child(nodeFrame, "codec");
					ezxml_t nodeProfile = ezxml_child(nodeFrame, "profile");
					ezxml_t nodeResolutionWidth = ezxml_child(nodeFrame, "resolutionWidth");
					ezxml_t nodeResolutionHeight = ezxml_child(nodeFrame, "resolutionHeight");
					ezxml_t nodeFrameRate = ezxml_child(nodeFrame, "frameRate");
					ezxml_t nodeKeyFrame = ezxml_child(nodeFrame, "keyFrame");
					
					result->type = kSDF_TYPE_VIDEO;
					result->video.sessionID = atoi(ezxml_txt(nodeSessionID));
					result->video.channelID = atoi(ezxml_txt(nodeChannelID));
					result->video.channelSubID = atoi(ezxml_txt(nodeChannelSubID));
					result->video.codec = kSDF_V_CODEC_H264;
					result->video.profile = kSDF_V_PROFILE_BASELINE;
					result->video.resolutionWidth = atoi(ezxml_txt(nodeResolutionWidth));
					result->video.resolutionHeight = atoi(ezxml_txt(nodeResolutionHeight));
					result->video.frameRate = atoi(ezxml_txt(nodeFrameRate));
					result->video.keyFrame = (0 == strcmp(ezxml_txt(nodeKeyFrame), "true")) ? true : false;

				}else if(0 == strcmp(attrType, "audio")){
					// audio framer
					ezxml_t nodeSessionID = ezxml_child(nodeFrame, "sessionID");
					ezxml_t nodeChannelID = ezxml_child(nodeFrame, "channelID");
					ezxml_t nodeCodec = ezxml_child(nodeFrame, "codec");
					ezxml_t nodeSampleRate = ezxml_child(nodeFrame, "sampleRate");
					ezxml_t nodeSampleBitWidth = ezxml_child(nodeFrame, "sampleBitWidth");

					result->type = kSDF_TYPE_AUDIO;
					result->audio.sessionID = atoi(ezxml_txt(nodeSessionID));
					result->audio.channelID = atoi(ezxml_txt(nodeChannelID));
					result->audio.codec = kSDF_A_CODEC_G711A;
					result->audio.sampleRate = atoi(ezxml_txt(nodeSampleRate));
					result->audio.sampleBitWidth = atoi(ezxml_txt(nodeSampleBitWidth));

				}else if(0 == strcmp(attrType, "picture")){
					// video framer
					ezxml_t nodeSessionID = ezxml_child(nodeFrame, "sessionID");
					ezxml_t nodeChannelID = ezxml_child(nodeFrame, "channelID");
					ezxml_t nodeCodec = ezxml_child(nodeFrame, "codec");
					ezxml_t nodeResolutionWidth = ezxml_child(nodeFrame, "resolutionWidth");
					ezxml_t nodeResolutionHeight = ezxml_child(nodeFrame, "resolutionHeight");
					
					result->type = kSDF_TYPE_PIC;
					result->picture.sessionID = atoi(ezxml_txt(nodeSessionID));
					result->picture.channelID = atoi(ezxml_txt(nodeChannelID));
					result->picture.codec = kSDF_P_CODEC_JPEG;
					result->picture.resolutionWidth = atoi(ezxml_txt(nodeResolutionWidth));
					result->picture.resolutionHeight = atoi(ezxml_txt(nodeResolutionHeight));

				}else if(0 == strcmp(attrType, "endOfFile")){
					// eof framer
					result->type = kSDF_TYPE_EOF;

				}else{
					ezxml_free(nodeSDCard);
					nodeSDCard = NULL;
					return -1;
				}

				ezxml_free(nodeSDCard);
				nodeSDCard = NULL;
				return 0;
			}
		}
		ezxml_free(nodeSDCard);
		nodeSDCard = NULL;
	}
	return -1;
}


int SDCARD_framer_parse(int fID, LP_SDCARD_FRAMER result, char *frame, int frameMax)
{
	int readN = 0;
	char framerBuf[2048];
	char msb = 0, lsb = 0;
	int xmlLength = 0;
	int checkSum = 0;

	// read magic
	readN = read(fID, framerBuf, strlen(kSD_FRAMER_MAGIC));
	if(strlen(kSD_FRAMER_MAGIC) != readN){
		_TRACE("Read Magic Error!");
		return -1;
	}
	// check magic
	if(0 != memcmp(framerBuf, kSD_FRAMER_MAGIC, strlen(kSD_FRAMER_MAGIC))){
		_TRACE("Magic Error!");
		return -1;
	}

	// read data length
	if(1 != read(fID, &msb, 1)){
		_TRACE("Read MSB Error!");
		return -1;
	}
	if(1 != read(fID, &lsb, 1)){
		_TRACE("Read LSB Error!");
		return -1;
	}

	xmlLength = (int)msb;
	xmlLength *= 256;
	xmlLength += (int)lsb;
	//_TRACE("XML Length %d!", xmlLength);

	// read xml
	readN = read(fID, framerBuf, xmlLength);
	if(xmlLength != readN){
		_TRACE("Read XML Framer Error!");
		return -1;
	}
	// parse xml header
	if(0 != sdcard_framer_parse(framerBuf, xmlLength, result)){
		_TRACE("Parse XML Framer Error!");
		return -1;
	}

	// read data from file
	_ASSERT(result->dataLength < frameMax, "Framer Parser Max=%d Not Enough", frameMax);
	readN = read(fID, frame, result->dataLength);
	if(result->dataLength != readN){
		_TRACE("Read Payload Data Error!");
		return -1;
	}

	//_TRACE("Frame Paser a Frame Length = %d", result->dataLength);
	return strlen(kSD_FRAMER_MAGIC) + 2 + xmlLength + result->dataLength;
}




