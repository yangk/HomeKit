#include "model.h"
#if MODEL == OUTLET
#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "hal_timer.h"
#include "timer_api.h"
#include "serial_api.h"
#include "sleep_ex_api.h"
#include <homekit/HAPServiceProfile.h>
#include <wifi/wifi_conf.h> //alexyi
#include <lwip_netconf.h>
#include <phytrex_homekit_flow.h>
#include <homekit/HAPAccessory.h>
#include <homekit/HAPAccessoryAPI.h>
#include <homekit/HAPServiceProfile.h>
#include <WACServer/WAC.h>
#include "phytrex_model.h"
#include "phytrex_update.h"
#include <polarssl/ssl.h>
#include "flash_api.h"
#include "uart_socket.h"
#include "smart_plc.h"
#include "update.h"



HomekitFlow_t hk_flow =
{
    .Wac = {
        .name = HK_NAME,
        .manufacturer = HK_MANUFACTURER,
        .model = HK_MODEL,
		.fw_ver = PHYTREX_SDK_VERSION,
		.hw_ver = HARDWARE_REVISION,
		.sn = SERIAL_NUMBER,
        .swap_name = HK_SWAP_NAME,
		.ea_protocols = {
			[0] = "com.haier.*",
		},
		.num_ea_protocols = 1,
		.bundle_seed_id = "PP27UD8NYZ",
    },
    .Hap = {
        .name = HK_NAME,
        .model = HK_MODEL
    }
};

#define BUF_SIZE		FLASH_DATA_LEN

//OTA Version resource
//#define OTA_VER_HOST				"s3.amazonaws.com"
//#define OTA_VER_PATH				"/rickysoung/FOTA/VERSION"
#define OTA_VER_PORT				443
#define OTA_TIMEOUT    				20*configTICK_RATE_HZ//ms

#define STACKSIZE               1024
void led_blink(uint32_t value, int timeout);

extern uint32_t rtw_join_status;

cJSON *MyDB = NULL;
cJSON *MyAccessory = NULL;
cJSON *MyOutletService1 = NULL,
      *MyOutletService2 = NULL;
#if FIRST_VERSION
cJSON *MyMeterService1 =NULL,
      *MyMeterService2 =NULL;
#endif

extern HomekitFlow_t hk_flow;
extern cJSON *MyOtaService;
extern cJSON *MySysService;

outlet_state outletstateA;
outlet_state outletstateB;
outlet_state outletstateA_app;
outlet_state outletstateB_app;
extern uart_socket_t *uart_socket;
static char Hisdata[HISDATABASE64LEN];

gtimer_t ledTmr;
gtimer_t btnTmr;
gpio_t gpio_rstbtn;
gpio_t relayFB1;
gpio_t relayFB2;
gpio_t relayCtr1;
gpio_t relayCtr2;
gpio_t identy_led;
gpio_t manualSW1;
gpio_t manualSW2;
gpio_t OlInUse1;
gpio_t OlInUse2;
gpio_irq_t gpio_oiu1;
gpio_irq_t gpio_oiu2;
int led_timecnt;
extern HomekitFlow_t hk_flow;
extern PhytrexParameter_t ex_param;
extern MyVersion cur_ver;

extern BOOL phytrex_get_latest_version(phytrex_update_cfg_cloud_t *res);

//------------------------------------------------------------------//
BOOL phytrex_get_latest_version(phytrex_update_cfg_cloud_t *res)
{
	int srv_fd = -1;
	ssl_context ssl = {0};
	int ret = 0;
	int result = 0;
	char fw_ver[32] = "",
	     sw_ver[32] = "";
	int method = -1;
	int buf_size = 0;
	int len = 0;
	
	unsigned char *buf = pvPortMalloc(BUF_SIZE);
	if(!buf){
	    printf("\n\r[%s] Alloc buffer failed", __FUNCTION__);
	    goto exit;
	}
	memset(buf, 0, BUF_SIZE);
	
	method = phytrex_parse_path(res);
	
	if(method == 0) {
#if 1
		printf("Not accept HTTP protocol\r\n");
		goto exit;
#else
		int read_size = 0,
		    resource_size = 0,
		    header_removed = 0,
		    content_len = 0,
		    crc32 = 0;
		char *body = NULL,
		     *fw_ver_pos = NULL, *hw_ver_pos = NULL, *sw_ver_pos = NULL,
		     *crc32_pos = NULL,
		     *content_len_pos = NULL;
		char *header = NULL;
#if 0
		if((ret = net_connect(&srv_fd, res->host_name, 80)) != 0) {
			printf("ERROR: net_connect ret(%d)\n", ret);
			goto method_0_exit;
		}
#else
		if((srv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("ERROR: Create socket fail\n");
			goto method_0_exit;
		}
		
		struct sockaddr_in srv_addr = {0};
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_len = sizeof(srv_addr);
		srv_addr.sin_addr.s_addr = inet_addr(res->host_name);
		srv_addr.sin_port = htons(80);
		
		if((ret = connect(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr))) < 0) {
			printf("ERROR: Connect fail\n");
			goto method_0_exit;
		}
#endif
		buf_size = sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",  res->file_path, res->host_name);

		send(srv_fd, buf, buf_size, 0);
		
		net_set_nonblock(srv_fd);
		
		TickType_t start, end;
		start = end = xTaskGetTickCount();
		while(1) {
			end = xTaskGetTickCount();
			if(fabs(end-start) > OTA_TIMEOUT) {
				printf("\n\r[%s] OTA Update Timeout %d", __FUNCTION__, end-start);
				goto method_1_exit;
			}
			memset(buf, 0, BUF_SIZE);
			read_size = recv(srv_fd, buf, BUF_SIZE, 0);
			if(read_size <= 0)
			    continue;
			start = xTaskGetTickCount();
			
			if(header_removed == 0) {
				header = strstr(buf, "\r\n\r\n");
				if(header) {
					body = header + strlen("\r\n\r\n");
					*(body - 2) = 0;
					header_removed = 1;
					printf("\n<<HTTP Header>>\n%s\n", buf);
					read_size = read_size - (body - buf);
					
					//data size
                    content_len_pos = strstr(buf, "Content-Length: ");
                    if(content_len_pos) {
                          content_len_pos += strlen("Content-Length: ");
                          sscanf(content_len_pos, "%d", &content_len);
                    }
                    else {
                          printf("ERROR: Didn't find \"Content-Length\"\n");
                          goto method_0_exit;
                    }
                }
                else {
                    printf("ERROR: HTTP header\n");
                        goto method_0_exit;
                }
            }
			
			resource_size += read_size;
                        printf("\rUpdate file size = %d/%d bytes  %3.1f %%",
                        resource_size, content_len, (float)resource_size/(float)content_len*100.0f);
            
                        if(resource_size >= content_len)
                            break;
		}
		
	method_0_exit:
		if(srv_fd != -1) {
			net_set_block(srv_fd);
			net_close(srv_fd);
		}
#endif
	}
	else if(method == 1){
		if((ret = net_connect(&srv_fd, res->host_name, OTA_VER_PORT)) != 0) {
			printf("ERROR: net_connect ret(%d)\n", ret);
			goto method_1_exit;
		}
		
		if((ret = ssl_init(&ssl)) != 0) {
			printf("ERRPR: ssl_init ret(%d)\n", ret);
			goto method_1_exit;
		}
		
		ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
		ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
		ssl_set_rng(&ssl, phytrex_my_random, NULL);
		ssl_set_bio(&ssl, net_recv, &srv_fd, net_send, &srv_fd);
		
		if((ret = ssl_handshake(&ssl)) != 0) {
			printf("ERROR: ssl_handshake ret(-0x%x)", -ret);
			goto method_1_exit;
		}
		else {
			int header_removed = 0;
			char *body = NULL,
				*fw_ver_pos = NULL,
				*sw_ver_pos = NULL,
				*crc32_pos = NULL,
				*path_pos = NULL;
			char *header = NULL;
			
			printf("SSL ciphersuite %s\n", ssl_get_ciphersuite(&ssl));
			sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", res->file_path, res->host_name);
			ssl_write(&ssl, buf, strlen(buf));
			
			net_set_nonblock(srv_fd);
			
			TickType_t start, end;
			start = end = xTaskGetTickCount();
			while(header_removed == 0) {
				end = xTaskGetTickCount();
				if(fabs(end-start) > OTA_TIMEOUT) {
					printf("\n\r[%s] OTA Update Timeout %d", __FUNCTION__, end-start);
					goto method_1_exit;
				}
				if(ssl_read(&ssl, buf, BUF_SIZE) <= 0)
					continue;
				start = xTaskGetTickCount();
				
				if(header_removed == 0) {
					header = strstr(buf, "\r\n\r\n");
					if(header) {
						body = header + strlen("\r\n\r\n");
						*(body - 2) = 0;
						header_removed = 1;
						printf("\n<<HTTP Header>>\n%s\n", buf);

						//crc32
						crc32_pos = strstr(buf, "x-amz-meta-crc32: ");
						if(crc32_pos) {
							crc32_pos += strlen("x-amz-meta-crc32: ");
							sscanf(crc32_pos, "%x\r\n", &res->crc32);
						}
						
						//path
						path_pos = strstr(buf, "x-amz-meta-path: ");
						if(path_pos) {
							path_pos += strlen("x-amz-meta-path: ");
							memset(res->full_path, 0, sizeof(res->full_path));
							sscanf(path_pos, "%s\r\n", res->full_path);
						}
						else
							goto method_1_exit;
						
						//firmware
						fw_ver_pos = strstr(buf, "x-amz-meta-firmware-revision: ");
						if(fw_ver_pos) {
							fw_ver_pos += strlen("x-amz-meta-firmware-revision: ");
							sscanf(fw_ver_pos, "%s\r\n", fw_ver);
#if 0
							//maybe 4.3.11.100000 v.s. 4.3.11
							if(strstr(cur_ver.firmware, fw_ver) == NULL)
#else
                                              printf("1result : %d\n",result);
							if(phytrex_version_compare(cur_ver.firmware, fw_ver) < 0)
#endif
								result |= 1;
						}
						
						//software
						sw_ver_pos = strstr(buf, "x-amz-meta-software-revision: ");
						if(sw_ver_pos) {
							sw_ver_pos += strlen("x-amz-meta-software-revision: ");
							sscanf(sw_ver_pos, "%s\r\n", sw_ver);
#if 0
							if(memcmp(cur_ver.software, sw_ver, sizeof(cur_ver.software)) != 0)
#else
                                                printf("2result : %d\n",result);
							if(phytrex_version_compare(cur_ver.software, sw_ver) < 0)
#endif
								result |= 1;
						}
					}
					else {
						printf("ERROR: HTTP header\n");
					}
				}
			}
		}

	method_1_exit:
		if(srv_fd != -1) {
			net_set_block(srv_fd);
			net_close(srv_fd);
		}
		ssl_free(&ssl);
	}
	
exit:
        printf("3result : %d\n",result);
	if(buf)
		vPortFree(buf);
	
	return result;
}
//------------------------------------------------------------------//
void print_data(uint8 *buf,int len)
{
    for (int i=0;i<len;i++)
    {
        printf("%02x ", buf[i]);
        if ((i+1)%(4*24) == 0) printf("\n");
    }
    printf("\n");
}
void ModelIdentifyRoutine(void)
{
  //led_blink(1000000, 7);
  for(uint8_t i=0; i<6; i++){
    gpio_write(&identy_led, !gpio_read(&identy_led));
    osDelay(500);
  }
}

