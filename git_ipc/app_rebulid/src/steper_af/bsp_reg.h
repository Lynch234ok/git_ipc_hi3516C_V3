#ifndef _BSP_REG_H
#define _BSP_REG_H
#include <mpi_sys.h>
#include <hi_type.h>
#include "bufio.h"
#include <object.h>

#define HI_SUCCESS 				0
#define GPIO_BASE_ADDR 			0x20140000
#define GPIO_MUX_BASE_ADDR		0x200F0000

/*
#define REG_ID_ZOOM_AP			64
#define REG_ID_ZOOM_AN			59
#define REG_ID_ZOOM_BP			62
#define REG_ID_ZOOM_BN			66

#define GPIO_GROUP_ZOOM_AP		11		
#define GPIO_GROUP_ZOOM_AN		11	
#define GPIO_GROUP_ZOOM_BP		11	
#define GPIO_GROUP_ZOOM_BN		1	

#define PIN_FUN_ZOOM_AP			5
#define PIN_FUN_ZOOM_AN			0
#define PIN_FUN_ZOOM_BP			0
#define PIN_FUN_ZOOM_BN			0

#define PIN_ID_ZOOM_AP			6	
#define PIN_ID_ZOOM_AN			1
#define PIN_ID_ZOOM_BP			4
#define PIN_ID_ZOOM_BN			2




#define REG_ID_FOCUS_AP		61
#define REG_ID_FOCUS_AN		60
#define REG_ID_FOCUS_BP		63
#define REG_ID_FOCUS_BN		65

#define GPIO_GROUP_FOCUS_AP		11	
#define GPIO_GROUP_FOCUS_AN		11	
#define GPIO_GROUP_FOCUS_BP		11	
#define GPIO_GROUP_FOCUS_BN		11

#define PIN_FUN_FOCUS_AP		0
#define PIN_FUN_FOCUS_AN		0
#define PIN_FUN_FOCUS_BP		0
#define PIN_FUN_FOCUS_BN		0

#define PIN_ID_FOCUS_AP			3
#define PIN_ID_FOCUS_AN			2
#define PIN_ID_FOCUS_BP			5
#define PIN_ID_FOCUS_BN			7


//ZOOM	PIN
#define REG_ID_ZOOM_AP			59
#define REG_ID_ZOOM_AN			64
#define REG_ID_ZOOM_BP			63
#define REG_ID_ZOOM_BN			55

#define GPIO_GROUP_ZOOM_AP		11		
#define GPIO_GROUP_ZOOM_AN		11	
#define GPIO_GROUP_ZOOM_BP		11	
#define GPIO_GROUP_ZOOM_BN		11	

#define PIN_FUN_ZOOM_AP			0
#define PIN_FUN_ZOOM_AN			5
#define PIN_FUN_ZOOM_BP			0
#define PIN_FUN_ZOOM_BN			0

#define PIN_ID_ZOOM_AP			1	
#define PIN_ID_ZOOM_AN			6
#define PIN_ID_ZOOM_BP			5
#define PIN_ID_ZOOM_BN			7

//FOCUS PIN
#define REG_ID_FOCUS_AP			66
#define REG_ID_FOCUS_AN			62
#define REG_ID_FOCUS_BP			61
#define REG_ID_FOCUS_BN			60

#define GPIO_GROUP_FOCUS_AP		1	
#define GPIO_GROUP_FOCUS_AN		11	
#define GPIO_GROUP_FOCUS_BP		11	
#define GPIO_GROUP_FOCUS_BN		11

#define PIN_FUN_FOCUS_AP		0
#define PIN_FUN_FOCUS_AN		0
#define PIN_FUN_FOCUS_BP		0
#define PIN_FUN_FOCUS_BN		0

#define PIN_ID_FOCUS_AP			2
#define PIN_ID_FOCUS_AN			4
#define PIN_ID_FOCUS_BP			3
#define PIN_ID_FOCUS_BN			2
*/
	//ZOOM	PIN
#define REG_ID_ZOOM_AP			66
#define REG_ID_ZOOM_AN			62
#define REG_ID_ZOOM_BP			61
#define REG_ID_ZOOM_BN			60
	
#define GPIO_GROUP_ZOOM_AP		1		
#define GPIO_GROUP_ZOOM_AN		11	
#define GPIO_GROUP_ZOOM_BP		11	
#define GPIO_GROUP_ZOOM_BN		11	
	
#define PIN_FUN_ZOOM_AP			0
#define PIN_FUN_ZOOM_AN			0
#define PIN_FUN_ZOOM_BP			0
#define PIN_FUN_ZOOM_BN			0
	
#define PIN_ID_ZOOM_AP			2	
#define PIN_ID_ZOOM_AN			4
#define PIN_ID_ZOOM_BP			3
#define PIN_ID_ZOOM_BN			2
	
	//FOCUS PIN
#define REG_ID_FOCUS_AP			59
#define REG_ID_FOCUS_AN			64
#define REG_ID_FOCUS_BP			65
#define REG_ID_FOCUS_BN			63
	
#define GPIO_GROUP_FOCUS_AP		11	
#define GPIO_GROUP_FOCUS_AN		11	
#define GPIO_GROUP_FOCUS_BP		11	
#define GPIO_GROUP_FOCUS_BN		11
	
#define PIN_FUN_FOCUS_AP		0
#define PIN_FUN_FOCUS_AN		5
#define PIN_FUN_FOCUS_BP		0
#define PIN_FUN_FOCUS_BN		0
	
#define PIN_ID_FOCUS_AP			1
#define PIN_ID_FOCUS_AN			6
#define PIN_ID_FOCUS_BP			7
#define PIN_ID_FOCUS_BN			5


typedef struct regCtrl{
	uint32_t (*reg_read_pin_data)(int gpio_group, int gpio_pin);
	void (*reg_write_pin_data)(int gpio_group, int gpio_pin, uint8_t val);
	int (*reg_set_pin_fun)(int regNum, int FunSel);
}stRegCtrl, *pstRegCtrl;

//extern pstRegCtrl RegCreate(void);
//extern stRegCtrl _RegCtrl;
extern pstRegCtrl pRegCtrl;
#endif

