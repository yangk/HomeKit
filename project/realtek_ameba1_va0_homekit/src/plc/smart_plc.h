#ifndef	_SMART_PLC_H_
#define	_SMART_PLC_H_
#include "config.h"

/***********************命令控制字************************/
#define     _ACK            0x00
#define     _NACK           0xFF
#define     CMD_SET_AID     0x01
#define     CMD_GET_AID     0x03
#define     CMD_ACK_AID     0x13
#define     CMD_DEL_AID     0x04
#define     CMD_REQ_AID     0x05
#define     CMD_GET_SID     0x06
#define     CMD_ACK_SID     0x16
#define     CMD_GET_EID     0x07
#define     CMD_ACK_EID     0x17
#define     CMD_SET_BPS     0x08
#define     CMD_SET_REG     0x09
#define     CMD_UNLINK      0x0A
#define     CMD_REGINFOR    0x0B
#define     CMD_SET_PANID   0x0C
#define     CMD_GET_GWAID   0x0D

#define     CMD_ACK         0x00
#define     CMD_NAK         0xFF

#define     CMD_SET         0x07
#define     CMD_READ        0x02
#define     CMD_UPDATE      0x05
#define     CMD_NOTIFY      0x01
#define     CMD_ALARM      0x00

#define     FRAME_INFOR     0x08
/************************************注册状态机********************************/
/*********************************************************************************
上电mcu对载波芯片需要设置AID、注册属性(默认使用密码注册)、网关PANID(mcu保存的有效网关地址)
*******************************************************************************/
enum
{
    INVALID=0, RST_PLC, R_EID, S_AID, S_PWDREG, S_PANID, S_BPS, 
    S_REG, UNLINK1, PWD_ERR, UNLINK2, G_GWID, G_SID, _END
};

enum
{
    OUTLET_A_ON=0, OUTLET_A_OFF, OUTLET_B_ON, OUTLET_B_OFF,SET_DATE,
    OUTLET_A_OVERVOL,OUTLET_B_OVERVOL,OUTLET_A_UNDERVOL,OUTLET_B_UNDERVOL,
    OUTLET_A_CUR,OUTLET_B_CUR,OUTLET_A_POW,OUTLET_B_POW,OUTLET_A_CALIBRATE,
    OUTLET_B_CALIBRATE
};

/**************error word***************/
#define    NO_ERR        0x00
#define    OTHER_ERR     0x0F
#define    LEN_ERR       0x01
#define    BUFFER_ERR    0x02
#define    DATA_ERR      0x03
#define    DID_ERR       0x04
#define    DEV_BUSY      0x05
#define    CHG_DID       0x80

//#define    REQ_ERR       0xFF     

/****************register type**********/
#define    PRESSKEY_REG     0x01
#define    PASSWORD_REG     0x00
#define    PASSWORD_ERR     0x02


/*******************数据有效标志*******************/
#define     VALID_DATA      0x05
#define     INVALID_DATA    0x0A

#define   _OFF    0x00
#define   _ON     0x01

#define STC             0x7e
#define     PANID_LEN       0x02
#define     PWD_LEN         0x02
#define     ID_LEN          0x04
#define     SID_LEN         0x02
#define     GID_LEN         0x20


struct PLC_STATE 
{
    uint8 cur_state;
    uint8 next_state;

    uint8 (*action)(uint8 init, void *args);                            
};

struct PLC_MACHINE
{
    uint8 init;
    uint8 trycnt;
    uint8 wait_t;
//    uint8 state;
    struct PLC_STATE *pstate;
};


struct REG
{
    uint8 type;
    uint8 last_status;
    uint8 wait_t;
};
struct UPDATE
{
    uint8 seq[2];
    uint8 ack;
    uint8 crc[2];
    uint8 len;
};
struct UPDATE_FILE
{
    uint8 file_sz[4];
    uint8 ver[2];
    uint8 blk_sz;
    uint8 type[8];
};

#define UPDATE_LEN    sizeof(struct UPDATE)
#define UPDATE_FILE_LEN    sizeof(struct UPDATE_FILE)
#define FIRST_PAC_LEN    1+UPDATE_LEN+UPDATE_FILE_LEN  // 1 : cmd = 0x05
#define OTA_BLK_SZ    40
#define DATA_PAC_LEN    1+UPDATE_LEN+OTA_BLK_SZ  // 1 : cmd = 0x05

#define little_bytes_to_word(byte)  ((uint16)(byte[1]<<8)+byte[0])
#define word_to_little_bytes(word,byte)  byte[0]=word&0xFF; byte[1]=(word>>8);
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

//#define set_bit(x, bit) 	do{uint8 cpu_sr; OS_ENTER_CRITICAL();(x) |= 1 << (bit);OS_EXIT_CRITICAL();}while(0)
//#define reset_bit(x, bit) 	do{uint8 cpu_sr; OS_ENTER_CRITICAL();(x) &= ~(1 << (bit));OS_EXIT_CRITICAL();}while(0)
//#define is_bit_set(x, bit) 	((x) & (1 << (bit)))

extern uint16 task_monitor;

#define notify(task_bit)		//set_bit(task_monitor, task_bit)
#define is_task_set(task_bit)	//is_bit_set(task_monitor, task_bit)
#define is_task_always_alive(flags)	(flags & ALWAYS_ALIVE)
#define reset_task(task_bit) 	//reset_bit(task_monitor, task_bit)

enum
{
    EV_CLRDOG,
	EV_RXCHAR,
    EV_REPORT,
    EV_20MS,
    EV_100MS,
    EV_SEC,
    EV_MIN,
    EV_KEY,
    EV_TICK,
    EV_STATE
};
enum
{
	ALWAYS_ALIVE = 0x01
};
struct task
{
    uint8 id;
	uint8 flags;
    void *args;
    void (*handle)(void *args);
};

extern struct PLC_MACHINE  plc_state;
extern struct EEP_PARAM   eep_param;
extern uint8 g_frame_buffer[MAX_BUFFER_SZ];

void task_handle(void);
void system_init(void);
void clr_watchdog(void *args);
uint8 creatFristPacket();
void getMCU_ver();
void getMCU_onoff(uint8 cc);
void getMCU_meterage(uint8 cc);
void getMCU_thrpow(uint8 cc);
void getMCU_thrcur(uint8 cc);
void getMCU_thrvol(uint8 cc);
//void init_frame_head(struct SHS_frame * pframe);
#endif
