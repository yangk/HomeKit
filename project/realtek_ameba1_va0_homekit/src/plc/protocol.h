#ifndef	_PROTOCOL_H_
#define	_PROTOCOL_H_

#include "smart_plc.h"
#include "comfunc.h"

struct EEP_PARAM
{
    uint8   panid[PANID_LEN];
    uint8   panid_flag;         //0x80:set plc panid, valid panid flag
    uint8   password[PWD_LEN];
    uint8   pwd_magic;          //0x55 is valid , or is 0xAA
    uint8   id[ID_LEN];
    uint8   gateway_id[ID_LEN];
//    uint8   bps;                //0:2400, 2:9600
    uint8   sid[SID_LEN];
    uint8   update[2];
	uint8	relay_flag[2];
};

struct func_ops
{
    uint8	di[2];                       //数据标志
	uint8	(*read)(uint8 *buff, uint8 max_len);//读功能执行函数
    uint8	(*write)(uint8 *buff, uint8 w_len);//写功能执行函数
};

struct FBD_Frame
{
    uint8 did[2];
    uint8 ctrl;
    uint8 data[1];
};
#define FBD_FRAME_HEAD   offset_of(struct FBD_Frame, data)

struct EVENT_INFOR
{
   uint8 type   :6; 
   uint8 report :1;
   uint8 invalid:1;
};


struct RAM
{
    uint8 magic_ram;//内存数据magic
    uint8 relay_stat;//继电器状态
#if DIMMER|COLOR_DIMMER
    uint8   target_percent;
    uint8   relay_state;
#endif
#if COLOR_DIMMER
    uint8   rgb_percent[3];
    uint8   target_rgb_percent[3];
    uint8   color_change_flag;
#endif
#if DIMMER
    uint8   percent;  //just for illuminate dimmer
#endif
};

struct SHS_frame
{
    uint8 stc;
    uint8 said[ADDRESS_LEN];
    uint8 taid[ADDRESS_LEN];
    uint8 seq;
    uint8 length;
    uint8 infor[1];
};

#define SHS_FRAME_HEAD       offset_of(struct SHS_frame, infor)
extern const struct func_ops func_items[];
//extern DATA_SEG struct RAM  ram;

uint8 compare_soft_ver(uint8 *buff, uint8 len);
static uint8 get_dkey(uint8 *buff, uint8 max_len);
uint8 _get_dev_infor(uint8* buff);
static uint8 get_device_attribute(uint8 *buff, uint8 max_len);
static uint8 get_sn(uint8 *buff, uint8 max_len);

#if DIMMER
void dimmer_100ms_hook(void);
#endif
#if COLOR_DIMMER 
void clour_dimmer_hook(void);
#endif
struct SHS_frame *get_smart_frame(uint8 rxframe_raw[],uint8 rxlen);
uint8 set_group_parameter(uint8 data[], uint8 len);
uint8 set_parameter(uint8 data[], uint8 len);
uint8 read_parameter(uint8 data[], uint8 len);
void _get_dev_type(uint8 *buff);
#endif
