#if defined(BLUETOOTH)



#include <zconf.h>

#ifndef GIT_IPC_BLUETOOTH_H
#define GIT_IPC_BLUETOOTH_H


/**
 * Serial command.
 *
 * Every command should be followed by \r\n
 *
 * IPC                      Bluetooth module
 * (Get bluetooth name)
 * AT+BTNAME=?\r\n    -->
 *                    <--   RES+NAME=XXX\r\n/RES+FAIL\r\n
 *
 * (Change bluetooth name)
 * AT+BTNAME=XXX\r\n  -->
 *                    <--   RES+OK\r\n/RES+FAIL\r\n
 *
 * (Open bluetooth hotspot)
 * AT+WAKE\r\n        -->
 *                    <--   RES+OK\r\n/RES+FAIL\r\n
 *
 * (Close bluetooth hotspot)
 * AT+SLEEP\r\n       -->
 *                    <--   RES+OK\r\n/RES+FAIL\r\n
 *
 */


/**
 * Bluetooth functions call order:
 *
 * BT_init()
 * -> BT_open_hotspot()
 * -> BT_get_recvbuf_data_sz()/BT_read()/BT_peek()/BT_skip()/BT_write()
 * -> BT_close_hotspot()
 * -> BT_deinit()
 *
 */

// hotspot_name can't be NULL
int BT_init( void );
int BT_deinit( void );
extern bool BT_is_init();

int BT_open_hotspot(char *hotspot_name);
int BT_close_hotspot( void );
extern bool BT_is_hotspot_on();

ssize_t BT_get_recvbuf_data_sz( void );

int BT_read(uint8_t *p_data, size_t read_size);
int BT_peek(uint8_t *p_data, size_t peek_size);
int BT_skip(size_t skip_size);

int BT_write(uint8_t *p_data, size_t write_size);


#endif //GIT_IPC_BLUETOOTH_H



#endif //defined(BLUETOOTH)