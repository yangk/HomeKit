#include "config.h"
#include "smart_plc.h"
#include "protocol.h"
#include "other/outlet/outlet.h"

extern outlet_state outletstateA;
extern outlet_state outletstateB;

//DATA_SEG struct RAM;                  //内存最多分配15个字节

/**************************************
    判断是否接受到完整的一帧数据
    output:		_TRUE, _FALSE
 
 举例：7e 7e 00 00 00 01 00 00 00 02 72 04 02 02 02 02 cs
**************************************/
struct SHS_frame *get_smart_frame(uint8 rxframe_raw[],uint8 rxlen)
{
	struct SHS_frame *pframe;
    uint8 i=0;
    uint8 len;

start_lbl:
    while(i < rxlen)
    {
        if(STC == rxframe_raw[i]) break;
        i++;
    }
//   if(i >= rxlen) return(NULL);
    if(rxlen-i < SHS_FRAME_HEAD) return(NULL);//接收等待length长度
    pframe = (struct SHS_frame*)&rxframe_raw[i];
    len = pframe->length;

    if(i+SHS_FRAME_HEAD+len+1 > rxlen)    
    {
        i++;
        goto start_lbl;
    }

    if(pframe->infor[len] != checksum((uint8 *)pframe, len+SHS_FRAME_HEAD))
    {
        i++;
        goto start_lbl;
    }
//    uart_read(rxframe_raw, i+SHS_FRAME_HEAD+len+1);
//    mymemcpy(&rxframe_raw[0], &rxframe_raw[i], SHS_FRAME_HEAD+len+1);
    pframe = (struct SHS_frame*)&rxframe_raw[i];
	return(pframe);
}

/*************************************************
                读取设备类型
*************************************************/
#define     LIGHT_CTRL      0x0013
#define     DIMMING_DEV     0x0014
#define     COLOR_DEV       0x0015
#define     PLUG_DEV        0x0018
#define     CURTAIN_DEV     0x0004
#define     CURTAIN_DEV_II  0x0004

#define     ES      0xFFFF

#if PWRCTRL_86
const uint8 _device_type[8] = {0xFF,0xFF,0x18,0x00,0x01,0x00,0x01,0x00};
#elif LIGHTCTRL_2
const uint8 _device_type[8] = {0xFF,0xFF,0x13,0x00,0x00,0x00,0x02,0x00};
#elif DIMMER|DIMMER_LIGHT
const uint8 _device_type[8] = {0xFF,0xFF,0x14,0x00,0x00,0x00,0x01,0x00};
#elif COLOR_DIMMER
const uint8 _device_type[8] = {0xFF,0xFF,0x15,0x00,0x00,0x00,0x01,0x00};
#elif CURTAIN_MOTOR

#if DOUBLE_CURTAIN
const uint8 _device_type[8] = {0xFF,0xFF,0x04,0x00,0x02,0x00,0x02,0x00};
#else
const uint8 _device_type[8] = {0xFF,0xFF,0x04,0x00,0x01,0x00,0x01,0x00};
#endif
#endif
const uint8 _device_type[8] = {0xFF,0xFF,0x04,0x00,0x01,0x00,0x01,0x00};
void _get_dev_type(uint8 *buff)
{
    mymemcpy(buff, _device_type, sizeof(_device_type));
}

