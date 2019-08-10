#ifndef GIT_IPC_CYCLE_BUFFER_H
#define GIT_IPC_CYCLE_BUFFER_H

#include <stdint.h>


#define kCYCLE_BUFFER_MAX_SIZE 204800LU

typedef void * H_CYCLE_BUFFER;

H_CYCLE_BUFFER CYCLE_BUFFER_new(size_t data_buf_sz);
int CYCLE_BUFFER_free(H_CYCLE_BUFFER hCycleBuffer);

// get CYCLE_BUFFER size
int CYCLE_BUFFER_get_buf_sz(H_CYCLE_BUFFER hCycleBuffer, size_t *p_out_size);
// get data size in CYCLE_BUFFER
int CYCLE_BUFFER_get_data_sz(H_CYCLE_BUFFER hCycleBuffer, size_t *p_out_size);

// out_buf can't be NULL
int CYCLE_BUFFER_read(H_CYCLE_BUFFER hCycleBuffer,
                      uint8_t *out_buf,
                      size_t out_data_len);
// out_buf can't be NULL
int CYCLE_BUFFER_peek(H_CYCLE_BUFFER hCycleBuffer,
                      uint8_t *out_buf,
                      size_t out_data_len);

int CYCLE_BUFFER_skip(H_CYCLE_BUFFER hCycleBuffer,
                      size_t skip_data_len);

// in_buf can't be NULL
int CYCLE_BUFFER_write(H_CYCLE_BUFFER hCycleBuffer,
                       uint8_t *in_buf,
                       size_t in_data_len);

int CYCLE_BUFFER_clear(H_CYCLE_BUFFER hCycleBuffer);

#endif //GIT_IPC_CYCLE_BUFFER_H
