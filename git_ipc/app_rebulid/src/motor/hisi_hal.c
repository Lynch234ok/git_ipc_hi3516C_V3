#include <stdint.h>
#include <stdio.h>

#include "stepper.h"
#include "hisi_hal.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

#define HISI_MPI_USING
#ifdef HISI_MPI_USING
#include <mpi_sys.h>

static int reg_read32(uint32_t addr)
{
	uint32_t data = 0;

	HI_MPI_SYS_GetReg((HI_U32)addr, (HI_U32*)&data);

	return data;
}

static int reg_write32(uint32_t addr, uint32_t data)
{
	if(HI_SUCCESS == HI_MPI_SYS_SetReg((HI_U32)addr, (HI_U32)data)){
		return 0;
	}

	return -1;
}

#else //NON HISI_MPI_USING

#include "memmap.h"

#define DEFAULT_MD_LEN 128

static uint32_t reg_read32(uint32_t addr)
{
	uint32_t data = 0;

	uint32_t ulAddr = addr;
	void*    pMem   = NULL;

	pMem = memmap(ulAddr, DEFAULT_MD_LEN);
	if(pMem == NULL) {
		return -1;
	}

	data = *(uint32_t*)pMem;

	memunmap(pMem);

	return data;
}

static uint32_t reg_write32(uint32_t addr, uint32_t data)
{
	uint32_t ulAddr = addr;
	void*    pMem   = NULL;

	pMem = memmap(ulAddr, DEFAULT_MD_LEN);
	if(pMem == NULL) {
		return -1;
	}

	*(uint32_t*)pMem = data;

	memunmap(pMem);

	return 0;
}

#endif


#ifdef HI3516E_V1
//HI3516Ev100 Based
#define HI_PLATFORM_REG_BASE (0x12000000)
#define GPIO0_BASE       (HI_PLATFORM_REG_BASE+0x140000)
#define GPIO_BASE(X)     (GPIO0_BASE + (X)*0x1000)

typedef struct {
	int Grp;
	int Pin;
	int Val;
} GpioSet_Struct;

static GpioSet_Struct GpioArray[8] = { //HI3516Ev100 Based
	{ 5, 0, 0 }, //FAN
	{ 5, 3, 0 }, //FAP

	{ 5, 4, 0 }, //ZAN
	{ 5, 1, 0 }, //ZAP

	{ 5, 2, 0 }, //ZBP
	{ 6, 5, 0 }, //ZBN

	{ 6, 0, 0 }, //FBN
	{ 6, 1, 0 }, //FBP
};

static int Gpio_Hal_Init(void)
{
	//Gpio_Multiplex: //HI3516Ev100 Based
	{
		reg_write32(0x120400C8, GPIO_PIN_LOW); //GPIO_FOCUS_A_P //0x120400C8 0:GPIO5_3 / 1:SPI1_CSN0 / 2:UART2_TXD / 3:JTAG_TMS / 4:I2S_WS_TX
		reg_write32(0x120400D4, GPIO_PIN_LOW); //GPIO_FOCUS_A_N //0x120400D4 0:GPIO5_0 / 1:SPI1_SCLK / 3:JTAG_TRSTN / 4:I2S_SD_RX
		reg_write32(0x12040018, GPIO_PIN_LOW); //GPIO_FOCUS_B_P //0x12040018 0:GPIO6_1 / 1:SHUTTER_TRIG
		reg_write32(0x1204001C, GPIO_PIN_LOW); //GPIO_FOCUS_B_N //0x1204001C 0:GPIO6_0 / 1:FLASH_TRIG

		reg_write32(0x120400CC, GPIO_PIN_LOW); //GPIO_ZOOM_A_P //0x120400CC 0:GPIO5_1 / 1:SPI1_SDO / 3:JTAG_TCK / 4:I2S_BCLK_TX
		reg_write32(0x120400C4, GPIO_PIN_LOW); //GPIO_ZOOM_A_N //0x120400C4 0:GPIO5_4 / 1:SPI1_CSN1 / 2:UART2_RXD / 3:JTAG_TDI / 4:I2S_SD_TX
		reg_write32(0x120400D0, GPIO_PIN_LOW); //GPIO_ZOOM_B_P //0x120400D0 0:GPIO5_2 / 1:SPI1_SDI / 3:JTAG_TDO / 4:I2S_MCLK
		reg_write32(0x12040004, GPIO_PIN_LOW); //GPIO_ZOOM_B_N //0x12040004 0:GPIO6_5 / 1:PWM0
	}

	//Gpio_InitSetting:
	{
		int ii;
		for(ii = 0; ii < ARRAY_SIZE(GpioArray); ii += 1) {
			gpio_set_pin(GpioArray[ii].Grp, GpioArray[ii].Pin, GpioArray[ii].Val);
			gpio_set_dir(GpioArray[ii].Grp, GpioArray[ii].Pin, GPIO_DIR_OUT);
		}
	}

    return 0;
}

static int Gpio_Hal_Exit(void)
{
	return 0;
}
#endif

int StepperGpio_Init(void)
{
	return Gpio_Hal_Init();
}

int StepperGpio_Set(int Gpio, int Value)
{
	if ((Gpio < 0) || (Gpio >= ARRAY_SIZE(GpioArray))) {
		return -1;
	}

	gpio_set_pin(GpioArray[Gpio].Grp, GpioArray[Gpio].Pin, Value);

	return 0;
}

int StepperGpio_Exit(void)
{
	return Gpio_Hal_Exit();
}