void HAPstatusNotify()
{
    printf("\n=====PhytrexHomekitNotifyHapStart====\n");
}

void HAPoiu1Notify(int value)
{
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(MyOutletService1, kCharacteristicUUID_OutletInUse);
    current_valueJSObject->valueint = value;
    HAPUpdateServiceCharacteristic(MyOutletService1, kCharacteristicUUID_OutletInUse, cJSON_CreateNumber(current_valueJSObject->valueint));
    printf("notify:%d\n",value);
}

void HAPoiu2Notify(int value)
{
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(MyOutletService2, kCharacteristicUUID_OutletInUse);
    current_valueJSObject->valueint = value;
    HAPUpdateServiceCharacteristic(MyOutletService2, kCharacteristicUUID_OutletInUse, cJSON_CreateNumber(current_valueJSObject->valueint));
    printf("notify:%d\n",value);
}

void HAPmsw1Notify(int value)
{
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(MyOutletService1, kCharacteristicUUID_On);
    current_valueJSObject->valueint = value;
    HAPUpdateServiceCharacteristic(MyOutletService1, kCharacteristicUUID_On, cJSON_CreateBool(current_valueJSObject->valueint));
}

void HAPmsw2Notify(int value)
{
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(MyOutletService2, kCharacteristicUUID_On);
    current_valueJSObject->valueint = value;
    HAPUpdateServiceCharacteristic(MyOutletService2, kCharacteristicUUID_On, cJSON_CreateBool(current_valueJSObject->valueint));
}

#if 0
int phytrex_MainHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject)
{
    int status = kHAPStatus_InvalidValue;
    cJSON *accessoryJSObject, *serviceJSObject, *characteristicJSObject;
    
    accessoryJSObject = HAPSearchDatabaseAID(pDB, aid);
    characteristicJSObject = HAPSearchDatabaseIID(pDB, accessoryJSObject, iid);
    serviceJSObject = HAPSearchCharacteristicService(pDB, characteristicJSObject);
        
    // Handle for Outlet
    if((accessoryJSObject == pAccessory) &&
       (serviceJSObject == MyOutletService1 || serviceJSObject == MyOutletService2)) {
            char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;
            if(serviceJSObject == MyOutletService1) {
                if(strcmp(characteristic_type, kCharacteristicUUID_On) == 0) {
                  if((valueJSObject->valueint == 0)||(valueJSObject->valueint == 1)) {
                        printf("\nOutlet1::ON = %d\n", valueJSObject->valueint);
                        relay1(valueJSObject->valueint);
                        osDelay(100);
                        //if(gpio_read(&relayFB1) == 1)
                        //  g_status.Outlet1.mode = 0;
                        //else
                        //  g_status.Outlet1.mode = 1;
                        if(valueJSObject->valueint == 1)
                          g_status.Outlet1.mode = 1;
                        else
                          g_status.Outlet1.mode = 0;
                        
                        // Update target characteristic value
                        if(HAPUpdateServiceCharacteristic(MyOutletService1, kCharacteristicUUID_On, cJSON_CreateBool(g_status.Outlet1.mode)) == kHAPStatus_Failure)
                          status = kHAPStatus_InvalidValue;
                        else {
                          phytrex_FlashUserWrite(&g_status, sizeof(OLT_Status_t));
                          status = kHAPStatus_Success;
                        }
                  }
                }
            }
            else if(serviceJSObject == MyOutletService2) {
                if(strcmp(characteristic_type, kCharacteristicUUID_On) == 0) {
                  if((valueJSObject->valueint == 0)||(valueJSObject->valueint == 1)) {
                        printf("\nOutlet2::ON = %d\n", valueJSObject->valueint);
                        relay2(valueJSObject->valueint);
                        osDelay(100);
                        //if(gpio_read(&relayFB2) == 1)
                        //  g_status.Outlet2.mode = 0;
                        //else
                        //  g_status.Outlet2.mode = 1;
                        if(valueJSObject->valueint == 1)
                          g_status.Outlet2.mode = 1;
                        else
                          g_status.Outlet2.mode = 0;
                        
                        // Update target characteristic value
                        if(HAPUpdateServiceCharacteristic(MyOutletService2, kCharacteristicUUID_On, cJSON_CreateBool(g_status.Outlet2.mode)) == kHAPStatus_Failure)
                          status = kHAPStatus_InvalidValue;
                        else {
                          phytrex_FlashUserWrite(&g_status, sizeof(OLT_Status_t));
                          status = kHAPStatus_Success;
                        }
                  }
                }
            }
    }
    
    return status;
}
#endif

void HAPoutletNotify(int channel, int value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kCharacteristicUUID_On);
    current_valueJSObject->valueint = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kCharacteristicUUID_On, cJSON_CreateNumber(current_valueJSObject->valueint));
    //ret = HAPUpdateServiceCharacteristic(current_service, kCharacteristicUUID_On, current_valueJSObject);
    printf("notify:%d\n",ret);
}

void HAPmetquaNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Met_Total_Quantity);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Met_Total_Quantity, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPthrpowNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Thr_Power);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Thr_Power, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPthrcurNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Thr_Current);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Thr_Current, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPthrvolNotify(int channel, double overvol, double undervol)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Thr_overVoltage);
    current_valueJSObject->valuedouble = overvol;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Thr_overVoltage, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);

    current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Thr_underVoltage);
    current_valueJSObject->valuedouble = undervol;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Thr_underVoltage, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPmetvolNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Met_Voltage);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Met_Voltage, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPmetcurNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Met_Current);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Met_Current, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

