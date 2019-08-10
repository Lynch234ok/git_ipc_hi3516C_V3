
#include "hikvisionv10.h"
#include "netsdk_util.h"
/*
<VideoInputChannel version=¡°1.0¡± xmlns=¡°http://www.hikvision.com/ver10/XMLSchema¡±>
<id> <!-- req, xs:integer --> </id>
<powerLineFrequencyMode> <!-- opt, xs:string ¡°50hz, 60hz¡± -->
</powerLineFrequencyMode>
<whiteBalanceMode>
.2009 ¨C 2014 by HIKVISION. All rights reserved.
47
<!-- opt, xs:string, ¡°manual,auto,indoor/incandescent¡± -->
</whiteBalanceMode>
<gainLevel> <!-- opt, xs:integer, 0..100--> </gainLevel>
<brightnessLevel> <!-- opt, xs:integer, 0..100 --> </brightnessLevel>
<contrastLevel> <!-- opt, xs:integer, 0..100 --> </contrastLevel>
<saturationLevel> <!-- opt, xs:integer, 0..100 --> </saturationLevel>
<DayNightFilter> <!-- opt -->
<dayNightFilterType>
<!-- opt, xs:string, ¡°day,night,auto¡± -->
</dayNightFilterType>
</DayNightFilter>
<VideoInputChannel>
*/

int HIKVISIONv10_video_inputs(LP_HTTP_CONTEXT context)
{
	const char *text = "<VideoInput version=\"1.0\" xmlns=\"http://www.hikvision.com/ver10/XMLSchema\">"
		"<VideoInputChannelList/> <!-- opt -->"
		"</VideoInput>";

	
	HIKv10_response(context->sock, text);
	return 0;
}



