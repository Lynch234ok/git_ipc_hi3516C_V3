#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <app_debug.h>
#include <stdlib.h>
#include "cycle_buffer.h"



// |(buf)--------(data_start)+++++++++++(data_end)----------|
// |(buf)+++++++(data_end)------------(data_start)++++++++++|
typedef struct {

    uint8_t *buf;
    uint8_t *data_start;
    uint8_t *data_end;

    size_t buf_len;
    size_t data_len;

    pthread_mutex_t lock;

} stCycleBuffer, *lpCycleBuffer;


/// helpers
static int cycle_buffer_helpers_read(lpCycleBuffer pCycleBuffer,
                                     uint8_t *out_buf,
                                     size_t out_data_len)
{
    ssize_t from_start_pos_buf_len;
    ssize_t data_start_pos_offset;

    if (pCycleBuffer->data_len < out_data_len) {
        APP_TRACE("%s: out_data_len %u too large, data length: %u",
                  __FUNCTION__, out_data_len, pCycleBuffer->data_len);
        return -1;
    }

    data_start_pos_offset = pCycleBuffer->data_start - pCycleBuffer->buf;
    from_start_pos_buf_len = pCycleBuffer->buf_len - data_start_pos_offset;

    if (from_start_pos_buf_len < 0) {
        APP_TRACE("%s: error calculate data len from data start pos!", __FUNCTION__);
        return -1;
    }

    if (from_start_pos_buf_len < out_data_len) {
        memcpy(out_buf, pCycleBuffer->data_start, from_start_pos_buf_len);
        memcpy(out_buf + from_start_pos_buf_len,
               pCycleBuffer->buf,
               out_data_len - from_start_pos_buf_len);
        pCycleBuffer->data_start = pCycleBuffer->buf + (out_data_len - from_start_pos_buf_len);
        pCycleBuffer->data_len -= out_data_len;
    } else {
        memcpy(out_buf, pCycleBuffer->data_start, out_data_len);
        pCycleBuffer->data_start += out_data_len;
        pCycleBuffer->data_len -= out_data_len;
    }

    return 0;
}

static int cycle_buffer_helpers_skip(lpCycleBuffer pCycleBuffer,
                                     size_t skip_data_len)
{
    ssize_t from_start_pos_buf_len;
    ssize_t data_start_pos_offset;

    if (pCycleBuffer->data_len < skip_data_len) {
        APP_TRACE("%s: out_data_len %u too large, data length: %u",
                  __FUNCTION__, skip_data_len, pCycleBuffer->data_len);
        return -1;
    }

    data_start_pos_offset = pCycleBuffer->data_start - pCycleBuffer->buf;
    from_start_pos_buf_len = pCycleBuffer->buf_len - data_start_pos_offset;

    if (from_start_pos_buf_len < 0) {
        APP_TRACE("%s: error calculate data len from data start pos!", __FUNCTION__);
        return -1;
    }

    if (from_start_pos_buf_len < skip_data_len) {
        pCycleBuffer->data_start = pCycleBuffer->buf + (skip_data_len - from_start_pos_buf_len);
        pCycleBuffer->data_len -= skip_data_len;
    } else {
        pCycleBuffer->data_start += skip_data_len;
        pCycleBuffer->data_len -= skip_data_len;
    }

    return 0;
}

static int cycle_buffer_helpers_peek(lpCycleBuffer pCycleBuffer,
                                     uint8_t *out_buf,
                                     size_t out_data_len)
{
    ssize_t from_start_pos_buf_len;
    ssize_t data_start_pos_offset;

    if (pCycleBuffer->data_len < out_data_len) {
        APP_TRACE("%s: out_data_len %u too large, data length: %u",
                  __FUNCTION__, out_data_len, pCycleBuffer->data_len);
        return -1;
    }

    data_start_pos_offset = pCycleBuffer->data_start - pCycleBuffer->buf;
    from_start_pos_buf_len = pCycleBuffer->buf_len - data_start_pos_offset;

    if (from_start_pos_buf_len < 0) {
        APP_TRACE("%s: error calculate data len from data start pos!", __FUNCTION__);
        return -1;
    }

    if (from_start_pos_buf_len < out_data_len) {
        memcpy(out_buf, pCycleBuffer->data_start, from_start_pos_buf_len);
        memcpy(out_buf + from_start_pos_buf_len,
               pCycleBuffer->buf,
               out_data_len - from_start_pos_buf_len);
    } else {
        memcpy(out_buf, pCycleBuffer->data_start, out_data_len);
    }

    return 0;
}