static uint8 get_device_type(uint8 *buff, uint8 max_len)
{	
	  if(max_len < 8) return(0);
    _get_dev_type(&buff[1]);
#if 0	
    uint8 i = 1, dev;

#if DOUBLE_CURTAIN
    dev = CURTAIN_DEV_II;
#else
    dev = CURTAIN_DEV;
#endif
    if(max_len < 0x08) return(0);

    buff[i++] = ES & 0xFF;          //厂商号
    buff[i++] = (ES>>8) & 0xFF;
    buff[i++] = dev & 0xFF;         //设备种类
    buff[i++] = (dev>>8) & 0xFF;

#if DOUBLE_CURTAIN
    buff[i++] = 0x02;
#else
    buff[i++] = 0x01;               //
#endif
    buff[i++] = 0x00;
    if((DIMMING_DEV == dev) || (PLUG_DEV == dev) || (COLOR_DEV == dev))
    {
        buff[i++] = 0x01;            //附加信息（bit0~bit3为通道号）
    }
    else if(LIGHT_CTRL == dev)
    {
        buff[i++] = 0x03;
    }
#if CURTAIN_MOTOR
#if DOUBLE_CURTAIN
    else if(CURTAIN_DEV_II == dev)
    {
        buff[i++] = 0x02;
    }  
#else  
    else if(CURTAIN_DEV == dev)
    {
        buff[i++] = 0x01;
    }
#endif
#endif
    else
    {
        buff[i++] = 0x00;
    }
    buff[i++] = 0x00;
#endif
    return(0x08);
}
#if PWRCTRL_3|DIMMER|COLOR_DIMMER|PWRCTRL_86
/***********************************
 (bit7:1/0 表示通/断；bit6:保留；bit5~bit0:1/0 表示通道操作/不操作；bit0: 表示第 1 个通道
 读取继电器状态：bit5~bit0: 1 表示通；0 表示断）
***********************************/
static uint8 _get_relay_status( uint8 *buff, uint8 max_len)
{
    if(max_len < 1) return(0);
  
    buff[1]=_OFF;
    if(GPIO_ReadOutputData(RELAY_CTRL1_PORT, RELAY_CTRL1_PIN))
    {
        buff[1] |= _ON;
    }
#if PWRCTRL_3
    if(GPIO_ReadOutputData(RELAY_CTRL2_PORT, RELAY_CTRL2_PIN))
    {
        buff[1] |= (_ON << 0x01);
    }    
    if(GPIO_ReadOutputData(RELAY_CTRL3_PORT, RELAY_CTRL3_PIN))
    {
        buff[1] |= (_ON << 0x02);
    }
#endif
    return(1);
}

static uint8 get_relay_status(uint8 *buff, uint8 max_len)
{
   return(_get_relay_status(buff,max_len));
}

