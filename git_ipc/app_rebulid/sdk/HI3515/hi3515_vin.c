
#include "sdk/sdk_api.h"
#include "hi3515.h"
#include "sdk_trace.h"

#define HI3515_VIN_CH_BACKLOG_REF (8)

typedef struct SDK_VIN_ATTR
{
	SDK_VIN_HW_SPEC_t hw_spec;
}SDK_VIN_ATTR_t;

typedef struct SDK_VIN
{
	SDK_VIN_API_t api;
	SDK_VIN_ATTR_t attr;
}SDK_VIN_t;
static SDK_VIN_t _sdk_vin;
SDK_VIN_API_t* sdk_vin = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SDK_VIN_HW_SPEC_t vin_hw_spec()
{
	if(sdk_vin){
		return _sdk_vin.attr.hw_spec;
	}
	return SDK_VIN_HW_SPEC_INVALID;
}

static int vin_capture(int vin, FILE* bitmap_stream)
{
	return -1;
}

static SDK_VIN_t _sdk_vin = {
	// init the interfaces
	.api = {
		.hw_spec = vin_hw_spec,
		.capture = vin_capture,
	},
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// external handler
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SDK_init_vin(SDK_VIN_HW_SPEC_t hw_spec)
{
	if(!sdk_vin){
		// set handler pointer
		sdk_vin = (SDK_VIN_API_t*)(&_sdk_vin);

		// attributes
		_sdk_vin.attr.hw_spec = hw_spec;

		// success
		return 0;
	}
	return -1;
}

int SDK_destroy_vin()
{
	if(sdk_vin){


		// clear handler pointer
		sdk_vin = NULL;
		return 0L;
	}
	return -1;
}

