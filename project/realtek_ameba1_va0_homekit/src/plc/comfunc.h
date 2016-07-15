#ifndef _COMFUNC_H_
#define _COMFUNC_H_


#include "config.h"

#define offset_of(obj_type,mb)  ((unsigned int)&(((obj_type*)0)->mb))

#define GET_NBITS_VAL(val,x1,x2)            (((val)>>(x1))&((1<<((x2)-(x1)+1))-1))
//#define MIN(x,y)    (x > y ? y : x )
//#define MAX(x,y)    (x > y ? x : y )


enum RXTX_STATUS
{
    PLC_RX=0x00,PLC_TX=0x01,RS485_RX=0x02,RS485_TX=0x03,NO_STATUS
};

uint8 checksum (uint8 *data,uint8 len);
void mymemcpy(void *dst,void *src,uint8 len);
//void push_rxtx_status(enum RXTX_STATUS status);
//uint8 get_rxtx_status(void);
uint8 memcmp_my(const void *s1, const void *s2, uint8 n);
void memset_my(void *s1, uint8 value, uint8 n);
void* memmove_my(void* dest, void* source, uint16 n);
uint8 is_all_xx(const void *s1, uint8 value, uint8 n);
uint8 find_max(uint8 buf[], uint8 n);
//uint16 sp_crc16(const uint8 *buf, uint8 size);
uint16 sp_crc16_with_init(uint16 crc, const uint8 *buf, uint8 size);
void numeric2bcd(uint32 value,uint8 *bcd, uint8 bytes);
#endif
