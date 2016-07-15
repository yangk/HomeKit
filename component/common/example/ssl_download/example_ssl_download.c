#include <FreeRTOS.h>
#include <task.h>
#include <platform/platform_stdlib.h>

#include <lwip/sockets.h>
#include <polarssl/config.h>
#include <polarssl/memory.h>
#include <polarssl/ssl.h>
#include <cJSON.h>
#include "rtc_api.h"
#include <time.h>
#include "sockets.h"
#include "Netdb.h"
#include "Mqtt_msg.h"
#include "other/outlet/outlet.h"
#define SERVER_HOST    "192.168.13.27"
#define SERVER_PORT    443
#define RESOURCE       "/dummy100k.bin"

static unsigned int arc4random(void)
{
	unsigned int res = xTaskGetTickCount();
	static unsigned int seed = 0xDEADB00B;

	seed = ((seed & 0x007F00FF) << 7) ^
		((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
		(res << 13) ^ (res >> 9);    // using the clock too!

	return seed;
}

static void get_random_bytes(void *buf, size_t len)
{
	unsigned int ranbuf;
	unsigned int *lp;
	int i, count;
	count = len / sizeof(unsigned int);
	lp = (unsigned int *) buf;

	for(i = 0; i < count; i ++) {
		lp[i] = arc4random();  
		len -= sizeof(unsigned int);
	}

	if(len > 0) {
		ranbuf = arc4random();
		memcpy(&lp[i], &ranbuf, len);
	}
}

static int my_random(void *p_rng, unsigned char *output, size_t output_len)
{
	get_random_bytes(output, output_len);
	return 0;
}

static void example_ssl_download_thread(void *param)
{
	int server_fd = -1, ret;
	struct sockaddr_in server_addr;
	ssl_context ssl;

	// Delay to wait for IP by DHCP
	vTaskDelay(10000);
	printf("\nExample: SSL download\n");

	memory_set_own(pvPortMalloc, vPortFree);
	memset(&ssl, 0, sizeof(ssl_context));

	if((ret = net_connect(&server_fd, SERVER_HOST, SERVER_PORT)) != 0) {
		printf("ERROR: net_connect ret(%d)\n", ret);
		goto exit;
	}

	if((ret = ssl_init(&ssl)) != 0) {
		printf("ERRPR: ssl_init ret(%d)\n", ret);
		goto exit;
	}

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, my_random, NULL);
	ssl_set_bio(&ssl, net_recv, &server_fd, net_send, &server_fd);

	if((ret = ssl_handshake(&ssl)) != 0) {
		printf("ERROR: ssl_handshake ret(-0x%x)", -ret);
		goto exit;
	}
	else {
		unsigned char buf[2048];
		int read_size = 0, resource_size = 0, content_len = 0, header_removed = 0;

		printf("SSL ciphersuite %s\n", ssl_get_ciphersuite(&ssl));
		sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", RESOURCE, SERVER_HOST);
		ssl_write(&ssl, buf, strlen(buf));

		while((read_size = ssl_read(&ssl, buf, sizeof(buf))) > 0) {
			if(header_removed == 0) {
				char *header = strstr(buf, "\r\n\r\n");

				if(header) {
					char *body, *content_len_pos;

					body = header + strlen("\r\n\r\n");
					*(body - 2) = 0;
					header_removed = 1;
					printf("\nHTTP Header: %s\n", buf);
					read_size = read_size - ((unsigned char *) body - buf);

					content_len_pos = strstr(buf, "Content-Length: ");
					if(content_len_pos) {
						content_len_pos += strlen("Content-Length: ");
						*(strstr(content_len_pos, "\r\n")) = 0;
						content_len = atoi(content_len_pos);
					}
				}
				else {
					printf("ERROR: HTTP header\n");
					goto exit;
				}
			}

			printf("read resource %d bytes\n", read_size);
			resource_size += read_size;
		}

		printf("final read size = %d bytes\n", read_size);
		printf("http content-length = %d bytes, download resource size = %d bytes\n", content_len, resource_size);
	}

exit:
	if(server_fd != -1)
		net_close(server_fd);

	ssl_free(&ssl);
	vTaskDelete(NULL);
}

void example_ssl_download(void)
{
	if(xTaskCreate(example_ssl_download_thread, ((const char*)"example_ssl_download_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
}

/*****************************************************************************************************/
/*
	Function	:
	Description	:Ji Guang Push
	Input		:
	Output		:
	Return		:
	Notes		:
*/
/*****************************************************************************************************/

#define JPUSH_URL    "api.jpush.cn"
#define JPUSH_PORT    443
#define JPUSH_HEAD    "POST /v3/push HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nAuthorization: Basic YWQ3OWJkMjk1Y2RlNDFkZThjNDI3ZGVhOiA2MTAzOWZhMWI1N2JlOGRjZGNmM2I4NDI=\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\n    \"platform\": \"ios\",\n    \"options\":\n    {\n        \"apns_production\": \"true\"\n    },\n    \"audience\":\n    {\n        \"tag\":\n        [\n            \"54322_bodyInfrared\"\n        ]\n    },\n    \"notification\":\n    {\n        \"ios\":\n        {\n            \"alert\": \"网关54322人体红外：检测到有人！\",\n            \"sound\": \"alarm.mp3\",\n            \"badge\": \"+1\",\n            \"extras\":\n            {\n                \"deviceType\": \"FFFF160000000200\",\n                \"aid\": \"1711292953\",\n                \"uuid\": \"\",\n                \"gid\": \"54322\"\n            }\n        }\n    }\n}\n\0"
#define JPUSH_BODY    "{\n    \"platform\": \"ios\",\n    \"options\":\n    {\n        \"apns_production\": \"true\"\n    },\n    \"audience\":\n    {\n        \"tag\":\n        [\n            \"54322_bodyInfrared\"\n        ]\n    },\n    \"notification\":\n    {\n        \"ios\":\n        {\n            \"alert\": \"网关54322人体红外：检测到有人！\",\n            \"sound\": \"alarm.mp3\",\n            \"badge\": \"+1\",\n            \"extras\":\n            {\n                \"deviceType\": \"FFFF160000000200\",\n                \"aid\": \"1711292953\",\n                \"uuid\": \"\",\n                \"gid\": \"54322\"\n            }\n        }\n    }\n}\n\0"
#define JPUSH_BODY_A    "{\"platform\":\"ios\",\"audience\":\"all\",\"options\":{\"apns_production\":false},\"notification\":{\"alert\":\"智能插座过压值越线警报\",\"android\":{\"extras\":{\"android-key1\":\"android-value1\"}},\"ios\":{\"sound\":\"sound.caf\",\"badge\":\"+1\",\"extras\":{\"ios-key1\":\"ios-value1\"}}}}"
#define JPUSH_HEAD_A    "POST /v3/push HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nAuthorization: Basic YWQ3OWJkMjk1Y2RlNDFkZThjNDI3ZGVhOjYxMDM5ZmExYjU3YmU4ZGNkY2YzYjg0Mg==\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"platform\":\"ios\",\"audience\":\"all\",\"options\":{\"apns_production\":false},\"notification\":{\"alert\":\"智能插座过压值越线警报\",\"android\":{\"extras\":{\"android-key1\":\"android-value1\"}},\"ios\":{\"sound\":\"sound.caf\",\"badge\":\"+1\",\"extras\":{\"ios-key1\":\"ios-value1\"}}}}"

//#define JPUSH_HEAD_B    "POST /v3/push HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nAuthorization: Basic YWQ3OWJkMjk1Y2RlNDFkZThjNDI3ZGVhOjYxMDM5ZmExYjU3YmU4ZGNkY2YzYjg0Mg==\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"platform\":\"ios\",\"audience\":\"all\",\"options\":{\"apns_production\":false},\"notification\":{\"alert\":\"\\u667a\\u80fd\\u63d2\\u5ea7\\u8fc7\\u538b\\u503c\\u8d8a\\u7ebf\\u8b66\\u62a5\",\"ios\":{\"sound\":\"alarm.mp3\",\"badge\":\"+1\",\"extras\":{\"ios-key1\":\"ios-value1\"}}}}"
#define JPUSH_HEAD_B    "POST /v3/push HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nAuthorization: Basic MGYzYTU4MWI0YmVjN2U1NDlhZDE4NWVlOjFlMmEwNDRkODYwNDE2ZjdlYzIxYmQ5NQ==\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n"
/*#define JPUSH_BODY_B    \
"{\"platform\":\"ios\","    \
"\"audience\":{\"tag\":[\"%s\",\"asdf\"]},"    \
"\"options\":{\"apns_production\":\"false\"},"    \
"\"notification\":{"    \
"\"ios\":{\"alert\":\"\\u667a\\u80fd\\u63d2\\u5ea7\\u8fc7\\u538b\\u503c\\u8d8a\\u7ebf\\u8b66\\u62a5\",\"sound\":\"alarm.mp3\",\"badge\":\"+1\",\"extras\":{\"ios-key1\":\"ios-value1\"}}}}"
*/
#define JPUSH_BODY_B    \
"{"\
"\"platform\":\"ios\","\
"\"options\":"\
"{"\
"\"apns_production\":\"true\""\
"},"\
"\"audience\":"\
"{"\
"\"tag\":"\
"["\
"\"%s\""\
"]"\
"},"\
"\"notification\":"\
"{"\
"\"ios\":"\
"{"\
"\"alert\":\"\\u667a\\u80fd\\u63d2\\u5ea7%s\\u8b66\\u62a5\","\
"\"sound\":\"alarm.mp3\","\
"\"badge\":\"+1\","\
"\"extras\":"\
"{"\
"\"alarmTime\":\"%s\","\
"\"topic\":\"%s\""\
"}"\
"}"\
"}"\
"}"

//\\u667a\\u80fd\\u63d2\\u5ea7  智能插座
//\\u8b66\\u62a5//警报
#define JPBD_OVER_VOL    "\\u8fc7\\u538b"//过压
#define JPBD_UNDER_VOL    "\\u6b20\\u538b"//欠压
#define JPBD_OVER_CUR    "\\u8fc7\\u6d41"//过流
#define JPBD_OVER_POW    "\\u8fc7\\u529f\\u7387"//过功率

extern cJSON *MyOutletService1;

void hex_printf(unsigned char *buf)
{
    for(int i=0;i<strlen(buf);i++)
    {
        printf("%02x ",buf[i]);
    }
    printf("\n");
}
void Jiguang_Push(int mode)
{
       int server_fd = -1, ret;
	struct sockaddr_in server_addr;
	ssl_context ssl;

      wait(2);
	printf("\nJPush start\n");

	memory_set_own(pvPortMalloc, vPortFree);
	memset(&ssl, 0, sizeof(ssl_context));

	//if((ret = net_connect(&server_fd, "192.168.1.149", 9999)) != 0) {
	//if((ret = net_connect(&server_fd, "113.31.17.107", JPUSH_PORT)) != 0) {
	//if((ret = net_connect(&server_fd, "119.90.34.208", JPUSH_PORT)) != 0) {
	if((ret = net_connect(&server_fd, JPUSH_URL, JPUSH_PORT)) != 0) {
		printf("ERROR: net_connect ret(%d)\n", ret);
		goto exit;
	}

	if((ret = ssl_init(&ssl)) != 0) {
		printf("ERRPR: ssl_init ret(%d)\n", ret);
		goto exit;
	}

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, my_random, NULL);
	ssl_set_bio(&ssl, net_recv, &server_fd, net_send, &server_fd);

	if((ret = ssl_handshake(&ssl)) != 0) {
		printf("ERROR: ssl_handshake ret(-0x%x)", -ret);
		goto exit;
	}
	else {
		unsigned char buf[2048];
		uint8_t modebuf[40];
		int read_size = 0, resource_size = 0, content_len = 0, header_removed = 0;
		int jpbd_len;
		printf("SSL ciphersuite %s\n", ssl_get_ciphersuite(&ssl));
		char pushid[32];
	       get_pushid(pushid);

	       memset(buf, 0, sizeof(buf));
	       memset(modebuf, 0, sizeof(modebuf));

            struct tm timeinfo;
            char timebuf[] = "1970-01-01 01:01";
            if (rtc_isenabled())
            {
                read_locoltime(&timeinfo);
                sprintf(timebuf, "%04d-%02d-%02d %02d:%02d", timeinfo.tm_year,timeinfo.tm_mon,
                timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min);
            }

	       switch(mode)
              {
                case 0: 
                    memcpy(modebuf, JPBD_OVER_VOL, sizeof(JPBD_OVER_VOL));
                    break;
                case 1: 
                    memcpy(modebuf, JPBD_UNDER_VOL, sizeof(JPBD_UNDER_VOL)); 
                    break;
                case 2: 
                    memcpy(modebuf, JPBD_OVER_CUR, sizeof(JPBD_OVER_CUR)); 
                    break;
                case 3: 
                    memcpy(modebuf, JPBD_OVER_POW, sizeof(JPBD_OVER_POW)); 
                    break;
                default : 
                    memcpy(modebuf, JPBD_OVER_VOL, sizeof(JPBD_OVER_VOL));
              }
              sprintf(buf, JPUSH_BODY_B, pushid, modebuf, timebuf, pushid);
              jpbd_len = strlen(buf);
                
		sprintf(buf, JPUSH_HEAD_B JPUSH_BODY_B, JPUSH_URL, jpbd_len, pushid, modebuf, timebuf, pushid);
		printf("%s\n", buf);
		ssl_write(&ssl, buf, strlen(buf));

             //memset(buf, 0, sizeof(buf));
             //sprintf(buf, JPUSH_BODY_B, pushid, pushid);
             //printf("%d\n", strlen(buf));
             
             memset(buf, 0, sizeof(buf));
		read_size = ssl_read(&ssl, buf, sizeof(buf));
			printf("HTTPS get============\n");
			printf("%s\n", buf);
	}

exit:
        printf("exit============\n");
	if(server_fd != -1)
		net_close(server_fd);
	ssl_free(&ssl);
}

/*****************************************************************************************************/
/*
	Function	:
	Description	:NTP
	Input		:
	Output		:
	Return		:
	Notes		:
*/
/*****************************************************************************************************/

char* ntp_server[] = {
    "210.72.145.44",
    "133.100.11.8",
    NULL,
};

#define NTP_PORT               123               /*NTP专 用端口号字符串*/
//#define NTP_SERVER_IP       "210.72.145.44" /*国家授时中心 IP*/
#define NTP_SERVER_IP       "133.100.11.8" /*上海交通大学网络中心NTP服务器地址*/
#define NTP_PORT_STR        "123"          /*NTP专用端口号字 符串*/
#define NTP_PCK_LEN 48
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6
 
#define JAN_1970 0x83aa7e80 /* 1900年～1970年之间的时间秒数 */
#define NTPFRAC(x)     (4294 * (x) + ((1981 * (x)) >> 11))

#define USEC(x)         (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))
typedef struct _ntp_time
{

    unsigned int coarse;

    unsigned int fine;
} ntp_time;

struct ntp_packet
{

     unsigned char leap_ver_mode;

     unsigned char startum;

     char poll;

     char precision;
     int root_delay;
     int root_dispersion;

     int reference_identifier;

     ntp_time reference_timestamp;

     ntp_time originage_timestamp;

     ntp_time receive_timestamp;

     ntp_time transmit_timestamp;
};
 
char protocol[32];
/*构建NTP协议包*/
int construct_packet(char *packet)
{

     char version = 1;

     long tmp_wrd;
     int port;
     time_t timer;

     
          memset(packet, 0, NTP_PCK_LEN);
          port = NTP_PORT;
          /*设置16字节的包头*/
          version = 4;

          tmp_wrd = htonl((LI << 30)|(version << 27)

                |(MODE << 24)|(STRATUM << 16)|(POLL << 8)|(PREC & 0xff));

          memcpy(packet, &tmp_wrd, sizeof(tmp_wrd));
         
          /*设置Root Delay、Root Dispersion和Reference Indentifier */

          tmp_wrd = htonl(1<<16);

          memcpy(&packet[4], &tmp_wrd, sizeof(tmp_wrd));

          memcpy(&packet[8], &tmp_wrd, sizeof(tmp_wrd));
          /*设置Timestamp部分*/
          //time(&timer);
          timer = rtc_read();
          /*设置Transmit Timestamp coarse*/

          tmp_wrd = htonl(JAN_1970 + (long)timer);

          memcpy(&packet[40], &tmp_wrd, sizeof(tmp_wrd));
          /*设置Transmit Timestamp fine*/

          tmp_wrd = htonl((long)NTPFRAC(timer));

          memcpy(&packet[44], &tmp_wrd, sizeof(tmp_wrd));

          return NTP_PCK_LEN;
     return 0;
}

int send_to_ntp(int sk, struct addrinfo *addr, struct ntp_packet *ret_time)
{
    fd_set pending_data;
    struct timeval block_time;
    char data[NTP_PCK_LEN * 8];
    int packet_len, data_len = addr->ai_addrlen, count = 0, result, i, re;

    if (!(packet_len = construct_packet(data)))
    {
        return 0;
    }
    /*客户端给服务器端发送NTP协议数据包*/
    if ((result = sendto(sk, data,packet_len, 0, addr->ai_addr, data_len)) < 0)
    {
        printf("sendto");
        return 0;
    }
    /*调用select()函数，并设定超时时间为1s*/
    FD_ZERO(&pending_data);
    FD_SET(sk, &pending_data);
    block_time.tv_sec=2;
    block_time.tv_usec=0;
    if (select(sk + 1, &pending_data, NULL, NULL, &block_time) > 0)
    {
        /*接收服务器端的信息*/
        if ((count = recvfrom(sk, data,NTP_PCK_LEN * 8, 0, addr->ai_addr, &data_len)) < 0)
        {
            printf("recvfrom");
            return 0;
        }
        if (count < NTP_PCK_LEN)
        {
            printf("count");
            return 0;
        }
        /* 设置接收NTP包的数据结构 */
        ret_time->leap_ver_mode = ntohl(data[0]);
        ret_time->startum = ntohl(data[1]);
        ret_time->poll = ntohl(data[2]);
        ret_time->precision = ntohl(data[3]);
        ret_time->root_delay = ntohl(*(int*)&(data[4]));
        ret_time->root_dispersion = ntohl(*(int*)&(data[8]));
        ret_time->reference_identifier = ntohl(*(int*)&(data[12]));
        ret_time->reference_timestamp.coarse = ntohl(*(int*)&(data[16]));
        ret_time->reference_timestamp.fine = ntohl(*(int*)&(data[20]));
        ret_time->originage_timestamp.coarse = ntohl(*(int*)&(data[24]));
        ret_time->originage_timestamp.fine = ntohl(*(int*)&(data[28]));
        ret_time->receive_timestamp.coarse = ntohl(*(int*)&(data[32]));
        ret_time->receive_timestamp.fine = ntohl(*(int*)&(data[36]));
        ret_time->transmit_timestamp.coarse = ntohl(*(int*)&(data[40]));
        ret_time->transmit_timestamp.fine = ntohl(*(int*)&(data[44]));
        return 1;
    } /* end of if select */
    return 0;
}

void set_local_time(struct ntp_packet * pnew_time_packet)
{
     rtc_write(pnew_time_packet->transmit_timestamp.coarse - JAN_1970);
}

void get_ntp_time()
{
    int sockfd = -1, ret,rc;
    struct sockaddr_in server_addr;
    struct addrinfo hints, *res = NULL;
    struct ntp_packet new_time_packet;
    printf("\nntp start\n");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    rc = getaddrinfo(NTP_SERVER_IP, NTP_PORT_STR, &hints, &res);
    if (rc != 0)
    {
    printf("ERROR: getaddrinfo(%d)\n", rc);
    goto exit;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd <0 )
    {
    printf("ERROR: socket(%d)\n", sockfd);
    goto exit;
    }    

    if (send_to_ntp(sockfd, res, &new_time_packet))
    {
        /*调整本地时间*/
        set_local_time(&new_time_packet);
        printf("NTP client success!\n");
    }    

exit:
    printf("\nexit============\n");
    if(sockfd != -1) 
        close(sockfd);
    if(res != NULL)
        freeaddrinfo(res);
}


/*************************************************************
report hisdata to iot
*************************************************************/
#define ESIOT_URL    "129.1.99.4"
//#define ESIOT_URL    "192.168.1.149"
#define ESIOT_PORT    1883

void print_recv_data(uint8_t *recvbuf,int ret)
{
       printf("net_recv ret(%d) : ", ret);
    	for (int i=0;i<ret;i++)
    	{
    	    printf("%02x ", recvbuf[i]);
    	}
    	printf("\n");
}

bool mqtt_send_conn(void)
{


}

void creatIotHisdata()
{

}

void print_time(char *timebuf)
{
    struct tm timeinfo;
    if (rtc_isenabled())
    {
        read_locoltime(&timeinfo);
        sprintf(timebuf, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year,timeinfo.tm_mon,
        timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
    }
}

#define MQTT_USER_NAME "ac64218d-51de-46fc-8686-e6f241b3f34f"
#define MQTT_USER_PWD "Sg3Uvr58"
#define MQTT_DOMAIN "56QXIADEK"
#define MQTT_TOPIC "56QXIADEK/topic1"

int report_hisdata_to_iot(uint8_t data1, uint8_t data2, struct tm timeinfo)
{
      int server_fd = -1, ret,result = -1;
      uint8_t recvbuf[100];
      fd_set pending_data;
      struct timeval block_time;
      char name[40] = MQTT_USER_NAME;
      char pwd[20] = MQTT_USER_PWD;
      char clentid[32];
      get_pushid(clentid);
      
      printf("report_hisdata_to_iot!\n");

	if((ret = net_connect(&server_fd, ESIOT_URL, ESIOT_PORT)) != 0) {
		printf("ERROR: net_connect ret(%d)\n", ret);
		goto exit;
	}

	//ret = net_send(&server_fd, "hello", 6);
	//printf("net_send ret(%d)\n", ret);

	mqtt_connection_t con;
	u8 buf[200];
	mqtt_connect_info_t info;

       memset(buf,0,200);
	con.buffer = buf;
	con.buffer_length = sizeof(buf);
	con.message_id = 0xffff;

	info.client_id = clentid;
	info.username = name;
	info.password = pwd;
	info.will_topic = NULL;
	info.will_message = NULL;
	info.keepalive = 60;
	info.will_qos = 0;
	info.will_retain = 0;
	info.clean_session = 0;

	mqtt_msg_connect(&con, &info);
	if(0 == con.message.length) {
		printf("Mqtt fail to frame the connect\n");
	}

	ret = net_send(&server_fd, con.message.data, con.message.length);
	printf("net_send ret(%d)\n", ret);
	block_time.tv_sec=3;
       block_time.tv_usec=0;
       FD_ZERO(&pending_data);
       FD_SET(server_fd, &pending_data);

       if (select(server_fd + 1, &pending_data, NULL, NULL, &block_time) > 0){
        	ret = net_recv(&server_fd, recvbuf, 100);
        	print_recv_data(recvbuf, ret);
        	if ((ret != 4)||(recvbuf[3] != 0x00)||(recvbuf[0] != 0x20)){
        	    goto exit;
        	}
	}
	else {
	    printf("net_recv timeout\n", ret, recvbuf);
	    goto exit;
	}

       uint16_t msgid = 0xffff;
       char *payload;
       cJSON *root = cJSON_CreateObject();  

       char pushid[32];
	get_pushid(pushid);
	cJSON_AddStringToObject(root,"UUID",pushid);
       
       char timebuf[] = "1970-01-01 01:01:00";
       sprintf(timebuf, "%04d-%02d-%02d %02d:%02d:00", timeinfo.tm_year,timeinfo.tm_mon,
        timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min/15*15);
       cJSON_AddStringToObject(root,"time",timebuf);

	char vel[10];
	sprintf(vel, "%1.2f", (double)data1/100.0);
       cJSON_AddStringToObject(root,"data1",vel);

       sprintf(vel, "%1.2f", (double)data2/100.0);
       //cJSON_AddNumberToObject(root,"data2",123.5);
       cJSON_AddStringToObject(root,"data2",vel);
       
       payload = cJSON_Print(root);
	mqtt_msg_publish(&con, MQTT_TOPIC, payload, strlen(payload), 1, 0, &msgid);
	cJSON_Delete(root);
	free(payload);
	
	printf("msgid : %d\n", msgid);
	ret = net_send(&server_fd, con.message.data, con.message.length);
	FD_ZERO(&pending_data);
       FD_SET(server_fd, &pending_data);
       if (select(server_fd + 1, &pending_data, NULL, NULL, &block_time) > 0){
        	ret = net_recv(&server_fd, recvbuf, 100);
        	print_recv_data(recvbuf, ret);
        	if ((ret == 4)&&(recvbuf[0] == 0x40)){
        	    result = 1;
        	}
	}
	else {
	    printf("net_recv timeout\n", ret, recvbuf);
	}
	
exit:
    if(server_fd != -1)
    {
        printf("close fd!\n");
        net_close(server_fd);
    }
    return result;
}

extern outlet_state outletstateA;
extern outlet_state outletstateB;
int report_alarm_to_iot(outlet_state *outletstate)
{
      int server_fd = -1, ret,result = -1;
      uint8_t recvbuf[100];
      fd_set pending_data;
      struct timeval block_time;
      char name[40] = MQTT_USER_NAME;
      char pwd[20] = MQTT_USER_PWD;
      char clentid[32];
      get_pushid(clentid);
      
      printf("report_hisdata_to_iot!\n");

	if((ret = net_connect(&server_fd, ESIOT_URL, ESIOT_PORT)) != 0) {
		printf("ERROR: net_connect ret(%d)\n", ret);
		goto exit;
	}

	//ret = net_send(&server_fd, "hello", 6);
	//printf("net_send ret(%d)\n", ret);

	mqtt_connection_t con;
	u8 buf[200];
	mqtt_connect_info_t info;

       memset(buf,0,200);
	con.buffer = buf;
	con.buffer_length = sizeof(buf);
	con.message_id = 0xffff;

	info.client_id = clentid;
	info.username = name;
	info.password = pwd;
	info.will_topic = NULL;
	info.will_message = NULL;
	info.keepalive = 60;
	info.will_qos = 0;
	info.will_retain = 0;
	info.clean_session = 0;

	mqtt_msg_connect(&con, &info);
	if(0 == con.message.length) {
		printf("Mqtt fail to frame the connect\n");
	}

	ret = net_send(&server_fd, con.message.data, con.message.length);
	printf("net_send ret(%d)\n", ret);
	block_time.tv_sec=3;
       block_time.tv_usec=0;
       FD_ZERO(&pending_data);
       FD_SET(server_fd, &pending_data);

       if (select(server_fd + 1, &pending_data, NULL, NULL, &block_time) > 0){
        	ret = net_recv(&server_fd, recvbuf, 100);
        	print_recv_data(recvbuf, ret);
	}
	else {
	    printf("net_recv timeout\n", ret, recvbuf);
	}

       uint16_t msgid = 0xffff;
       char *payload = NULL;
       cJSON *root = cJSON_CreateObject();  

       char pushid[32];
	get_pushid(pushid);
	cJSON_AddStringToObject(root,"UUID",pushid);
       
       char timebuf[] = "1970-01-01 01:01:00";
       print_time(timebuf);
       cJSON_AddStringToObject(root,"time",timebuf);
       
       if (outletstate == (&outletstateA))
           cJSON_AddStringToObject(root,"channel","A");
       else if (outletstate == (&outletstateB))
           cJSON_AddStringToObject(root,"channel","B");
       
       if (outletstate->alarm & BIT0)
            cJSON_AddStringToObject(root,"type","1");
       else if (outletstate->alarm & BIT1)
            cJSON_AddStringToObject(root,"type","2");
       else if (outletstate->alarm & BIT2)
            cJSON_AddStringToObject(root,"type","3");
       else if ((outletstate->alarm & BIT3)||(outletstate->alarm & BIT4))
            cJSON_AddStringToObject(root,"type","4");
       else 
            cJSON_AddStringToObject(root,"type","0");

	char vel[10];
	sprintf(vel, "%3.2f", outletstate->thrvol_over);
       cJSON_AddStringToObject(root,"threshold",vel);
       sprintf(vel, "%3.2f", outletstate->metvol);
       cJSON_AddStringToObject(root,"meterage",vel);
       
       payload = cJSON_Print(root);
	mqtt_msg_publish(&con, MQTT_TOPIC, payload, strlen(payload), 1, 0, &msgid);
	
	
	printf("msgid : %d\n", msgid);
	ret = net_send(&server_fd, con.message.data, con.message.length);
	FD_ZERO(&pending_data);
       FD_SET(server_fd, &pending_data);
       if (select(server_fd + 1, &pending_data, NULL, NULL, &block_time) > 0){
        	ret = net_recv(&server_fd, recvbuf, 100);
        	print_recv_data(recvbuf, ret);
        	if ((ret == 4)&&(recvbuf[0] == 0x40)){
        	    result = 1;
        	}
	}
	else {
	    printf("net_recv timeout\n", ret, recvbuf);
	}
	
exit:
    if(server_fd != -1)
    {
        printf("close fd!\n");
        net_close(server_fd);
    }
    cJSON_Delete(root);
    free(payload);
    return result;
}

