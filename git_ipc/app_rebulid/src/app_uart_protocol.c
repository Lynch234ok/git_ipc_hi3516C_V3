#include "uart_protocol.h"
#include "netsdk.h"

fUART_CALLBACK uart_callback(int cmd, DATA* data)
{
	unsigned int ircut_mode;
	ST_NSDK_VIN_CH vin_ch;
	NETSDK_conf_vin_ch_get(1, &vin_ch);
	switch(cmd){
		case UART_CMD_READ_BRIGHT:
			data->dataBase = vin_ch.brightnessLevel*255/100;		
			break;
		case UART_CMD_READ_SHARPENESS:
			data->dataBase = 128;
			break;
		case UART_CMD_READ_SHUTTER:
			data->dataBase = 0;
			break;
		case UART_CMD_READ_MIRROR:
			data->dataBase = (vin_ch.flip?1:0)<<1 | (vin_ch.mirror?1:0);
			break;
		case UART_CMD_SET_AF:
			break;
		case UART_CMD_SET_SATURATION:
			ircut_mode = data->dataBase; //0:daytime 1:night
//			printf("\033[33m   ircut_mode = %d   \033[0m \n",ircut_mode);
			ircut_mode++;
			SENSOR_ircut_mode_set(ircut_mode);
			break;
		case UART_CMD_SET_BRIGHT:
			vin_ch.brightnessLevel = data->dataBase*100/255;
			NETSDK_conf_vin_ch_set(1, &vin_ch);
			break;
		case UART_CMD_SET_SHARPENESS:
			break;
		case UART_CMD_SET_SHUTTER:
			break;
		case UART_CMD_SET_MIRROR:
			vin_ch.flip = (data->dataBase & 0x2)? true:false;
			vin_ch.mirror= (data->dataBase & 0x1)? true:false;
			NETSDK_conf_vin_ch_set(1, &vin_ch);
			break;
		case UART_CMD_SET_STANDARD_OSD:
			APP_OVERLAY_table_info_add(data->osd.line, data->osd.cow,data->osd.string);
			APP_OVERLAY_table_info_update();
			break;
		case UART_CMD_SET_OSD1:
			APP_OVERLAY_table_info_add(data->osd.line, data->osd.cow,data->osd.string);
			APP_OVERLAY_table_info_update();
			break;
		case UART_CMD_SET_OSD2:
			APP_OVERLAY_table_info_add(data->osd.line, data->osd.cow,data->osd.string);
			APP_OVERLAY_table_info_update();
			break;
		case UART_CMD_CLEAR_OSD:
			APP_OVERLAY_table_info_clear();
			break;
		default:
			break;
	}
}


int APP_UART_protocol_init(void)
{
#if defined(UART_PROTOCOL)
	UART_callback_init(uart_callback);
#endif
	return 0;
}
