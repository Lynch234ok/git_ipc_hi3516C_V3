#include <hi_type.h>
#include <sdk/sdk_sys.h>
#include "network_interface.h"

#if defined(HI3516C_V3) | defined(HI3516E_V1)
#define MDIO_RWCTRL_ADDR				(0x10051100)
#else
#define MDIO_RWCTRL_ADDR				(0x10091100)
#endif

#define MDIO_RWCTRL_VALUE_ONE			(0x143)
#define MDIO_RWCTRL_VALUE_TWO			(0x145)

#if defined(HI3516C_V3) | defined(HI3516E_V1)
#define MDIO_RO_DATA_ADDR				(0x10051104)
#else
#define MDIO_RO_DATA_ADDR				(0x10091104)
#endif

#define NO_PHY_CONNECT					(0xffff)

/* 返回:true有线，false无线 */
bool network_check_interface()
{
    unsigned phy_val1, phy_val2;
    bool ret = true;
    if(sdk_sys) {
        sdk_sys->write_reg(MDIO_RWCTRL_ADDR, MDIO_RWCTRL_VALUE_ONE);
        sdk_sys->read_reg(MDIO_RO_DATA_ADDR, &phy_val1);

        sdk_sys->write_reg(MDIO_RWCTRL_ADDR, MDIO_RWCTRL_VALUE_TWO);
        sdk_sys->read_reg(MDIO_RO_DATA_ADDR, &phy_val2);

        if(NO_PHY_CONNECT == phy_val1
            || NO_PHY_CONNECT == phy_val2){

            printf("There is no phy device connect !!! \n");
            ret = false;
        }else{//读取到PHY设备，就进行配置

            printf("Phy device is connected !!! \n");
            ret = true;
        }
    }
    return ret;
}

int network_ifconf_set_interface(const char if_name[IFNAMSIZ], ifconf_interface_t* ifr)
{
    bool inter;
    inter = network_check_interface();
    if(inter) {
        ifconf_set_interface(if_name, ifr);
    }
    else {
    }

    return 0;
}

