

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "object.h"

//#include <JaEmbedded/types.h>
//#include <JaCPU/JaCPU.h>
//#include <JaEmbedded/log.h>
//#include "conf.h"

#ifndef BSP_IO_H_
#define BSP_IO_H_
#ifdef __cplusplus 
	extern "C" { 
#endif

//#define PRODUCT_P1
//#define PRODUCT_P2
//#define PRODUCT_C1
/*
/// 不使用引脚定义。
#define IO_NC					(-1)

#if defined(PRODUCT_P1)

#define IO_LED_RED				(JAC_GPIO_ID(5, 2))
#define IO_LED_RED_ACT			(JAC_GPIO_LOW)
#define IO_LED_BLUE				(JAC_GPIO_ID(5, 3))
#define IO_LED_BLUE_ACT			(JAC_GPIO_LOW)
#define IO_IN_DARK				(JAC_GPIO_ID(0, 6))
#define IO_IN_DARK_ACT			(JAC_GPIO_HIGH)
#define IO_IR_LIGHT				(JAC_GPIO_ID(0, 0))
#define IO_IR_LIGHT_ACT			(JAC_GPIO_HIGH)
#define IO_IRCUT_RED			(JAC_GPIO_ID(0, 2))
#define IO_IRCUT_RED_ACT		(JAC_GPIO_HIGH)
#define IO_IRCUT_BLACK			(JAC_GPIO_ID(0, 4))
#define IO_IRCUT_BLACK_ACT		(JAC_GPIO_HIGH)
#define IO_RST_KEY				(JAC_GPIO_ID(0, 7))
#define IO_RST_KEY_ACT			(JAC_GPIO_LOW)
#define IO_RST_IMG_SNSR			(JAC_GPIO_ID(0, 1))
#define IO_RST_IMG_SNSR_ACT		(JAC_GPIO_LOW)
#define IO_SPEAKER				IO_NC
#define IO_SPEAKER_ACK			(JAC_GPIO_HIGH)

#define IO_PAN_LIMIT			IO_NC
#define IO_PAN_LIMIT_ACK		(JAC_GPIO_LOW)
#define IO_PAN_AP				IO_NC
#define IO_PAN_AP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_AN				IO_NC
#define IO_PAN_AN_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BP				IO_NC
#define IO_PAN_BP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BN				IO_NC
#define IO_PAN_BN_ACK			(JAC_GPIO_HIGH)

#define IO_TILT_LIMIT			IO_NC
#define IO_TILT_LIMIT_ACK		(JAC_GPIO_HIGH)
#define IO_TILT_AP				IO_NC
#define IO_TILT_AP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_AN				IO_NC
#define IO_TILT_AN_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BP				IO_NC
#define IO_TILT_BP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BN				IO_NC
#define IO_TILT_BN_ACK			(JAC_GPIO_HIGH)

#elif defined(PRODUCT_P2)

#define IO_LED_RED				(JAC_GPIO_ID(9, 6))
#define IO_LED_RED_ACT			(JAC_GPIO_LOW)
#define IO_LED_BLUE				(JAC_GPIO_ID(9, 0))
#define IO_LED_BLUE_ACT			(JAC_GPIO_LOW)
#define IO_IN_DARK				(JAC_GPIO_ID(9, 5))
#define IO_IN_DARK_ACT			(JAC_GPIO_LOW)
#define IO_IR_LIGHT				(JAC_GPIO_ID(9, 1))
#define IO_IR_LIGHT_ACT			(JAC_GPIO_HIGH)
#define IO_IRCUT_RED			(JAC_GPIO_ID(9, 7))
#define IO_IRCUT_RED_ACT		(JAC_GPIO_HIGH)
#define IO_IRCUT_BLACK			(JAC_GPIO_ID(9, 4))
#define IO_IRCUT_BLACK_ACT		(JAC_GPIO_HIGH)
#define IO_RST_KEY				(JAC_GPIO_ID(9, 2))
#define IO_RST_KEY_ACT			(JAC_GPIO_LOW)
#define IO_RST_IMG_SNSR			(JAC_GPIO_ID(9, 3))
#define IO_RST_IMG_SNSR_ACT		(JAC_GPIO_LOW)
#define IO_SPEAKER				(JAC_GPIO_ID(6, 1))
#define IO_SPEAKER_ACK			(JAC_GPIO_LOW)

#define IO_PAN_LIMIT			IO_NC
#define IO_PAN_LIMIT_ACK		(JAC_GPIO_LOW)
#define IO_PAN_AP				IO_NC
#define IO_PAN_AP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_AN				IO_NC
#define IO_PAN_AN_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BP				IO_NC
#define IO_PAN_BP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BN				IO_NC
#define IO_PAN_BN_ACK			(JAC_GPIO_HIGH)

#define IO_TILT_LIMIT			IO_NC //(JAC_GPIO_ID(5, 2))
#define IO_TILT_LIMIT_ACK		(JAC_GPIO_HIGH)
#define IO_TILT_AP				IO_NC
#define IO_TILT_AP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_AN				IO_NC
#define IO_TILT_AN_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BP				IO_NC
#define IO_TILT_BP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BN				IO_NC
#define IO_TILT_BN_ACK			(JAC_GPIO_HIGH)

#elif defined(PRODUCT_C1)

#define IO_LED_RED				IO_NC
#define IO_LED_RED_ACT			(JAC_GPIO_LOW)
#define IO_LED_BLUE				IO_NC
#define IO_LED_BLUE_ACT			(JAC_GPIO_LOW)
#define IO_IN_DARK				(JAC_GPIO_ID(0, 6))
#define IO_IN_DARK_ACT			(JAC_GPIO_HIGH)
#define IO_IR_LIGHT				(JAC_GPIO_ID(0, 0))
#define IO_IR_LIGHT_ACT			(JAC_GPIO_HIGH)
#define IO_IRCUT_RED			(JAC_GPIO_ID(0, 2))
#define IO_IRCUT_RED_ACT		(JAC_GPIO_HIGH)
#define IO_IRCUT_BLACK			(JAC_GPIO_ID(0, 4))
#define IO_IRCUT_BLACK_ACT		(JAC_GPIO_HIGH)
#define IO_RST_KEY				(JAC_GPIO_ID(0, 7))
#define IO_RST_KEY_ACT			(JAC_GPIO_LOW)
#define IO_RST_IMG_SNSR			(JAC_GPIO_ID(0, 1))
#define IO_RST_IMG_SNSR_ACT		(JAC_GPIO_LOW)
#define IO_SPEAKER				IO_NC
#define IO_SPEAKER_ACK			(JAC_GPIO_HIGH)

#define IO_PAN_LIMIT			(JAC_GPIO_ID(5, 3))
#define IO_PAN_LIMIT_ACK		(JAC_GPIO_LOW)
#define IO_PAN_AP				(JAC_GPIO_ID(9, 4))
#define IO_PAN_AP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_AN				(JAC_GPIO_ID(9, 7))
#define IO_PAN_AN_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BP				(JAC_GPIO_ID(9, 3))
#define IO_PAN_BP_ACK			(JAC_GPIO_HIGH)
#define IO_PAN_BN				(JAC_GPIO_ID(9, 6))
#define IO_PAN_BN_ACK			(JAC_GPIO_HIGH)

#define IO_TILT_LIMIT			(JAC_GPIO_ID(5, 2))
#define IO_TILT_LIMIT_ACK		(JAC_GPIO_HIGH)
#define IO_TILT_AP				(JAC_GPIO_ID(9, 2))
#define IO_TILT_AP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_AN				(JAC_GPIO_ID(9, 1))
#define IO_TILT_AN_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BP				(JAC_GPIO_ID(9, 5))
#define IO_TILT_BP_ACK			(JAC_GPIO_HIGH)
#define IO_TILT_BN				(JAC_GPIO_ID(9, 0))
#define IO_TILT_BN_ACK			(JAC_GPIO_HIGH)

#endif

static inline JA_Void
BSP_IO_DUMP0(JA_PChar name, JA_Int gpio_id, TJA_GPIOPinIO io, TJA_GPIOPinLH act)
{
	if (IO_NC == gpio_id) {
		printf("%32s NC\r\n", name);
	} else {
		printf("%32s %s %2d,%d %s\r\n",
				name,
				JAC_GPIO_OUT == io ? ">>" : "<<",
				JAC_GPIO_GROUP(gpio_id), JAC_GPIO_PIN(gpio_id),
				JAC_GPIO_HIGH == act ? "High" : "Low");
	}
}
*/
/**
 * 打印 BSP IO 定义。
 */
 /*
static inline JA_Void
BSP_IO_DUMP()
{
	BSP_IO_DUMP0("Red LED", IO_LED_RED, JAC_GPIO_OUT, IO_LED_RED_ACT);
	BSP_IO_DUMP0("Blue LED", IO_LED_BLUE, JAC_GPIO_OUT, IO_LED_BLUE_ACT);
	BSP_IO_DUMP0("Dark", IO_IN_DARK, JAC_GPIO_IN, IO_IN_DARK_ACT);
	BSP_IO_DUMP0("IR Light", IO_IR_LIGHT, JAC_GPIO_OUT, IO_IR_LIGHT_ACT);
	BSP_IO_DUMP0("IR-Cut Red Drive", IO_IRCUT_RED, JAC_GPIO_OUT, IO_IRCUT_RED_ACT);
	BSP_IO_DUMP0("IR-Cut Black Drive", IO_IRCUT_BLACK, JAC_GPIO_OUT, IO_IRCUT_BLACK_ACT);
	BSP_IO_DUMP0("Reset Key", IO_RST_KEY, JAC_GPIO_IN, IO_RST_KEY_ACT);
	BSP_IO_DUMP0("Image Reset", IO_RST_IMG_SNSR, JAC_GPIO_IN, IO_RST_IMG_SNSR_ACT);
	BSP_IO_DUMP0("Speaker Drive", IO_SPEAKER, JAC_GPIO_OUT, IO_SPEAKER_ACK);

	BSP_IO_DUMP0("Pan Limit", IO_PAN_LIMIT, JAC_GPIO_IN, IO_SPEAKER_ACK);
	BSP_IO_DUMP0("Pan Drive[0]", IO_PAN_AP, JAC_GPIO_OUT, IO_PAN_AP_ACK);
	BSP_IO_DUMP0("Pan Drive[1]", IO_PAN_AN, JAC_GPIO_OUT, IO_PAN_AN_ACK);
	BSP_IO_DUMP0("Pan Drive[2]", IO_PAN_BP, JAC_GPIO_OUT, IO_PAN_BP_ACK);
	BSP_IO_DUMP0("Pan Drive[3]", IO_PAN_BN, JAC_GPIO_OUT, IO_PAN_BN_ACK);
	BSP_IO_DUMP0("Tilt Limit", IO_TILT_LIMIT, JAC_GPIO_IN, IO_TILT_LIMIT_ACK);
	BSP_IO_DUMP0("Tilt Drive[0]", IO_TILT_AP, JAC_GPIO_OUT, IO_TILT_AP_ACK);
	BSP_IO_DUMP0("Tilt Drive[1]", IO_TILT_AN, JAC_GPIO_OUT, IO_TILT_AN_ACK);
	BSP_IO_DUMP0("Tilt Drive[2]", IO_TILT_BP, JAC_GPIO_OUT, IO_TILT_BP_ACK);
	BSP_IO_DUMP0("Tilt Drive[3]", IO_TILT_BN, JAC_GPIO_OUT, IO_TILT_BN_ACK);
}


static inline JA_Void
BSP_IO_SET(JA_Int const gpio_id, TJA_GPIOPinLH lh)
{
	if (IO_NC == gpio_id) {
		return;
	}
	JaCPU()->GPIO()->setPinIO(gpio_id, JAC_GPIO_OUT);
	JaCPU()->GPIO()->setPinLH(gpio_id, lh);
}

static inline TJA_GPIOPinLH
BSP_IO_GET(JA_Int const gpio_id)
{
	if (IO_NC == gpio_id) {
		return JAC_GPIO_LOW;
	}
	TJA_GPIOPinLH lh = JAC_GPIO_LOW;
	JaCPU()->GPIO()->setPinIO(gpio_id, JAC_GPIO_IN);
	JaCPU()->GPIO()->getPinLH(gpio_id, &lh);
	return lh;
}

static inline JA_Void
BSP_IO_ACTIVE(JA_Int const gpio_id, TJA_GPIOPinLH act_lv, JA_Boolean act)
{
	if (IO_NC == gpio_id) {
		return;
	}

	JaCPU()->GPIO()->setPinIO(gpio_id, JAC_GPIO_OUT);
	JaCPU()->GPIO()->setPinLH(gpio_id, act ? act_lv : !act_lv);
}

static inline JA_Boolean
BSP_IO_IS_ACTIVED(JA_Int const gpio_id, TJA_GPIOPinLH act_lv)
{
	TJA_GPIOPinLH lh = !act_lv;

	if (IO_NC == gpio_id) {
		return JA_False;
	}

	JaCPU()->GPIO()->setPinIO(gpio_id, JAC_GPIO_IN);

	/// 重试多次消除电平抖动。
	if (0 == JaCPU()->GPIO()->getPinLH(gpio_id, &lh)) {
		return act_lv == lh;
	}

	return !act_lv;
}

#define BSP_IO_RESET_IMAGE_SENSOR() \
	do{\
		BSP_IO_SET(IO_RST_IMG_SNSR, JAC_GPIO_HIGH); usleep(1000);\
		BSP_IO_SET(IO_RST_IMG_SNSR, JAC_GPIO_LOW); usleep(4000);\
		BSP_IO_SET(IO_RST_IMG_SNSR, JAC_GPIO_HIGH); usleep(1000);\
	} while(0)

#define BSP_IO_LED_RED(__en)		do{ BSP_IO_ACTIVE(IO_LED_RED, IO_LED_RED_ACT, (__en)); } while(0)
#define BSP_IO_LED_BLUE(__en)		do{ BSP_IO_ACTIVE(IO_LED_BLUE, IO_LED_BLUE_ACT, (__en)); } while(0)

#define BSP_IO_IN_DARK()			BSP_IO_IS_ACTIVED(IO_IN_DARK, IO_IN_DARK_ACT)

/// 控制红外补光灯。
#define BSP_IO_IR_LIGHT(__en)		do{ BSP_IO_ACTIVE(IO_IR_LIGHT, IO_IR_LIGHT_ACT, (__en)); } while(0)

/// 启用喇叭。
#define BSP_IO_SPEAKER(__en	)		do{ BSP_IO_ACTIVE(IO_SPEAKER, IO_SPEAKER_ACK, (__en)); } while(0)

static inline JA_Void
BSP_IO_IRCUT(JA_Boolean daylight)
{
	JA_Int const io_red = IO_IRCUT_RED;
	JA_Int const io_black = IO_IRCUT_BLACK;

	if (io_red < 0 || io_black < 0) {
		return;
	}

	JaCPU()->GPIO()->setPinIO(io_red, JAC_GPIO_OUT);
	JaCPU()->GPIO()->setPinIO(io_black, JAC_GPIO_OUT);

	/// 脉冲低电平。
	JaCPU()->GPIO()->setPinLH(io_red, !IO_IRCUT_RED_ACT);
	JaCPU()->GPIO()->setPinLH(io_black, !IO_IRCUT_BLACK_ACT);
	usleep(10000);

	if (daylight)
	{
		JaCPU()->GPIO()->setPinLH(io_red, IO_IRCUT_RED_ACT);
	}
	else
	{
		JaCPU()->GPIO()->setPinLH(io_black, IO_IRCUT_BLACK_ACT);
	}

	usleep(100000);

	/// 脉冲低电平。
	JaCPU()->GPIO()->setPinLH(io_red, !IO_IRCUT_RED_ACT);
	JaCPU()->GPIO()->setPinLH(io_black, !IO_IRCUT_BLACK_ACT);

}

/// 判断复位开关是否按下。
#define BSP_IO_RST_KEY_PRESSED()	BSP_IO_IS_ACTIVED(IO_RST_KEY, IO_RST_KEY_ACT)

/// 判断水平限位开关触发。
#define BSP_IO_PAN_LIMIT()			BSP_IO_IS_ACTIVED(IO_PAN_LIMIT, IO_PAN_LIMIT_ACK)

/// 电机节拍控制。
static inline JA_Void
BSP_IO_PAN_CTRL(JA_UInt32 io_ctrl)
{
	BSP_IO_ACTIVE(IO_PAN_AP, IO_PAN_AP_ACK, !!(io_ctrl & (1<<0)));
	BSP_IO_ACTIVE(IO_PAN_AN, IO_PAN_AN_ACK, !!(io_ctrl & (1<<1)));
	BSP_IO_ACTIVE(IO_PAN_BP, IO_PAN_BP_ACK, !!(io_ctrl & (1<<2)));
	BSP_IO_ACTIVE(IO_PAN_BN, IO_PAN_BN_ACK, !!(io_ctrl & (1<<3)));
}

/// 判断垂直限位开关触发。
#define BSP_IO_TILT_LIMIT()			BSP_IO_IS_ACTIVED(IO_TILT_LIMIT, IO_TILT_LIMIT_ACK)

/// 电机节拍控制。
static inline JA_Void
BSP_IO_TILT_CTRL(JA_UInt32 io_ctrl)
{
	BSP_IO_ACTIVE(IO_TILT_AP, IO_TILT_AP_ACK, !!(io_ctrl & (1<<0)));
	BSP_IO_ACTIVE(IO_TILT_AN, IO_TILT_AN_ACK, !!(io_ctrl & (1<<1)));
	BSP_IO_ACTIVE(IO_TILT_BP, IO_TILT_BP_ACK, !!(io_ctrl & (1<<2)));
	BSP_IO_ACTIVE(IO_TILT_BN, IO_TILT_BN_ACK, !!(io_ctrl & (1<<3)));
}

/// IO 测试。

static inline JA_Void
BSP_IO_TEST()
{
	do
	{
		int counter = 0;

		JaLog()->info("BSP: LED Red On & LED Blue Off");
		BSP_IO_LED_RED(JA_True);
		BSP_IO_LED_BLUE(JA_False);
		sleep(5);

		JaLog()->info("BSP: LED Red Off & LED Blue On");
		BSP_IO_LED_RED(JA_False);
		BSP_IO_LED_BLUE(JA_True);
		sleep(5);

		JaLog()->info("BSP: IRCut Switched to Daylight Mode.");
		BSP_IO_IRCUT(JA_True);
		sleep(5);

		JaLog()->info("BSP: IRCut Switched to Night Mode.");
		BSP_IO_IRCUT(JA_False);
		sleep(5);

		counter = 10;
		while(counter--)
		{
			JaLog()->info("BSP: Reset Key %s.", BSP_IO_RST_KEY_PRESSED() ? "Pressed" : "Released");
			sleep(1);
		}

		counter = 10;
		while(counter--)
		{
			JaLog()->info("BSP: %sIn Dark.", BSP_IO_IN_DARK() ? "" : "NOT ");
			sleep(1);
		}

		JaLog()->info("BSP: IR Light Off.");
		BSP_IO_IR_LIGHT(JA_False);
		sleep(5);

		JaLog()->info("BSP: IR Light On.");
		BSP_IO_IR_LIGHT(JA_True);
		sleep(5);

	} while ('q' != getchar());

}

*/
#ifdef __cplusplus 
	}
#endif

#endif /* BSP_IO_H_ */