static int cycle_buffer_helpers_write(lpCycleBuffer pCycleBuffer,
                                      uint8_t *in_buf,
                                      size_t in_data_len)
{
    ssize_t from_end_pos_buf_len;
    ssize_t data_end_pos_offset;

    if (pCycleBuffer->buf_len < pCycleBuffer->data_len + in_data_len) {
        APP_TRACE("%s: in data %u too large, data length: %u, buf length: %u",
                  __FUNCTION__, in_data_len, pCycleBuffer->data_len, pCycleBuffer->buf_len);
        return -1;
    }

    data_end_pos_offset = pCycleBuffer->data_end - pCycleBuffer->buf;
    from_end_pos_buf_len = pCycleBuffer->buf_len - data_end_pos_offset;

    if (from_end_pos_buf_len < 0) {
        APP_TRACE("%s: error calculate data len from data end pos!", __FUNCTION__);
        return -1;
    }

    if (from_end_pos_buf_len < in_data_len) {
        memcpy(pCycleBuffer->data_end, in_buf, from_end_pos_buf_len);
        memcpy(pCycleBuffer->buf,
               in_buf + from_end_pos_buf_len,
               in_data_len - from_end_pos_buf_len);
        pCycleBuffer->data_end = pCycleBuffer->buf + (in_data_len - from_end_pos_buf_len);
        pCycleBuffer->data_len += in_data_len;
    } else {
        memcpy(pCycleBuffer->data_end, in_buf, in_data_len);
        pCycleBuffer->data_end += in_data_len;
        pCycleBuffer->data_len += in_data_len;
    }

    return 0;
}


/// interfaces
H_CYCLE_BUFFER CYCLE_BUFFER_new(size_t data_buf_sz)
{
    int ret;
    lpCycleBuffer pCycleBuffer = NULL;
    size_t st_size = 0;

    if (data_buf_sz > kCYCLE_BUFFER_MAX_SIZE) {
        APP_TRACE("%s: buffer size too large. size want to create: %u, max allowed: %lu",
                  __FUNCTION__, data_buf_sz, kCYCLE_BUFFER_MAX_SIZE);
        return NULL;
    }

    st_size = sizeof(stCycleBuffer);
    pCycleBuffer = malloc(st_size);
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: failed to malloc %u bytes for cycle buffer struct!",
                  __FUNCTION__, st_size);
        goto FAIL_RETURN;
    }


    pCycleBuffer->buf = malloc(data_buf_sz + 1);
    if (NULL == pCycleBuffer->buf) {
        APP_TRACE("%s: failed to malloc %u bytes for data buffer!",
                  __FUNCTION__, data_buf_sz);
        goto FAIL_RETURN;
    }


    ret = pthread_mutex_init(&pCycleBuffer->lock, NULL);
    if (0 != ret) {
        APP_TRACE("%s: pthread_mutex_init failed!!", __FUNCTION__);
        goto FAIL_RETURN;
    }

    pCycleBuffer->buf_len = data_buf_sz;
    pCycleBuffer->data_start = pCycleBuffer->buf;
    pCycleBuffer->data_end = pCycleBuffer->buf;
    pCycleBuffer->data_len = 0;

    return (H_CYCLE_BUFFER)pCycleBuffer;