void HAPmetpowNotify(int channel, double value)
{
    int ret;
    cJSON *current_service = NULL;
    if (1 == channel){
        current_service = MyOutletService1;
    }else if (2 == channel){
        current_service = MyOutletService2;
    }else{
        return;
    }
    
    cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(current_service, kChaUUID_Met_Power);
    current_valueJSObject->valuedouble = value;
    printf("notify\n");
    ret = HAPUpdateServiceCharacteristic(current_service, kChaUUID_Met_Power, cJSON_CreateNumber(current_valueJSObject->valuedouble));
    printf("notify:%d\n",ret);
}

int write_timerdata(outlet_state *outletstate, cJSON *valJSObj)
{
    int outlen = OUTLET_TIMER_ONELEN;
    int i;
    flash_t flash;
    int timeraddr;
    uint8_t timerdatabuf[OUTLET_TIMER_TWOLEN];
    char valbuf[OUTLET_TIMER_ONELEN];
    
    timeraddr = OUTLET_TIMER_ADDR;
    memset(timerdatabuf, 0, OUTLET_TIMER_TWOLEN);
    memset(valbuf, 0xFF, OUTLET_TIMER_ONELEN);
    
    base64_decode(valbuf, &outlen, valJSObj->valuestring, strlen(valJSObj->valuestring));
    
    flash_stream_read(&flash, timeraddr, OUTLET_TIMER_TWOLEN, timerdatabuf);
    
    if (outletstate == (&outletstateA))
    {
        memcpy(timerdatabuf, valbuf, OUTLET_TIMER_ONELEN);
    }
    else
    {
        memcpy(timerdatabuf+OUTLET_TIMER_ONELEN, valbuf, OUTLET_TIMER_ONELEN);
    }
    printf("\r\n");
    for (i=0; i<OUTLET_TIMER_TWOLEN; i++)
    {
        printf("%x",timerdatabuf[i]);
    }
    printf("\r\n");

    //time_t t;
    //t = rtc_read();
    flash_erase_sector(&flash, timeraddr);
    flash_stream_write(&flash, timeraddr, OUTLET_TIMER_TWOLEN, timerdatabuf);
    //flash_stream_write(&flash, timeraddr+OUTLET_TIMER_TWOLEN, sizeof(t), &t);
    return 0;
}

