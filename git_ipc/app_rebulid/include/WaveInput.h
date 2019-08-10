//
//  WaveInput.h
//  JA_minimodem
//
//  Created by LiHong on 16/7/13.
//  Copyright © 2016年 LiHong. All rights reserved.
//

#ifndef WaveInput_h
#define WaveInput_h

#include <stdio.h>
#include <stdint.h>


typedef void (*OnData)(char *data,int len);
long InitWaveInput(OnData callback);
void WriteAudioData(long handle,uint16_t *data,int len);
void StopWaveInput(long handle);

#endif /* WaveInput_h */