#if PWRCTRL_86
/*********************************************************
   继电器翻转，使用的数据标识为OxC018，数据返回0xC012
********************************************************/
static uint8 set_relay_reverse(uint8 *buff, uint8 w_len)
{
    uint8 i=0;
    GPIO_WriteReverse(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
    //DID
    buff[i++] = 0x12;
    buff[i++] = 0xC0;
    //ctrl
    buff[i++] = 0x01;
    _get_relay_status(&buff[i++], 1);
    if(buff[0]&0x01)
    {
        ram.relay_stat |= 0x81;
    }

    return(CHG_DID|i);
}
#endif

static uint8 set_relay_ctrl(uint8 *buff, uint8 w_len)
{
    uint8 stat1,stat2,i;

    if(w_len != 0x01) return(DATA_ERR);
    _get_relay_status(&stat1, 1);

    if (buff[0]&0x80) 
    {
#if COLOR_DIMMER
 //       for(i=0; i < 3; i++)
 //       {
 //           rgb_percent[i] = 0;
 //           pwm_color_adjust(i+1, rgb_percent[i]);
 //       }
        ram.color_change_flag =1;
#endif
        //turn on
        if (buff[0] & 0x01) GPIO_WriteHigh(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
#if PWRCTRL_3
        if (buff[0] & 0x02) GPIO_WriteHigh(RELAY_CTRL2_PORT,RELAY_CTRL2_PIN);
        if (buff[0] & 0x04) GPIO_WriteHigh(RELAY_CTRL3_PORT,RELAY_CTRL3_PIN);
#endif
    }
    else
    {
        if (buff[0] & 0x01)  GPIO_WriteLow(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
#if COLOR_DIMMER
        ram.color_change_flag = 0;
#endif
#if PWRCTRL_3
        if (buff[0] & 0x02)  GPIO_WriteLow(RELAY_CTRL2_PORT,RELAY_CTRL2_PIN);
        if (buff[0] & 0x04)  GPIO_WriteLow(RELAY_CTRL3_PORT,RELAY_CTRL3_PIN);
#endif
    }
    ram.relay_stat = buff[0];//备份继电器状态
    _get_relay_status(&stat2, 1);
//    if(stat1^stat2)
    {
//        event.report = 0x01;
//        event.type |= 0x01;
    }
    _get_relay_status(buff, 1);
    return(NO_ERR);
}
#endif

#if DIMMER
/****************************************************
占空比：0~100（百分比），百分比越大，占空比越大
****************************************************/
#define DIMM_STEP  3
void dimmer_100ms_hook(void)
{
    int8 delt; 
     
    if(0x00 == ram.relay_stat&0x80) 
    {
        ram.percent = 0;
        pwm_prescaler(ram.percent);
        return;
    }
    delt = ram.target_percent - ram.percent;
        
    if (delt > 0) 
    {
        if (delt > DIMM_STEP) delt = DIMM_STEP;
        ram.percent += delt;
        pwm_prescaler(ram.percent);
    }
    else if (delt < 0) 
    {
        delt = -delt;
        if (delt > DIMM_STEP) delt = DIMM_STEP;
        ram.percent -= delt;
        pwm_prescaler(ram.percent);
    }
}
static uint8 set_dimmer(uint8 *buff, uint8 w_len)
{
    if(w_len != 0x01) 
    {
        return(DATA_ERR);
    }
    if (buff[0]>100) buff[0] = 100;
    ram.target_percent = buff[0];
    if(0x00 == ram.target_percent)
    {
        GPIO_WriteLow(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
    }
    return(NO_ERR);
}
static uint8 get_dimmer(uint8 *buff, uint8 max_len)
{    
    if(max_len < 1) return(0);
    buff[1] = ram.target_percent;
    return(1);
}
#endif
#if COLOR_DIMMER 
/****************************************************
占空比：0~100（百分比），百分比越大，占空比越大
****************************************************/
#define DIMM_COLOUR_STEP  2
void clour_dimmer_hook(void)
{
   int8 delt,i;
   uint8 p, max_p;

   if(0x00 == ram.relay_stat&0x80) 
   {
       for (i = 0; i < 3;  i++) 
       {
           ram.rgb_percent[i] = 0;
           pwm_color_adjust(i+1, ram.rgb_percent[i]);
       }
       return;
   }
   if (0x00 == ram.color_change_flag) return;
   ram.color_change_flag = 0;

   max_p = find_max(ram.target_rgb_percent, 3);
   for (i = 0 ; i < 3;  i++) 
   {
#if DIMMER_LIGHT
        p =  ram.target_percent;
#else       
        p =  ram.target_rgb_percent[i];  
        p = (uint16_t)(p * ram.target_percent) / max_p;
#endif              

         delt = p - ram.rgb_percent[i];
         if (0 != delt) ram.color_change_flag++;
         if (delt > 0) {
            if (delt > DIMM_COLOUR_STEP) delt = DIMM_COLOUR_STEP;
            ram.rgb_percent[i] += delt;
            pwm_color_adjust(i+1, ram.rgb_percent[i]);
         }
         if (delt < 0) {
            delt = -delt;
            if (delt > DIMM_COLOUR_STEP) delt = DIMM_COLOUR_STEP;
            ram.rgb_percent[i] -= delt;
            pwm_color_adjust(i+1, ram.rgb_percent[i]);
         }
	 if(0 != ram.color_change_flag) break;
   }
}
/*rgb is 0~255 ,so...*/
static uint8 set_rgb_percent(uint8 *buff, uint8 w_len)
{
   uint8 i;
    if(w_len != 0x03)  return(DATA_ERR);
    for (i =0 ; i < 3 ; i++) 
    {
       ram.target_rgb_percent[i] = buff[i]; 
    }
    ram.color_change_flag = 1;
    return(NO_ERR);
}
 
static uint8 get_rgb_percent(uint8 *buff, uint8 max_len)
{    
    if(max_len < 3) return(0);
    mymemcpy(&buff[1], ram.target_rgb_percent,3); 
    return(3);
}
static uint8 set_color_dimmer(uint8 *buff, uint8 w_len)
{
    uint8_t stat;

    if(w_len != 0x01) 
    {
        return(DATA_ERR);
    }
    if (buff[0]>100) buff[0] = 100;
    ram.target_percent = buff[0]; 
    _get_relay_status(&stat, 1);
    if(_ON == stat)
    {
        ram.color_change_flag = 1;
    }
    return(NO_ERR);
}
static uint8 get_color_dimmer(uint8 *buff, uint8 max_len)
{
    if(max_len < 1) return(0);
    buff[1] = ram.target_percent; 
    return(1);
}

#endif
/*******************************************************
                应用层通信协议及版本
********************************************************/
#if  CURTAIN_MOTOR
const static uint8 soft_ver[]="EASTSOFT(v1.0)";
#if  DOUBLE_CURTAIN
const static uint8 dev_infor[]="ESACT-CC2-AC(v1.0)-20150917";
#else
const static uint8 dev_infor[]="ESACT-CC1-AC(v1.0)-20150917";
#endif
#endif

const static uint8 soft_ver[]="EASTSOFT(v1.0)";
const static uint8 dev_infor[]="ESACT-CC1-AC(v1.0)-20150917";

static uint8 get_string(uint8 *buff, uint8 max_len, uint8 *str)
{
    if(max_len < strlen(str)) return(0);

    mymemcpy(&buff[1], (uint8 *)str, strlen(str));
    return(strlen(str));
}
/******************************************************
    设备描述信息设备制造商
    **************************************************/
static uint8 get_dev_infor(uint8 *buff, uint8 max_len)
{
    return(get_string(buff, max_len, dev_infor));
}
/*******************************************************/
static uint8 get_soft_ver(uint8 *buff, uint8 max_len)
{
    return(get_string(buff, max_len, soft_ver));
}

static uint8 set_password(uint8 *buff, uint8 w_len)
{
    uint8 i=0, ret = 0;

    if(w_len != 0x02)
    {
        return(DATA_ERR);
    }
    if((VALID_DATA == eep_param.pwd_magic) 
       && (0x00 == memcmp_my(eep_param.password, buff, sizeof(eep_param.password))))
    {
        ret = 1;
    }
    else if(INVALID_DATA == eep_param.pwd_magic)
    {
        ret = 1;
        mymemcpy(eep_param.password, buff, sizeof(eep_param.password));
        eep_param.pwd_magic = VALID_DATA;
        //EEP_Write(OF_PLC_PARAM, (uint8 *)&eep_param, sizeof(eep_param));
    }
    buff[i++] = 0x30;
    buff[i++] = 0xC0;
    buff[i++] = 0x01;
    buff[i++] = ret;
    return(CHG_DID|i);
}

static uint8 set_onoff(uint8 *buff, uint8 w_len)
{
    if (w_len != 0x02) {
        return(DATA_ERR);
    }
    
    if (0x01 == buff[0]) {
        outletstateA.on = buff[1];
    }else if (0x02 == buff[0]) {
        outletstateB.on = buff[1];
    }
    
    //HAPoutletNotify((int)buff[0],(int)buff[1]);
    return(0);
}

static uint8 get_time(uint8 *buff, uint8 w_len)
{
     if (w_len < 0x02) {
        app_handle(SET_DATE, NULL);
    }
    return(0);
}

static uint8 set_resetdefault(uint8 *buff, uint8 w_len)
{
    wait(2);
    phytrex_reset(2);
    return(0);
}

static uint8 set_meter(uint8 *buff, uint8 w_len)
{
    float voltage = 0;
    float current = 0;
    float power = 0;

    if (w_len != 10) {
        return(DATA_ERR);
    }
    voltage = bcd2bin(buff[2])/10.0+bcd2bin(buff[3])*10.0;
    current = bcd2bin(buff[4])/1000.0+bcd2bin(buff[5])/10.0+bcd2bin(buff[6])*10.0;
    power = bcd2bin(buff[7])/10000.0+bcd2bin(buff[8])/100.0+bcd2bin(buff[9]);
    power *=1000;//kW--->W
    
    if (0x01 == buff[0]) {
        outletstateA.metvol = voltage;
        outletstateA.metcur = current;
        outletstateA.metpow = power;
        outletstateA.alarm = buff[1];
    }else if (0x02 == buff[0]) {
        outletstateB.metvol = voltage;
        outletstateB.metcur = current;
        outletstateB.metpow = power;
        outletstateB.alarm = buff[1];
    }

    //HAPmetvolNotify((int)buff[0], (double)voltage);
    //HAPmetcurNotify((int)buff[0], (double)current);
    //HAPmetpowNotify((int)buff[0], (double)power);
    
    return(0);
}

#if HISDATA_15_MIN
static uint8 set_hisdata(uint8 *buff, uint8 w_len)
{
    hisdata_state reporthisdata;
    
    if (w_len != 0x07) {
        return(DATA_ERR);
    }
    reporthisdata.channel = buff[0];
    reporthisdata.year = bcd2bin(buff[1]);
    reporthisdata.mon = bcd2bin(buff[2]);
    reporthisdata.day = bcd2bin(buff[3]);
    reporthisdata.hour = bcd2bin(buff[4]);
    reporthisdata.min = bcd2bin(buff[5]);
    reporthisdata.quantity = buff[6];
    wirte_hisdata2flash(reporthisdata);
    return(0);
}
#else
static uint8 set_hisdata(uint8 *buff, uint8 w_len)
{
    hisdata_state reporthisdata;
    
    if (w_len != 0x06) {
        return(DATA_ERR);
    }
    reporthisdata.channel = buff[0];
    reporthisdata.year = bcd2bin(buff[1]);
    reporthisdata.mon = bcd2bin(buff[2]);
    reporthisdata.day = bcd2bin(buff[3]);
    reporthisdata.hour = bcd2bin(buff[4]);
    reporthisdata.quantity = buff[5];
    wirte_hisdata2flash(reporthisdata);
    return(0);
}
#endif
static uint8 set_thrvol(uint8 *buff, uint8 w_len)
{
    float overvoltage = 0;
    float undervoltage = 0;
    
    if (w_len != 0x05) {
        return(DATA_ERR);
    }
    overvoltage = bcd2bin(buff[1])/10.0+bcd2bin(buff[2])*10.0;
    undervoltage = bcd2bin(buff[3])/10.0+bcd2bin(buff[4])*10.0;

    if (0x01 == buff[0]) {
        outletstateA.thrvol_over = overvoltage;
        outletstateA.thrvol_under = undervoltage;
    }else if (0x02 == buff[0]) {
        outletstateB.thrvol_over = overvoltage;
        outletstateB.thrvol_under = undervoltage;
    }
    
    //HAPthrvolNotify((int)buff[0], (double)overvoltage, (double)undervoltage);
    return(0);
}

static uint8 set_thrcur(uint8 *buff, uint8 w_len)
{
    float current = 0;
    if (w_len != 0x04) {
        return(DATA_ERR);
    }
    current = bcd2bin(buff[1])/1000.0+bcd2bin(buff[2])/10.0+bcd2bin(buff[3])*10.0;

    if (0x01 == buff[0]) {
        outletstateA.thrcur = current;
    }else if (0x02 == buff[0]) {
        outletstateB.thrcur = current;
    }
    //HAPthrcurNotify((int)buff[0], (double)current);
    return(0);
}

static uint8 set_thrpow(uint8 *buff, uint8 w_len)
{
    float power = 0;
    if (w_len != 0x04) {
        return(DATA_ERR);
    }
    power = bcd2bin(buff[1])/10000.0+bcd2bin(buff[2])/100.0+bcd2bin(buff[3]);

    if (0x01 == buff[0]) {
        outletstateA.thrpow = power;
    }else if (0x02 == buff[0]) {
        outletstateB.thrpow = power;
    }
    
    //HAPthrpowNotify((int)buff[0], (double)power);
    return(0);
}

static uint8 set_metqua(uint8 *buff, uint8 w_len)
{
    float tatalquan = 0;
    if (w_len != 0x05) {
        return(DATA_ERR);
    }
    tatalquan = bcd2bin(buff[1])/100.0+bcd2bin(buff[2])+bcd2bin(buff[3])*100+bcd2bin(buff[4])*10000;

    if (0x01 == buff[0]) {
        outletstateA.metqua = tatalquan;
    }else if (0x02 == buff[0]) {
        outletstateB.metqua = tatalquan;
    }
    
    //HAPmetquaNotify((int)buff[0], (double)tatalquan);
    return(0);
}

static uint8 set_alarmenable(uint8 *buff, uint8 max_len)
{
    return(0);
}

#define scat_flag(f) {(uint8)(f), (uint8)(f>>8)}
const struct func_ops func_items[]=
{
	//di,r_len,w_len, read, write
    {scat_flag(0x0001), get_device_type, NULL},
    {scat_flag(0x0002), get_soft_ver,    NULL},
    {scat_flag(0x0003), get_dev_infor,   NULL},
    {scat_flag(0x0005), get_dkey,        NULL},
    {scat_flag(0x0006), get_device_attribute,   NULL},
    {scat_flag(0x0007), get_sn,          NULL},
    {scat_flag(0x9010), NULL,          set_metqua},
    {scat_flag(0x9400), NULL,          set_hisdata},
    {scat_flag(0xB511), NULL,          set_thrvol},
    {scat_flag(0xB521), NULL,          set_thrcur},
    {scat_flag(0xB530), NULL,          set_thrpow},
    {scat_flag(0xB53F), NULL,          set_meter},
    {scat_flag(0xC011), NULL,          get_time},
    {scat_flag(0xC012), NULL,          set_onoff},
    {scat_flag(0xC003), NULL,          NULL},
    {scat_flag(0xC004), NULL,          NULL},
    {scat_flag(0xC005), NULL,          set_resetdefault},    
    {scat_flag(0xB624), NULL,          set_alarmenable}, 
};
#define	METER_ITEM_MAX		ARRAY_SIZE(func_items)
/**************************************
根据数据标志执行相应的功能
**************************************/
#define DATA_LEN(pframe)    (pframe->ctrl&0x7F)
uint8 set_parameter(uint8 data[], uint8 len)
{
	uint8 i,ret;
    struct FBD_Frame *pframe;
    uint8 *pw,*pr;

    pw = data;
    pr = &g_frame_buffer[MAX_BUFFER_SZ-1-1]-len;
    memmove_my(pr, pw, len);

    while ((len >= FBD_FRAME_HEAD))//&&(pw < pr)) 
    {
        pframe = (struct FBD_Frame *)pr;

        if(len <  FBD_FRAME_HEAD + DATA_LEN(pframe)) 
        {//ctrl 长度出错
            mymemcpy(pw, pframe, 2); 
            pw += 2;
            *(pw++) = 0x82;
            *(pw++) = LEN_ERR;
            *(pw++) = 0x00;
            break;
        }
        mymemcpy(pw, pr, FBD_FRAME_HEAD + DATA_LEN(pframe));
        pframe = (struct FBD_Frame *)pw;
        pw += 2;
        len -=FBD_FRAME_HEAD + DATA_LEN(pframe);
        pr += FBD_FRAME_HEAD + DATA_LEN(pframe);
        
        for(i=0; i< METER_ITEM_MAX; i++)
        {
            if(memcmp_my(pframe->did,func_items[i].di,2) == 0) break;
        }
        //printf("id : %d  did : 0x%02x%02x\n",i,pframe->did[1],pframe->did[0]);
        if((i >= METER_ITEM_MAX) || (NULL == func_items[i].write ))
        {
            *(pw++) = 0x82;
            *(pw++) = DID_ERR;
            *(pw++) = 0x00;
        }
        else
        {//调用函数，函数指针(各种控制命令等)
            ret = func_items[i].write(pframe->data, DATA_LEN(pframe));
            if(CHG_DID & ret)
            {//修改数据did
                ret &= 0x7f;
                mymemcpy(pframe, pframe->data, ret);
                ret -= 2;
                pw += ret;            
            }
            else if(NO_ERR != ret)
            {
                *(pw++) = 0x82;
                *(pw++) = ret;
                *(pw++) = 0x00;
            }
            else
            {
                ret = DATA_LEN(pframe)+1;//设置数据长度
                pw += ret;
            }
        }
    } 
    return(pw-data);
}

#define GROUP_LEN       0x3F
static uint8 is_gid_equal(uint8 data[])
{
    uint16 s_gid,t_gid;
    uint8 i,len_t,type_t;

    s_gid = eep_param.sid[1];
    s_gid <<= 8;
    s_gid += eep_param.sid[0];
    len_t = data[0]&GROUP_LEN;
    type_t = data[0]>>6;
    if(0x00 == type_t)
    {
        s_gid--;
        //if(len_t < s_gid>>3) return(0);
        if(len_t < (s_gid>>3)+1)return(0);//组地址长度(sid-1)/8+1-------2014.5.6
        if(data[(s_gid>>3)+1]&(0x01<<(s_gid&0x07))) return(1);
    }
    else
    {
        for(i = 1; i <= len_t; i++)
        { 
            t_gid = data[i];
            if(0x02 == type_t)
            { 
                if(len_t & 1) return(0);
                t_gid +=  data[i+1] << 8;
                i++; 
            }
             
            if((0x00 == t_gid) || (t_gid == s_gid))
            {
                return(1);
            }
        }
    }
    return(0);
}

uint8 set_group_parameter(uint8 data[], uint8 len)
{
    uint8 i,j,gid_len,fbd_len;
    struct FBD_Frame *pframe;

    j = 0;
    gid_len = (data[j] & GROUP_LEN) + 1;
    while (len >= (FBD_FRAME_HEAD + gid_len))
    {
        pframe = (struct FBD_Frame *)&data[j+ gid_len];
        fbd_len = DATA_LEN(pframe)+FBD_FRAME_HEAD + gid_len;
        if (len < fbd_len) break;

        if (is_gid_equal(&data[j]))
        {
            for (i=0; i< METER_ITEM_MAX; i++)
            {
                if (memcmp_my(pframe->did,func_items[i].di,2) == 0) break;
            }
            if ((i < METER_ITEM_MAX) && (NULL != func_items[i].write ))
            {//调用函数，函数指针(各种控制命令等)
                func_items[i].write(pframe->data, DATA_LEN(pframe));
            }
        }

        j += fbd_len;
        len -= fbd_len;

        gid_len = (data[j] & GROUP_LEN) + 1; 
    } 
    return(0);
}
/**************************************
根据数据标志执行相应的功能
**************************************/
uint8 read_parameter(uint8 data[], uint8 len)
{
    uint8 i, ret;
    struct FBD_Frame *pframe;
    uint8 *pw,*pr;

    pw = data;
    pr = &g_frame_buffer[MAX_BUFFER_SZ-1-1]-len;//最大可读数据长度,当数据返回长度刚刚好，cs无法发送出去问题？！
    memmove_my(pr, &data[0], len);

    while (len >= FBD_FRAME_HEAD) 
    {
        pframe = (struct FBD_Frame *)pr;

        if(len <  FBD_FRAME_HEAD + DATA_LEN(pframe)) 
        {//ctrl长度出错
            mymemcpy(pw, pframe, 2); 
            pw += 2;
            *(pw++) = 0x82;
            *(pw++) = LEN_ERR;
            *(pw++) = 0x00;
            break;
        }
        mymemcpy(pw, pr, FBD_FRAME_HEAD + DATA_LEN(pframe));
        pframe = (struct FBD_Frame *)pw;
        pw += 2;
        len -=FBD_FRAME_HEAD + DATA_LEN(pframe);
        pr += FBD_FRAME_HEAD + DATA_LEN(pframe);

        for(i=0; i<METER_ITEM_MAX; i++)
        {
            if(memcmp_my(pframe->did,func_items[i].di,2) == 0) break;
        }
        if((i >= METER_ITEM_MAX) || (func_items[i].read == NULL))
        {
            *(pw++) = 0x82;
            *(pw++) = DID_ERR;
            *(pw++) = 0x00;
        }
        else
        {
            ret= func_items[i].read(pw, (uint8)(pr-(pw+1)));
            if(0x00 == ret) //数据返回0，可能导致读取数据内容没有数据标识，数据长度为1的报文!?
            {
                pw -= 2;
                continue;
            }
            *(pw++) = ret;
            pw += (ret&0x7F);//最高位为错误标志位，返回的数据如果是带有错误标志位？！
        }
    }
    return(pw-data);
}

uint8 compare_soft_ver(uint8 *buff, uint8 len)
{
    return(memcmp_my((uint8 *)dev_infor, buff, strlen((char const *)dev_infor)));
}

static uint8 get_dkey(uint8 *buff, uint8 max_len)
{
#if 0
    struct ENCODE_PARAM encode_param;

    if(max_len < DKEY_LEN) return(0);

    EEP_Read(ENCODE_MAGIC_ADDR,(uint8 *)&encode_param, sizeof(struct ENCODE_PARAM));
    if(ENCODE_MAGIC == encode_param.sole_magic) 
    {
        mymemcpy(&buff[1], encode_param.dev.dkey, DKEY_LEN);
        return(DKEY_LEN);
    }

    buff[1] = DATA_ERR;
    buff[2] = 0x00;
#endif
    return(0x82);
}

uint8 _get_dev_infor(uint8* buff)
{
    mymemcpy(&buff[0], (uint8 *)dev_infor, strlen(dev_infor));
    return(strlen(dev_infor));
}

static uint8 get_device_attribute(uint8 *buff, uint8 max_len)
{
    uint8 len;
#if 0
    EEP_Read(ENCODE_PARAM_ADDR+ENCODE_LEN,(uint8 *)&len, 1);
    //if((len < 1) || (len > max_len)) return(0);
    if((len < 1) || (len > MAX_ATTRIBUTE_LEN)) 
    {
        buff[1] = DATA_ERR;
        buff[2] = 0x00;
        return(0x82);
    }

    EEP_Read(ENCODE_PARAM_ADDR+offset_of(struct DEV_INFOR, infor),(uint8 *)&buff[1], len);
#endif
    return(len);
}

static uint8 get_sn(uint8 *buff, uint8 max_len)
{
#if 0
    struct ENCODE_PARAM encode_param;

    if(max_len < SN_LEN) return(0);

    EEP_Read(ENCODE_MAGIC_ADDR,(uint8 *)&encode_param, sizeof(struct ENCODE_PARAM));
    if(ENCODE_MAGIC == encode_param.sole_magic) 
    {
        mymemcpy(&buff[1], encode_param.dev.sn, SN_LEN);
        return(SN_LEN);
    }

    buff[1] = DATA_ERR;
    buff[2] = 0x00;
#endif
    return(0x82);
}