int write_OutletA_handler(const char *cha_type, cJSON *valJSObj)
{
    if (strcmp(cha_type, kCharacteristicUUID_On) == 0)
    {
        if(valJSObj->valueint == 0)
        {
            outletstateA.on = 0;
            outletstateA_app.on = outletstateA.on;
            app_handle(OUTLET_A_OFF, NULL);
        }
        else if(valJSObj->valueint == 1)
        {
            outletstateA.on = 1;
            outletstateA_app.on = outletstateA.on;
            app_handle(OUTLET_A_ON, NULL);
        }
        //HAPoutletNotify(1,outletstateA_app.on);
        // Update target characteristic value
        HAPUpdateServiceCharacteristic(MyOutletService1, cha_type, cJSON_Duplicate(valJSObj, 0));
    }
    else if (strcmp(cha_type, kChaUUID_Timing_On) == 0)
    {
        printf("Timing_On_Data : len : %d : %s\n", strlen(valJSObj->valuestring), valJSObj->valuestring);
        write_timerdata(&outletstateA, valJSObj);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_overVoltage) == 0)
    {
        printf("Thr_Voltage : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_A_OVERVOL, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_underVoltage) == 0)
    {
        printf("Thr_Voltage : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_A_UNDERVOL, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Current) == 0)
    {
        printf("Thr_Current : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_A_CUR, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Power) == 0)
    {
        printf("Thr_Power : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_A_POW, &(valJSObj->valuedouble));
    }
    return kHAPStatus_Success;
}

int write_OutletB_handler(const char *cha_type, cJSON *valJSObj)
{
    if (strcmp(cha_type, kCharacteristicUUID_On) == 0)
    {
        if(valJSObj->valueint == 0)
        {
            outletstateB.on = 0;
            outletstateB_app.on = outletstateB.on;
            app_handle(OUTLET_B_OFF, NULL);
        }
        else if(valJSObj->valueint == 1)
        {
            outletstateB.on = 1;
            outletstateB_app.on = outletstateB.on;
            app_handle(OUTLET_B_ON, NULL);
        }
        // Update target characteristic value
        HAPUpdateServiceCharacteristic(MyOutletService2, cha_type, cJSON_Duplicate(valJSObj, 0));
    }
    else if (strcmp(cha_type, kChaUUID_Timing_On) == 0)
    {
        printf("Timing_On_Data : len : %d : %s\n", strlen(valJSObj->valuestring), valJSObj->valuestring);
        write_timerdata(&outletstateB, valJSObj);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_overVoltage) == 0)
    {
        printf("Thr_Voltage : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_B_OVERVOL, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_underVoltage) == 0)
    {
        printf("Thr_Voltage : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_B_UNDERVOL, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Current) == 0)
    {
        printf("Thr_Current : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_B_CUR, &(valJSObj->valuedouble));
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Power) == 0)
    {
        printf("Thr_Power : type : %d : val : %f\n", valJSObj->type, valJSObj->valuedouble);
        app_handle(OUTLET_B_POW, &(valJSObj->valuedouble));
    }
    return kHAPStatus_Success;
}
int write_Ota_handler(const char *cha_type, cJSON *valJSObj)
{
    if (strcmp(cha_type, kChaUUID_WF_Ota_Data) == 0)
    {
        //printf("WF_Ota_Data : len : %d : %s\n", strlen(valJSObj->valuestring), valJSObj->valuestring);
        ES_phytrex_update_ota_amazon_cloud_task(valJSObj->valuestring, strlen(valJSObj->valuestring));
    }
    else if (strcmp(cha_type, kChaUUID_MCU_Ota_Data) == 0)
    {
        printf("MCU_Ota_Data : len : %d : %s\n", strlen(valJSObj->valuestring), valJSObj->valuestring);
        ES_mcu_ota_task(valJSObj->valuestring, strlen(valJSObj->valuestring));
    }
    return kHAPStatus_Success;
}
int write_System_handler(const char *cha_type, cJSON *valJSObj)
{
    if (strcmp(cha_type, kChaUUID_Time) == 0)
    {
        printf("Time_Data : len : %d : %s\n", strlen(valJSObj->valuestring), valJSObj->valuestring);
        set_locoltime(valJSObj->valuestring);
        app_handle(SET_DATE, NULL);
    }
    else if (strcmp(cha_type, kChaUUID_Reset) == 0)
    {
        printf("Reset_Data : type : %d : int : %d\n", valJSObj->type, valJSObj->valueint);
    }
    else if (strcmp(cha_type, kChaUUID_Status) == 0)
    {
        if(valJSObj->valueint == 1)
        {
            app_handle(OUTLET_A_CALIBRATE, NULL);
        }
        else if(valJSObj->valueint == 0)
        {
            app_handle(OUTLET_B_CALIBRATE, NULL);
        }
    }
    return kHAPStatus_Success;
}
int ES_phytrex_MainHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject)
{
    int status = kHAPStatus_InvalidValue;
    cJSON *accessoryJSObject, *serviceJSObject, *chaJSObj;
    
    accessoryJSObject = HAPSearchDatabaseAID(pDB, aid);
    chaJSObj = HAPSearchDatabaseIID(pDB, accessoryJSObject, iid);
    serviceJSObject = HAPSearchCharacteristicService(pDB, chaJSObj);
        
    // Handle for Outlet
    if ((accessoryJSObject == pAccessory) &&
       (serviceJSObject == MyOutletService1 || 
        serviceJSObject == MyOutletService2 ||
        serviceJSObject == MyOtaService||
        serviceJSObject == MySysService))
    {
        char *cha_type = cJSON_GetObjectItem(chaJSObj, kChaObj_Type)->valuestring;
        if (serviceJSObject == MyOutletService1)
        {
            write_OutletA_handler(cha_type, valueJSObject);
        }
        if (serviceJSObject == MyOutletService2)
        {
            write_OutletB_handler(cha_type, valueJSObject);
        }
        if (serviceJSObject == MyOtaService)
        {
            #if FIRST_VERSION
            return status;
            #else
            write_Ota_handler(cha_type, valueJSObject);
            #endif
        }
        if (serviceJSObject == MySysService)
        {
            write_System_handler(cha_type, valueJSObject);
        }
        status = kHAPStatus_Success;
    }
    return status;
}
int phytrex_AccessoryOperationHandler(int aid, int iid, cJSON *valueJSObject)
{
    int status = kHAPStatus_InvalidValue;
    
    if(ES_phytrex_MainHandler(MyDB, MyAccessory, aid, iid, valueJSObject) == kHAPStatus_Success)
        status = kHAPStatus_Success;
    else if(phytrex_OtherHandler(MyDB, MyAccessory, aid, iid, valueJSObject) == kHAPStatus_Success)
        status = kHAPStatus_Success;
    
    return status;
}

int read_handler(int aid, int iid, cJSON **outValue)
{
    static int number=0;
    cJSON *accessoryJSObject, *serviceJSObject, *characteristicJSObject;
    
    accessoryJSObject = HAPSearchDatabaseAID(MyDB, aid);
    characteristicJSObject = HAPSearchDatabaseIID(MyDB, accessoryJSObject, iid);
    serviceJSObject = HAPSearchCharacteristicService(MyDB, characteristicJSObject);
    cJSON *value = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Value);
    
    if((accessoryJSObject == MyAccessory) &&
       (serviceJSObject == MyOutletService1 ||
        serviceJSObject == MyOutletService2)) {
        char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;
        if(serviceJSObject == MyOutletService1) {
            //printf("\nOutlet1:");
        }
        else if(serviceJSObject == MyOutletService2) {
            //printf("\nOutlet2:");
        }
    }
    *outValue = cJSON_Duplicate(value, 0);
exit:
    return kHAPStatus_Success;
}

int checkdatanum(char *characteristic_type)
{
    if(strcmp(characteristic_type, kChaUUID_1st_Hisdata) == 0) return 1;
    if(strcmp(characteristic_type, kChaUUID_2nd_Hisdata) == 0) return 2;
    if(strcmp(characteristic_type, kChaUUID_3rd_Hisdata) == 0) return 3;
    if(strcmp(characteristic_type, kChaUUID_4th_Hisdata) == 0) return 4;
    if(strcmp(characteristic_type, kChaUUID_5th_Hisdata) == 0) return 5;
    if(strcmp(characteristic_type, kChaUUID_6th_Hisdata) == 0) return 6;
    if(strcmp(characteristic_type, kChaUUID_7th_Hisdata) == 0) return 7;
    if(strcmp(characteristic_type, kChaUUID_8th_Hisdata) == 0) return 8;
    return 0;
}

#if HISDATA_15_MIN
int read_hisdata(cJSON *serviceJSObject, char *characteristic_type)
{
    int datanum, outlen2;
    int outlen = HISDATALEN;
    flash_t flash;
    int hisaddr,offset;
    uint8 hisdatabuf[HISDATALEN];
    struct tm *timeinfo;
    int tm_year,tm_mon;
    uint8_t data[3];
    int addstate = 0;

    read_locoltime(timeinfo);
    tm_year = timeinfo->tm_year;
    tm_mon = timeinfo->tm_mon;
    
    datanum = checkdatanum(characteristic_type);
    if ((datanum == 0) || (tm_year < 2000)) return -1;
    hisaddr = (serviceJSObject == MyOutletService1) ? (OUTLET_A_HISADDR) : (OUTLET_B_HISADDR);
    flash_stream_read(&flash, hisaddr+4096-1, 1, data);
    if (data[0] != 0x5A)
    {
        flash_stream_read(&flash, hisaddr+2*4096-1, 1, data);
        if (data[0] == 0x5A)
        {
            hisaddr += 4096;
            addstate = 1;
        }
        else
        {
            return -1;
        }
    }
    
    if (datanum<5)
    {
        if (addstate)
        {
            hisaddr -= 4096;
        }
        else
        {
            hisaddr += 4096;
        }
    }
    offset = (datanum - 1)%4*1024;
    hisaddr += offset;
    memset(hisdatabuf, 0, HISDATALEN);
    flash_stream_read(&flash, hisaddr, HISDATALEN, hisdatabuf);
    print_data(hisdatabuf,HISDATALEN);
    for (int i=3;i<HISDATALEN;i++)
    {
        if (hisdatabuf[i] != 0xFF) hisdatabuf[i] &= 0x7F;
    }
    print_data(hisdatabuf,HISDATALEN);
    memset(Hisdata, 0, HISDATABASE64LEN);
    outlen2 = HISDATABASE64LEN;
    base64_encode(Hisdata, &outlen2, hisdatabuf, outlen);
    //printf("read Hisdata len : %d : %s\n", outlen2, Hisdata);
    return 0;
}
#endif
#if 0
int read_hisdata(cJSON *serviceJSObject, char *characteristic_type)
{
    int datanum, outlen2;
    int outlen = HISDATALEN;
    flash_t flash;
    int hisaddr,offset;
    uint8 hisdatabuf[HISDATALEN];
    struct tm *timeinfo;
    int tm_year,tm_mon;

    read_locoltime(timeinfo);
    tm_year = timeinfo->tm_year;
    tm_mon = timeinfo->tm_mon;
    
    datanum = checkdatanum(characteristic_type);
    if ((datanum != 0) && (tm_year > 2000))
    {
        hisaddr = (serviceJSObject == MyOutletService1) ? (OUTLET_A_HISADDR) : (OUTLET_B_HISADDR);
        offset = (tm_mon-1)*1024;
        offset -=(8-datanum)*1024;
        hisaddr += offset;
        if (offset < 0)
        {  
            hisaddr += 4096*3;
        }
        memset(hisdatabuf, 0, HISDATALEN);
        flash_stream_read(&flash, hisaddr, HISDATALEN, hisdatabuf);
        print_data(hisdatabuf,HISDATALEN);
        memset(Hisdata, 0, HISDATABASE64LEN);
        outlen2 = HISDATABASE64LEN;
        base64_encode(Hisdata, &outlen2, hisdatabuf, outlen);
        //printf("read Hisdata len : %d : %s\n", outlen2, Hisdata);
        return 0;
    }
    return -1;
}
#endif

int read_monthdata()
{
    int datanum, outlen2;
    int outlen = 72;
    flash_t flash;
    int hisaddr;
    char hisdatabuf[72];
    
    hisaddr = OUTLET_MON_HISADDR;
    memset(hisdatabuf, 0, outlen);
    flash_stream_read(&flash, hisaddr, outlen, hisdatabuf);
    memset(Hisdata, 0, HISDATABASE64LEN);
    outlen2 = HISDATABASE64LEN;
    base64_encode(Hisdata, &outlen2, hisdatabuf, outlen);
    printf("read monthdata len : %d : %s\n", outlen2, Hisdata);
    return 0;
}

int timerdatalen(const unsigned char *timerdata)
{
    int len = 0;
    int i;
    for (i=0; i<300; i++)
    {
        //printf("%x", timerdata[i]);
        if (timerdata[i] == 0xff) 
        {
            len = i;
            break;
        }
    }
    if (i == 300) return 300;
    return len;
}

int read_timerdata(outlet_state *outletstate)
{
    int datanum, outlen2, timerlen, i;
    int outlen = OUTLET_TIMER_TWOLEN;
    flash_t flash;
    int hisaddr;
    char hisdatabuf[OUTLET_TIMER_TWOLEN];
    
    hisaddr = OUTLET_TIMER_ADDR;
    memset(hisdatabuf, 0, outlen);
    flash_stream_read(&flash, hisaddr, outlen, hisdatabuf);

//    printf("\r\n");
//    for (i=0; i<600; i++)
//    {
//        printf("%x",hisdatabuf[i]);
//    }
//    printf("\r\n");
    
    memset(Hisdata, 0, HISDATABASE64LEN);
    outlen2 = HISDATABASE64LEN;
    if (outletstate == (&outletstateA))
    {
        timerlen = timerdatalen(hisdatabuf);
        printf("timerlen : %d \n", timerlen);
        base64_encode(Hisdata, &outlen2, hisdatabuf, timerlen);
    }
    else
    {
        timerlen = timerdatalen(hisdatabuf+OUTLET_TIMER_ONELEN);
        printf("timerlen : %d \n", timerlen);
        base64_encode(Hisdata, &outlen2, hisdatabuf+OUTLET_TIMER_ONELEN, timerlen);
    }
    printf("read timerdata len : %d : %s\n", outlen2, Hisdata);
    return 0;
}

cJSON *read_Outlet_handler(cJSON *serviceJSObject, const char *cha_type, cJSON *charJSObj)
{//print_data(cha_type, 10);
    //char *cha_type = cJSON_GetObjectItem(charJSObj, kChaObj_Type)->valuestring;
    //printf("read : %s\n",cha_type);
    outlet_state *outletstate;
    if (serviceJSObject == MyOutletService1
    #if FIRST_VERSION
    ||serviceJSObject == MyMeterService1
    #endif
    ){
        outletstate = &outletstateA;
        //printf("read 1\n");
    }
    else if (serviceJSObject == MyOutletService2
    #if FIRST_VERSION
    ||serviceJSObject == MyMeterService2
    #endif
    ){
        outletstate = &outletstateB;
        //printf("read 2\n");
    }
    else{
        return NULL;
    }
    
    if (0 == read_hisdata(serviceJSObject, cha_type))
    {
        return cJSON_CreateString(Hisdata);
    }
    else if (strcmp(cha_type, kCharacteristicUUID_On) == 0)
    {
        //printf("read on\n");
        return cJSON_CreateBool(outletstate->on);
    }
    else if (strcmp(cha_type, kCharacteristicUUID_OutletInUse) == 0)
    {
        //printf("read in use :%d\n",outletstate->inuse);
        return cJSON_CreateBool(outletstate->inuse);
    }
    else if (strcmp(cha_type, kChaUUID_Month_Data) == 0)
    {
        read_monthdata();
        return cJSON_CreateString(Hisdata);
    }
    else if (strcmp(cha_type, kChaUUID_Timing_On) == 0)
    {
        read_timerdata(outletstate);
        return cJSON_CreateString(Hisdata);
    }
    else if (strcmp(cha_type, kChaUUID_Met_Total_Quantity) == 0)
    {
        return cJSON_CreateNumber(outletstate->metqua);
    }
    else if (strcmp(cha_type, kChaUUID_Met_Voltage) == 0)
    {
        return cJSON_CreateNumber(outletstate->metvol);
    }
    else if (strcmp(cha_type, kChaUUID_Met_Current) == 0)
    {
        return cJSON_CreateNumber(outletstate->metcur);
    }
    else if (strcmp(cha_type, kChaUUID_Met_Power) == 0)
    {
        return cJSON_CreateNumber(outletstate->metpow);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_overVoltage) == 0)
    {
        return cJSON_CreateNumber(outletstate->thrvol_over);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_underVoltage) == 0)
    {
        return cJSON_CreateNumber(outletstate->thrvol_under);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Current) == 0)
    {
        return cJSON_CreateNumber(outletstate->thrcur);
    }
    else if (strcmp(cha_type, kChaUUID_Thr_Power) == 0)
    {
        return cJSON_CreateNumber(outletstate->thrpow);
    }
    #if FIRST_VERSION
    else if (strcmp(cha_type, kChaUUID_FV_Met_Voltage) == 0)
    {
        //printf("read kChaUUID_FV_Met_Voltage\n");
        printf("read vol\n");
        printf("%4.2f\n",outletstate->metvol);
        return cJSON_CreateNumber(outletstate->metvol);
    }
    else if (strcmp(cha_type, kChaUUID_FV_Met_Current) == 0)
    {
        printf("read cur\n");
        return cJSON_CreateNumber(outletstate->metcur);
    }
    else if (strcmp(cha_type, kChaUUID_FV_Met_Power) == 0)
    {
        printf("read pow\n");
        return cJSON_CreateNumber(outletstate->metpow);
    }
    #endif
    
    return NULL;
}

cJSON *read_Ota_handler(const char *cha_type)
{
    if (strcmp(cha_type, kChaUUID_WF_Ota_Prog) == 0)
    {
        return cJSON_CreateNumber(0);
    }
    else if (strcmp(cha_type, kChaUUID_MCU_Ota_Prog) == 0)
    {
        return cJSON_CreateNumber(0);
    }
    else if (strcmp(cha_type, kChaUUID_WF_Ota_Res) == 0)
    {
        return cJSON_CreateBool(0);
    }
    else if (strcmp(cha_type, kChaUUID_MCU_Ota_Res) == 0)
    {
        return cJSON_CreateBool(0);
    }
    return NULL;
}

cJSON *read_System_handler(const char *cha_type)
{
    if (strcmp(cha_type, kChaUUID_Status) == 0)
    {
        return cJSON_CreateBool(1);
    }
    else if (strcmp(cha_type, kChaUUID_Time) == 0)
    {
     struct tm timeinfo;
     char buffer[] = "1970-01-01 01:01";
     if (!rtc_isenabled()) return cJSON_CreateString("");
     read_locoltime(&timeinfo);
     sprintf(buffer, "%04d-%02d-%02d %02d:%02d", timeinfo.tm_year,timeinfo.tm_mon,timeinfo.tm_mday,
     timeinfo.tm_hour,timeinfo.tm_min);
     printf("read : %s",buffer);
        return cJSON_CreateString(buffer);
    }
    return NULL;
}

int ES_read_handler(int aid, int iid, cJSON **outValue)
{
    static int number = 0;
    cJSON *accesJSObj, *servJSObj, *charJSObj;
    
    accesJSObj = HAPSearchDatabaseAID(MyDB, aid);
    charJSObj = HAPSearchDatabaseIID(MyDB, accesJSObj, iid);
    servJSObj = HAPSearchCharacteristicService(MyDB, charJSObj);
    if ((accesJSObj == MyAccessory) &&
       (servJSObj == MyOutletService1 ||
        servJSObj == MyOutletService2 ||
        servJSObj == MyOtaService||
        servJSObj == MySysService
        #if FIRST_VERSION
        ||servJSObj == MyMeterService1 ||servJSObj == MyMeterService2
        #endif
        ))
    {
        char *cha_type = cJSON_GetObjectItem(charJSObj, kChaObj_Type)->valuestring;
        if (servJSObj == MyOutletService1 || servJSObj == MyOutletService2
        #if FIRST_VERSION
        || servJSObj == MyMeterService1 ||servJSObj == MyMeterService2
        #endif
        )
        {
            *outValue = read_Outlet_handler(servJSObj, cha_type, charJSObj);
        }
        else if (servJSObj == MyOtaService )
        {
            *outValue = read_Ota_handler(cha_type);
        }
        else if (servJSObj == MySysService )
        {
            *outValue = read_System_handler(cha_type);
        }
    }
    return kHAPStatus_Success;
}
cJSON *cJSON_CreateAlarmArray()
{
    PhytrexClockAlarm CA[ALARM_COUNT] = {0};
    phytrex_FlashClockRead(&CA, sizeof(CA));
      
    cJSON *object = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(object, "alarm", array);
    for(int i=0; i<10; i++) {
        cJSON *alarm = cJSON_CreateObject();
        cJSON_AddItemToObject(alarm, "id", cJSON_CreateNumber(i));
        //cJSON_AddItemToObject(alarm, "id", cJSON_CreateNumber(i));
        cJSON_AddItemToArray(array, alarm);
    }
    
    return object;
}

void Parse_Alarm_JSON(PhytrexClockAlarm *pArray, int len, cJSON *AlarmGroup)
{
    PhytrexClockAlarm ca = {0};
  
    cJSON *array = cJSON_GetObjectItem(AlarmGroup, "alarm");
    int size = cJSON_GetArraySize(array);
    for(int i=0; i<size; i++) {
        cJSON *object = cJSON_GetArrayItem(array, i);
        if(object != NULL) {
            char *str = cJSON_Print(object);
            printf("%s\n\r", str);
            free(str);
        }
        /*memset(&ca, 0, sizeof(ca));
        ca.t;
        memcpy(&pCA[i], &ca, sizeof(ca));*/
    }
}

void phytrex_alarm_action(PhytrexClockAlarm CA)
{
#if CONFIG_ALARM
	/*if(pCA->action & ACTION_MOTOR_POWER_MASK) {
		int time = ((pCA->action & ACTION_MOTOR_POWER_MASK)>>ACTION_MOTOR_POWER_SHIFT);
		if(time == 0)
			send_cmd(CMD_KEY, CMD_MOTOR_OFF, NULL);
		else if(time == 2 || time == 8 || time == 12) {
			if(time == 2)
				send_cmd(CMD_KEY, CMD_MOTOR_2H, NULL);
			else if(time == 8)
				send_cmd(CMD_KEY, CMD_MOTOR_8H, NULL);
			else if(time == 12)
				send_cmd(CMD_KEY, CMD_MOTOR_12H, NULL);
		}
		else
			send_cmd(CMD_CUSTOM, time, NULL);
	}*/
#endif
}

cJSON *phytrex_AccessoryDatabaseSetup(HAPParameter_t *param)
{
    HAPInformationService_t info;
    HAPOutletService_t outlet;

    // Generate an accessory with information
    MyDB = HAPCreateAccessoryDatabase();
    memset(&info, 0, sizeof(HAPInformationService_t));
    info.manufacturer = hk_flow.Wac.manufacturer;
    info.model = hk_flow.Wac.model;
    info.name = param->name;
    phytrex_FlashDataRead(&ex_param, sizeof(ex_param));
    info.serial_number = (char*)ex_param.serial_number;
    info.firmware_revision = (char*)cur_ver.firmware;
    info.hardware_revision = (char*)ex_param.hardware_revision;
    info.software_revision = (char*)cur_ver.software;
    MyAccessory = HAPCreateDatabaseAccessory(MyDB, &info);
    
    outlet.on = ON_STATE;
    outlet.outlet_in_use = OUTLET_IN_USE_STATE;
    
    outlet.name = OUTLET_1;
    MyOutletService1 = ES_HAPCreateOutletService(&outlet);
    HAPAddAccessoryService(MyAccessory, MyOutletService1);
#if FIRST_VERSION
    outlet.name = OUTLET_1 "-meter";
    MyMeterService1 = ES_HAPCreateMeterService(&outlet);
    HAPAddAccessoryService(MyAccessory, MyMeterService1);
#endif

    outlet.name = OUTLET_2;
    MyOutletService2 = ES_HAPCreateOutletService(&outlet);
    HAPAddAccessoryService(MyAccessory, MyOutletService2);
#if FIRST_VERSION
    outlet.name = OUTLET_2 "-meter";
    MyMeterService2 = ES_HAPCreateMeterService(&outlet);
    HAPAddAccessoryService(MyAccessory, MyMeterService2);
#endif
    
    HAPCreatePhytrexServiceGroup(MyAccessory);
    
    HAPSetAccessoryCategory(kAccessoryCategory_Outlet);
    
    HAPSetCharacteristicReader(ES_read_handler);
    
    return MyDB;
}

void getMCU_WACNotifyReady()
{
    printf("\n=====phytrex_WACPlatformNotify :  WACNotifyReady====\n");
    setMCU_WAC_on();
} 
void getMCU_WACNotifyComplete()
{
    printf("\n=====phytrex_WACPlatformNotify :  WACNotifyComplete====\n");
    setMCU_WAC_off();
} 

void phytrex_homekit_cb(PhytrexHomekitNotify_t notify)
{
    switch(notify) {
    case PhytrexHomekitNotifyWacStart:
    printf("\n=====phytrex_homekit_cb :  PhytrexHomekitNotifyWacStart====\n");
    getMCU_WACNotifyReady();
        break;
    case PhytrexHomekitNotifyWacAlready:
    printf("\n=====phytrex_homekit_cb :  PhytrexHomekitNotifyWacAlready====\n");
    getMCU_WACNotifyComplete();
        break;
    case PhytrexHomekitNotifyWacAfter:
    printf("\n=====phytrex_homekit_cb :  PhytrexHomekitNotifyWacAfter====\n");
        break;
    case PhytrexHomekitNotifyHapStart:
        HAPstatusNotify();
        break;
    default:
        break;
    }
}

void phytrex_WACPlatformNotify(WACNotify_t notify)
{
	switch(notify) {
		case WACNotifyReady:
		          getMCU_WACNotifyReady();
                        start_web_server();
			break;
		case WACNotifyConfiguring:
		printf("\n=====phytrex_WACPlatformNotify :  WACNotifyConfiguring====\n");
			break;
		case WACNotifyComplete:
		           getMCU_WACNotifyComplete();
                        stop_web_server();
                        phytrex_homekit_hap(&hk_flow.Hap);
			break;
		case WACNotifyError:
		printf("\n=====phytrex_WACPlatformNotify :  WACNotifyError====\n");
                        LoadWifiConfig(); //alexyi
                        RestartSoftAP(); //alexyi
			break;
		default:
			break;
	}
}

void phytrex_HAPPlatformNotify(HAPNotify_t notify)
{
	switch(notify) {
		case HAPNotifyReady:
			break;
		default:
			break;
	}
}

void init_gpio(void)
{
    int i;
    // JTAG enable pin is disabled
     //   sys_jtag_off();
       // for (i=0; i<(400*1000); i++) asm(" nop"); //delay for JTAG off done
        //printf("jtag off\r\n");
     

  //identify led init
     gpio_init(&identy_led, GPIO_IDENTY);
     gpio_dir(&identy_led, PIN_OUTPUT);
    gpio_mode(&identy_led, PullNone);
        //outlet1-in-use init
        gpio_init(&OlInUse1, GPIO_OLINUSE1);
        gpio_dir(&OlInUse1, PIN_INPUT);
        gpio_mode(&OlInUse1, PullUp);
        //outlet2-in-use init
        gpio_init(&OlInUse2, GPIO_OLINUSE2);
        gpio_dir(&OlInUse2, PIN_INPUT);
        gpio_mode(&OlInUse2, PullUp);
  //      gpio_write(&identy_led, 1); //turn on identify led at first

        //Initial LED timer
        printf("====init ledTmr====\n");
        gtimer_init(&ledTmr, LED_TIMER_ID);
        //Initial button timer
     //   gtimer_init(&btnTmr, BTN_TIMER_ID);
}
void read_locoltime(struct tm *ptm)
{
    struct tm *timeinfo;
    time_t t;

    t = rtc_read();
    t += 8*60*60;
    timeinfo = localtime(&t);

    ptm->tm_year = timeinfo->tm_year+1900;
    ptm->tm_mon = timeinfo->tm_mon+1;
    ptm->tm_mday = timeinfo->tm_mday;
    ptm->tm_hour = timeinfo->tm_hour;
    ptm->tm_min = timeinfo->tm_min;
    ptm->tm_sec = timeinfo->tm_sec;
    ptm->tm_wday = timeinfo->tm_wday;
    ptm->tm_yday = timeinfo->tm_yday;
}

void write_locoltime(struct tm *ptm)
{
    time_t t;

    t = mktime(ptm);
    t -= 8*60*60;
    rtc_write(t);
}

void set_locoltime(char *time)
{
    int year,mon,day,hour,min;
    struct tm *timeinfo;
    
    if (!rtc_isenabled()) return;
    
    time[4] = 0;
    time[7] = 0;
    time[10] = 0;
    time[13] = 0;
    year = atoi(&time[0]);
    mon = atoi(&time[5]);
    day = atoi(&time[8]);
    hour = atoi(&time[11]);
    min = atoi(&time[14]);
    printf("set time:%d %d %d %d %d\n",year,mon,day,hour,min);
    timeinfo->tm_year = year-1900;
    timeinfo->tm_mon = mon-1;
    timeinfo->tm_mday = day;
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = min;
    timeinfo->tm_sec = 0;
    timeinfo->tm_isdst = 0;
    write_locoltime(timeinfo);
    //difftime
}
int chech_sector_writable(uint32 begin, uint32 end)
{
    int i;
    flash_t flash;
    uint8 data;
    
    for (i=begin;i<end;i++)
    {
        flash_stream_read(&flash, i, 1, &data);
        if (data != 0xFF) return 0;
    }
    return 1;
}

#if HISDATA_15_MIN
void wirte_hisdata2flash(hisdata_state report_hisdata)
{
    uint32 his_sector,offset;
    //uint32 mon_addr,lastmon_addr,data_addr,hour_addr;
    flash_t flash;
    uint8 channel, year,mon,day,hour,min,quantity;
    uint8 data[3];
    uint8 sector_mon;
    int addstate = 0;
    
    channel = report_hisdata.channel;
    year = report_hisdata.year;
    mon = report_hisdata.mon;
    day = report_hisdata.day;
    hour = report_hisdata.hour;
    min = report_hisdata.min;
    quantity = report_hisdata.quantity;
    if ((mon>12)||(mon<1)||(day<1)||(day>31)||(hour>23)||(min>59)) return;
    
    his_sector = (channel == 1) ? (OUTLET_A_HISADDR) : (OUTLET_B_HISADDR);
    
    flash_stream_read(&flash, his_sector+4096-1, 1, data);
    if (data[0] != 0x5A)
    {
        flash_stream_read(&flash, his_sector+2*4096-1, 1, data);
        if (data[0] == 0x5A)
        {
            his_sector += 4096;
            addstate = 1;
        }
        else
        {
            goto exit_erase_flash;
        }
    }
    
    flash_stream_read(&flash, his_sector, 2, data);
    printf("year,mon:%d,%d\n",data[0],data[1]);
    if ((data[0] == year) && (data[1] == mon))
    {
        goto exit_write_data;
    }

    data[0] = 0x00;
    flash_stream_write(&flash, his_sector+4096-1, 1, data);
    
    if (addstate)
    {
        his_sector -= 4096;
    }
    else
    {
        his_sector += 4096;
    }
    
exit_erase_flash:
    printf("[%s] exit_erase_flash\n", __FUNCTION__);
    flash_erase_sector(&flash, his_sector);
    data[0] = 0x5A;
    flash_stream_write(&flash, his_sector+4096-1, 1, data);

    
exit_write_mon:
    printf("[%s] exit_write_mon : %d   %d\n", __FUNCTION__,year,mon);
    data[0] = year;
    data[1] = mon;
    data[2] = 1;
    flash_stream_write(&flash, his_sector, 3, data);
    data[2] = 9;
    flash_stream_write(&flash, his_sector+1024, 3, data);
    data[2] = 17;
    flash_stream_write(&flash, his_sector+2*1024, 3, data);
    data[2] = 25;
    flash_stream_write(&flash, his_sector+3*1024, 3, data);
    
exit_write_data:
    printf("[%s] exit_write_data : %d\n", __FUNCTION__,quantity);
    offset = day/8*1024+3+(day-1)%8*96+hour*4+min/15;
    flash_stream_read(&flash, his_sector+offset, 1, data);
    if (data[0] == 0xFF)
    {
        data[0] = quantity | 0x80;    //bit7 means haven't reported to Iot
        flash_stream_write(&flash, his_sector+offset, 1, data);
    }
    
    uint8 checkdata[1024];
    for(int i=0;i<2;i++)
    {
    flash_stream_read(&flash, OUTLET_A_HISADDR+i*4096+1024*3, 1024, checkdata);
    printf("%d : ",i+1);
    printf("%d ",checkdata[0]);
    printf("%d ",checkdata[1]);
    printf("%d \n",checkdata[2]);
    print_data(checkdata+3, 1024-3);
    }
    
}
#endif
#if 0
void wirte_hisdata2flash(hisdata_state report_hisdata)
{
    uint32 his_sector;
    uint32 mon_addr,lastmon_addr,data_addr,hour_addr;
    flash_t flash;
    uint8 channel, year,mon,day,hour,quantity;
    uint8 data[2];
    uint8 sector_mon;
    
    channel = report_hisdata.channel;
    year = report_hisdata.year;
    mon = report_hisdata.mon;
    day = report_hisdata.day;
    hour = report_hisdata.hour;
    quantity = report_hisdata.quantity;
    if ((mon>12)||(mon<1)||(day<1)||(day>31)||(hour>23)) return;
    
    his_sector = (channel == 1) ? (OUTLET_A_HISADDR) : (OUTLET_B_HISADDR);
    his_sector += (mon-1)/4*4096;
    mon_addr = his_sector+((mon-1)%4)*1024;
    data_addr = mon_addr+2;
    hour_addr = data_addr+(day-1)*24+hour;
    
    flash_stream_read(&flash, mon_addr, 2, data);
    printf("year,mon:%d,%d\n",data[0],data[1]);
    if ((data[0] == year) && (data[1] == mon))
    {
        goto exit_write_data;
    }

    if ((data[0] == 0xFF) && (data[1] == 0xFF))
    {
        if(chech_sector_writable(mon_addr,his_sector+4096))
        {
            goto exit_write_mon;
        }
    }
    
exit_erase_flash:
    printf("[%s] exit_erase_flash\n", __FUNCTION__);
    flash_erase_sector(&flash, his_sector);

exit_write_mon:
    printf("[%s] exit_write_mon : %d   %d\n", __FUNCTION__,year,mon);
    data[0] = year;
    data[1] = mon;
    flash_stream_write(&flash, mon_addr, 2, data);
    
exit_write_data:
    printf("[%s] exit_write_data : %d\n", __FUNCTION__,quantity);
    flash_stream_read(&flash, hour_addr, 1, data);
    if (data[0] == 0xFF)
    {
        data[0] = quantity;
        flash_stream_write(&flash, hour_addr, 1, data);
    }
}
#endif
void init_outletstate(uint8 cc)
{
    //wait_ms(1000);
    getMCU_meterage(cc);
    vTaskDelay(1000);
    //wait_ms(1000);
    getMCU_thrpow(cc);
    vTaskDelay(1000);
    //wait_ms(1000);
    getMCU_thrcur(cc);
    vTaskDelay(1000);
    //wait_ms(1000);
    getMCU_thrvol(cc);
    vTaskDelay(1000);
    //wait_ms(1000);
}

void print_outletstate(outlet_state outletstate)
{
    printf("on_off : %1d  in_use : %1d\n",outletstate.on,outletstate.inuse);
    printf("meter vol : %4.2f  cur : %4.2f  pow : %4.2f  qua : %4.2f\n",
        outletstate.metvol,outletstate.metcur,outletstate.metpow,outletstate.metqua);
    printf("thres vol : %4.2f ~ %4.2f  cur : %4.2f  pow : %4.2f\n",
        outletstate.thrvol_under,outletstate.thrvol_over,outletstate.thrcur,outletstate.thrpow);
}

void getMCU_state()
{
    printf("outlet : A\n");
    print_outletstate(outletstateA);
    printf("outlet : B\n");
    print_outletstate(outletstateB);
}

void getMCU_synch()
{
    //init_outletstate(0x01);
    //init_outletstate(0x02);
    getMCU_onoff_meter();
}

void MCU_meterage_synch()
{
    //getMCU_onoff(0x01);
    //getMCU_onoff(0x02);
    //getMCU_meterage(0x01);
    //getMCU_meterage(0x02);
    getMCU_onoff_meter();
}
int init_time()
{
    struct tm timeinfo;
    if (rtc_isenabled())
    {
        read_locoltime(&timeinfo);
        if (timeinfo.tm_year < 2000) 
        {
            get_ntp_time();
        }

        read_locoltime(&timeinfo);
        if (timeinfo.tm_year > 2000)
        {
            app_handle(SET_DATE, NULL);
            return 1;
        }
    }
    return 0;
}

void alarm_handle()
{
    static uint8 alarmA,alarmB;
    int ret = -1;

    if ((alarmA != outletstateA.alarm))
    {
        alarmA = outletstateA.alarm;
        if (alarmA != 0)
        {
            printf("\n======MCU ALARM =======!!!\n");
            if (alarmA & BIT2){
                Jiguang_Push(2);
            }else if ((alarmA & BIT3)||(alarmA & BIT4)){
                Jiguang_Push(3);
            }else if (alarmA & BIT0){
                Jiguang_Push(0);
            }else if (alarmA & BIT1){
                Jiguang_Push(1);
            }    
            ret = report_alarm_to_iot(&outletstateA);
            if (ret != 1) alarmA = 0;
        }
    }

    if ((alarmB != outletstateB.alarm))
    {
        alarmB = outletstateB.alarm;
        if (alarmB != 0)
        {
            printf("\n======MCU ALARM =======!!!\n");
            if (alarmB & BIT2){
                Jiguang_Push(2);
            }else if ((alarmB & BIT3)||(alarmB & BIT4)){
                Jiguang_Push(3);
            }else if (alarmB & BIT0){
                Jiguang_Push(0);
            }else if (alarmB & BIT1){
                Jiguang_Push(1);
            }
            report_alarm_to_iot(&outletstateB);
        }
    }
}

void set_outlet_timer(struct outlet_timer *timer,uint8_t *data)
{
    timer->repeat = data[0]>>7;
    timer->onoff = (data[0]&0x7f)>>6;
    timer->hour = data[0]&0x1f;
    timer->min = data[1]&0x3f;
    timer->enable = data[2]>>7;
    timer->weekday = data[2]&0x7f;
}

void outlet_timer_handle(uint8_t * timerdata,struct tm timeinfo,uint8_t cc)
{
    struct outlet_timer timer;
    uint8_t* data;    
    time_t t1,t2;
    uint8_t repeat;
    t2 = rtc_read();

    for (int i=0;i<OUTLET_TIMER_ONELEN;i+=7)
    {
        data = timerdata+i;
        if (data[0] == 0xff) return;
        set_outlet_timer(&timer,data);
        memcpy(&t1, &data[3], 4);
        //printf("t1 : %d ; t2 : %d ; t2-t1 : %d\n",t1,t2,t2-t1);
         if ((t2-t1)>7*24*60*60)
        {
            repeat = 0;
        }
        else
        {
            repeat = 1;
        }
        
        //printf("%02x  %02x  %02x\n",data[0],data[1],data[2]);
        //printf("repeat:%d onoff:%d  hour:%d  min:%d  enable:%d  week:%02x\n",
            //timer.repeat,timer.onoff,timer.hour,timer.min,timer.enable,timer.weekday);
        //printf("weekday : %d , \n0x01<<(7-weekday) : 0x%02x  \n&  setweek : %02x\n=0x%02x\n",
            //timeinfo.tm_wday,0x01<<(7-timeinfo.tm_wday),timer.weekday,(0x01<<(7-timeinfo.tm_wday))&timer.weekday);
        
        //if (timer.enable == 0) continue;
        if ((timer.hour != timeinfo.tm_hour)||(timer.min != timeinfo.tm_min)) continue;
        
        
        if ((0x01<<(7-timeinfo.tm_wday))&timer.weekday)
        {
            if(timer.repeat|repeat)
            {
                setOutlet_onoff(cc, timer.onoff);
            }
        }
    }
}

void timer_handle()
{
    int outlen = OUTLET_TIMER_TWOLEN;
    flash_t flash;
    int addr;
    uint8_t timerdatabuf[OUTLET_TIMER_TWOLEN];
    static int minute = -1;

    struct tm timeinfo;
    if (!rtc_isenabled())  return;
    read_locoltime(&timeinfo);
    if (timeinfo.tm_year < 2000)  return;
    if (minute != timeinfo.tm_min)
    {
        minute = timeinfo.tm_min;
        printf("minute : %d\n",minute);
        addr = OUTLET_TIMER_ADDR;
        memset(timerdatabuf, 0, outlen);
        flash_stream_read(&flash, addr, outlen, timerdatabuf);
        outlet_timer_handle(&timerdatabuf[0],timeinfo,0x01);
        outlet_timer_handle(&timerdatabuf[OUTLET_TIMER_ONELEN],timeinfo,0x02);
    }

}

void hisdata2iot_handle()
{
    static int minute = -1;
    
    if (!rtc_isenabled())  return;
    struct tm timeinfo;
    read_locoltime(&timeinfo);
    if (timeinfo.tm_year < 2000)  return;
    //if ((timeinfo.tm_min)%15 != 2) return;
    if (minute == timeinfo.tm_min) return;
    minute = timeinfo.tm_min;
    printf("hisdata2iot_handle\n");
    int outlen = HISDATALEN;
    flash_t flash;
    int hisaddr1,hisaddr2,offset,ret;

    uint8_t data[3];
    uint8_t data1[3],data2[3];
    uint8 year,mon,day,hour,min,quantity;

    year = timeinfo.tm_year;
    mon = timeinfo.tm_mon;
    day = timeinfo.tm_mday;
    hour = timeinfo.tm_hour;
    min = timeinfo.tm_min;

    hisaddr1 = OUTLET_A_HISADDR;
    flash_stream_read(&flash, hisaddr1+4096-1, 1, data);
    printf("data:0x%x\n",data[0]);
    if (data[0] != 0x5A)
    {
        flash_stream_read(&flash, hisaddr1+2*4096-1, 1, data);
        printf("data:0x%x\n",data[0]);
        if (data[0] == 0x5A)
        {
            hisaddr1 += 4096;
            //addstate = 1;
        }
        else
        {
            return;
        }
    }
    offset = day/8*1024+3+(day-1)%8*96+hour*4+min/15;
    flash_stream_read(&flash, hisaddr1+offset, 1, data1);

    hisaddr2 = OUTLET_B_HISADDR;
    flash_stream_read(&flash, hisaddr2+4096-1, 1, data);
    printf("data:0x%x\n",data[0]);
    if (data[0] != 0x5A)
    {
        flash_stream_read(&flash, hisaddr2+2*4096-1, 1, data);
        printf("data:0x%x\n",data[0]);
        if (data[0] == 0x5A)
        {
            hisaddr2 += 4096;
            //addstate = 1;
        }
        else
        {
            return;
        }
    }
    offset = day/8*1024+3+(day-1)%8*96+hour*4+min/15;    
    flash_stream_read(&flash, hisaddr2+offset, 1, data2);

    printf("data1:0x%x  data2:0x%x\n",data1[0],data2[0]);
    
    if ((data1[0]&0x80)&&(data1[0] != 0xFF)&&(data2[0]&0x80)&&(data2[0] != 0xFF))
    {
        ret = report_hisdata_to_iot(data1[0]&0x7F, data2[0]&0x7F, timeinfo);
        if (ret == 1)
        {
            data1[0] &=0x7F;
            data2[0] &=0x7F;
            flash_stream_write(&flash, hisaddr1+offset, 1, data1);
            flash_stream_write(&flash, hisaddr2+offset, 1, data2);
        }
    }

}

void HAPNotifyHandle(void *param)
{
    xSemaphoreHandle *sema = param;
    static int count_synch = 0;
    static int count_ntp = 0;
    static int count_basic = 0;
    static int count_hap = 0;
    static int ntp_done = 0;

    
    /*outletstateA.on = 1;
    outletstateB.on = 1;
    outletstateA_app.on = outletstateA.on;
    outletstateB_app.on = outletstateB.on;*/

    outletstateA.inuse = gpio_read(&OlInUse1);
    outletstateB.inuse = gpio_read(&OlInUse2);

    vTaskDelay(1000);
    //init_outletstate(0x01);
    //init_outletstate(0x02);
    getMCU_synch();

    outletstateA_app.on = outletstateA.on;
    outletstateB_app.on = outletstateB.on;
    
    while(1)
    {
        xSemaphoreGive(*sema);
        vTaskDelay(500);
        //gpio_write(&identy_led, 0);
        //gpio_write(&identy_led, !gpio_read(&identy_led));
        
        //printf("LED status = %d\r\n", gpio_read(&identy_led));
        if (outletstateA_app.on != outletstateA.on)
        {
            outletstateA_app.on = outletstateA.on;
            HAPoutletNotify(1,outletstateA_app.on);
        }
        if (outletstateB_app.on != outletstateB.on)
        {
            outletstateB_app.on = outletstateB.on;
            HAPoutletNotify(2,outletstateB_app.on);
        }

        if (outletstateA.inuse != gpio_read(&OlInUse1))
        {
            outletstateA.inuse = gpio_read(&OlInUse1);
            printf("OIU1 status = %d\r\n", outletstateA.inuse);
            HAPoiu1Notify(outletstateA.inuse);
        }
        if (outletstateB.inuse != gpio_read(&OlInUse2))
        {
            outletstateB.inuse = gpio_read(&OlInUse2);
            printf("OIU2 status = %d\r\n", outletstateB.inuse);
            HAPoiu2Notify(outletstateB.inuse);
        }   
        
        count_synch++;
        if (count_synch > 30*2*4){
            count_synch = 0;
            MCU_meterage_synch();
        }

        count_ntp++;
        if (count_ntp > 10*4){
            count_ntp = 0;
            if (ntp_done == 0){
                if (init_time()){
                    ntp_done = 1;
                }
            }
        }

        count_basic++;
         if (count_basic > 4){
            count_basic = 0;

            alarm_handle();

            timer_handle();

            hisdata2iot_handle();
        }
        
    }
Exit:
    vTaskDelete(NULL);
}

void led_timer_handler(uint32_t id)
{
    gpio_write(&identy_led, !gpio_read(&identy_led));
    //printf("LED status = %d\r\n", gpio_read(&identy_led));
    if(led_timecnt == -1) //blink forever
    {
    }
    else
    {
        led_timecnt--; //count down
        if(led_timecnt == 0) //stop led timer
        {
            gtimer_stop(&ledTmr);
            gpio_write(&identy_led, 0);
        }
    }
}

void led_blink(uint32_t value, int timeout) //value->unit:micro-second; timeout->unit:second
{
    if((value ==0)||(timeout ==0))
    {
        printf("vlaue can not be zero\r\n");
        return;
    }
    //enable led timer
    gtimer_start_periodical(&ledTmr, value, (void*)led_timer_handler, (uint32_t)&ledTmr);
    if(timeout > 0)
        led_timecnt = timeout*(1000000/value); //convert to timeout counter
    else
        led_timecnt = -1; //no timeout
}

#endif
