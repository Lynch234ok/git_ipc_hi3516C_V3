#!/bin/sh
## VIC driver VIC1##
insmod vpl_vic.ko abAEEn=0,1 abIrisEn=0,0 abAFEn=0,0 abAWBEn=0,1 gdwBusFreq=200000000 gdwSignalWaitTime=4000

## sensor driver VIC1 RDK ##
insmod AR0330.ko dwSignalPort=1 adwVideoWidth=2304,2304 adwVideoHeight=1536,1536 adwIICBusNum=1,1 abEnMipi=0,1 abEnData10bit=0,1

insmod AutoExposure.ko
insmod AutoWhiteBalance.ko
