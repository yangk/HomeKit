#ifndef EXAMPLE_SSL_DOWNLOAD_H
#define EXAMPLE_SSL_DOWNLOAD_H

void example_ssl_download(void);
void Jiguang_Push(int mode);
void get_ntp_time();
int report_hisdata_to_iot(uint8_t data1, uint8_t data2, struct tm timeinfo);
int report_alarm_to_iot(outlet_state outletstate);
#endif /* EXAMPLE_SSL_DOWNLOAD_H */
