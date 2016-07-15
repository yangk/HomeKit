#ifndef PHYTREX_CLOCK_H
#define PHYTREX_CLOCK_H

#include <time.h>

#define ALARM_COUNT		10

//typedef void (*alarm_event_ptr)(void);

typedef struct {
    char hostname[16];
    char port[4];
    char interval[16];
    char offset[4];
} PhytrexNtpServer;

typedef struct {
	uint8_t id;
    time_t time; // GMT+0
	uint8_t repeat;
    uint32_t action;
	uint8_t on;
	uint8_t already;
    //alarm_event_ptr p_alarm_cb;
} PhytrexClockAlarm;

void phytrex_clock_init(int msg_flag);
PhytrexNtpServer *phytrex_clock_get_ntp_info();
void phytrex_clock_set_ntp_info(PhytrexNtpServer ntp);
time_t phytrex_clock_get_time();
struct tm *phytrex_clock_get_time_info();
char *phytrex_clock_print_time(time_t timet);
void phytrex_clock_free();
void phytrex_FlashClockRead(PhytrexClockAlarm *pCA, int len);
void phytrex_FlashClockWrite(PhytrexClockAlarm *pCA, int len);
void phytrex_alarm_action(PhytrexClockAlarm CA);

#endif