FAIL_RETURN:

    if (NULL != pCycleBuffer) {

        if (NULL != pCycleBuffer->buf) {
            free(pCycleBuffer->buf);
        }

        free(pCycleBuffer);
    }

    return NULL;
}


int CYCLE_BUFFER_free(H_CYCLE_BUFFER hCycleBuffer)
{
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }

    if (NULL != pCycleBuffer->buf) {
        free(pCycleBuffer->buf);
    }

    free(pCycleBuffer);

    return 0;
}

// get Cycle Buffer size
int CYCLE_BUFFER_get_buf_sz(H_CYCLE_BUFFER hCycleBuffer, size_t *p_out_size)
{
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == p_out_size) {
        APP_TRACE("%s: p_out_size can't be NULL!", __FUNCTION__);
        return -1;
    }

    *p_out_size = pCycleBuffer->buf_len;

    return 0;
}

// get data size in Cycle Buffer
int CYCLE_BUFFER_get_data_sz(H_CYCLE_BUFFER hCycleBuffer, size_t *p_out_size)
{
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == p_out_size) {
        APP_TRACE("%s: p_out_size can't be NULL!", __FUNCTION__);
        return -1;
    }

    *p_out_size = pCycleBuffer->data_len;

    return 0;
}

// out_buf can't be NULL
int CYCLE_BUFFER_read(H_CYCLE_BUFFER hCycleBuffer,
                      uint8_t *out_buf,
                      size_t out_data_len)
{
    int ret;
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == out_buf) {
        APP_TRACE("%s: out_buf can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&pCycleBuffer->lock);

    ret = cycle_buffer_helpers_read(pCycleBuffer, out_buf, out_data_len);
    if (0 != ret) {
        APP_TRACE("%s: Failed to read data!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_unlock(&pCycleBuffer->lock);

    return 0;
}
// out_buf can't be NULL
int CYCLE_BUFFER_peek(H_CYCLE_BUFFER hCycleBuffer,
                      uint8_t *out_buf,
                      size_t out_data_len)
{
    int ret;
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == out_buf) {
        APP_TRACE("%s: out_buf can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&pCycleBuffer->lock);

    ret = cycle_buffer_helpers_peek(pCycleBuffer, out_buf, out_data_len);
    if (0 != ret) {
        APP_TRACE("%s: Failed to peek data!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_unlock(&pCycleBuffer->lock);

    return 0;
}

int CYCLE_BUFFER_skip(H_CYCLE_BUFFER hCycleBuffer,
                      size_t skip_data_len)
{
    int ret;
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&pCycleBuffer->lock);

    ret = cycle_buffer_helpers_skip(pCycleBuffer, skip_data_len);
    if (0 != ret) {
        APP_TRACE("%s: Failed to read data!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_unlock(&pCycleBuffer->lock);

    return 0;
}

// in_buf can't be NULL
int CYCLE_BUFFER_write(H_CYCLE_BUFFER hCycleBuffer,
                       uint8_t *in_buf,
                       size_t in_data_len)
{
    int ret;
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == in_buf) {
        APP_TRACE("%s: in_buf can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&pCycleBuffer->lock);

    ret = cycle_buffer_helpers_write(pCycleBuffer, in_buf, in_data_len);
    if (0 != ret) {
        APP_TRACE("%s: Failed to write data!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_unlock(&pCycleBuffer->lock);

    return 0;
}

int CYCLE_BUFFER_clear(H_CYCLE_BUFFER hCycleBuffer)
{
    int ret;
    lpCycleBuffer pCycleBuffer = (lpCycleBuffer)hCycleBuffer;
    if (NULL == pCycleBuffer) {
        APP_TRACE("%s: handler of cycle buffer can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&pCycleBuffer->lock);

    pCycleBuffer->data_start = pCycleBuffer->buf;
    pCycleBuffer->data_end = pCycleBuffer->buf;
    pCycleBuffer->data_len = 0;

    pthread_mutex_unlock(&pCycleBuffer->lock);

    return 0;
}
