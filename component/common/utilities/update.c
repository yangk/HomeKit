#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <lwip/sockets.h>
#include <sys.h>
#include "phytrex_update.h"
#include "flash_api.h"
#include "update.h"
#include "other/outlet/outlet.h"

#define OFFSET_DATA		FLASH_SYSTEM_DATA_ADDR
#define IMAGE_2			0x0000B000
#define WRITE_OTA_ADDR		1
#define CONFIG_CUSTOM_SIGNATURE 1
#define SWAP_UPDATE 0

#if WRITE_OTA_ADDR
#define BACKUP_SECTOR	(FLASH_SYSTEM_DATA_ADDR - 0x1000)
#endif

#define STACK_SIZE		1024
#define TASK_PRIORITY	tskIDLE_PRIORITY + 1
#define BUF_SIZE		512
#define ETH_ALEN	6

typedef struct
{
	uint32_t	ip_addr;
	uint16_t	port;
}update_cfg_local_t;

#define REPOSITORY_LEN	16
#define FILE_PATH_LEN	64
typedef struct
{
	uint8_t	 	repository[REPOSITORY_LEN];
	uint8_t		file_path[FILE_PATH_LEN];
}update_cfg_cloud_t;

sys_thread_t TaskOTA = NULL;
extern PhytrexParameter_t ex_param;
extern MyVersion cur_ver;

//---------------------------------------------------------------------
static void* update_malloc(unsigned int size)
{
	return pvPortMalloc(size);
}

//---------------------------------------------------------------------
static void update_free(void *buf)
{
	vPortFree(buf);
}

//---------------------------------------------------------------------

void ota_platform_reset(void)
{
	//wifi_off();

	// Set processor clock to default before system reset
	HAL_WRITE32(SYSTEM_CTRL_BASE, 0x14, 0x00000021);
	osDelay(100);

	// Cortex-M3 SCB->AIRCR
	HAL_WRITE32(0xE000ED00, 0x0C, (0x5FA << 16) |                             // VECTKEY
	                              (HAL_READ32(0xE000ED00, 0x0C) & (7 << 8)) | // PRIGROUP
	                              (1 << 2));                                  // SYSRESETREQ
	while(1) osDelay(1000);
}
#if WRITE_OTA_ADDR
int write_ota_addr_to_system_data(flash_t *flash, uint32_t ota_addr)
{
	uint32_t data, i = 0;
	//Get upgraded image 2 addr from offset
	flash_read_word(flash, OFFSET_DATA, &data);
	printf("\n\r[%s] data 0x%x ota_addr 0x%x", __FUNCTION__, data, ota_addr);
	if(data == ~0x0){
		flash_write_word(flash, OFFSET_DATA, ota_addr);
	}else{
		//erase backup sector
		flash_erase_sector(flash, BACKUP_SECTOR);
		//backup system data to backup sector
		for(i = 0; i < 0x1000; i+= 4){
			flash_read_word(flash, OFFSET_DATA + i, &data);
			if(i == 0)
				data = ota_addr;
			flash_write_word(flash, BACKUP_SECTOR + i,data);
		}
		//erase system data
		flash_erase_sector(flash, OFFSET_DATA);
		//write data back to system data
		for(i = 0; i < 0x1000; i+= 4){
			flash_read_word(flash, BACKUP_SECTOR + i, &data);
			flash_write_word(flash, OFFSET_DATA + i,data);
		}
		//erase backup sector
		flash_erase_sector(flash, BACKUP_SECTOR);
	}
	return 0;
}
#endif
static void update_ota_local_task(void *param)
{
	int server_socket;
	struct sockaddr_in server_addr;
	unsigned char *buf;
        union { uint32_t u; unsigned char c[4]; } file_checksum;
	int read_bytes = 0, size = 0, i = 0;
	update_cfg_local_t *cfg = (update_cfg_local_t *)param;
	uint32_t address, checksum = 0;
	flash_t	flash;
	uint32_t NewImg2BlkSize = 0, NewImg2Len = 0, NewImg2Addr = 0, file_info[3];
	uint32_t Img2Len = 0;
	int ret = -1 ;
	//uint8_t signature[8] = {0x38,0x31,0x39,0x35,0x38,0x37,0x31,0x31};
	uint32_t IMAGE_x = 0, ImgxLen = 0, ImgxAddr = 0;
#if WRITE_OTA_ADDR
	uint32_t ota_addr = 0x80000;
#endif
#if CONFIG_CUSTOM_SIGNATURE
	char custom_sig[32] = "Customer Signature-modelxxx";
	uint32_t read_custom_sig[8];
#endif
	printf("\n\r[%s] Update task start", __FUNCTION__);
	buf = update_malloc(BUF_SIZE);
	if(!buf){
		printf("\n\r[%s] Alloc buffer failed", __FUNCTION__);
		goto update_ota_exit;
	}
	// Connect socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0){
		printf("\n\r[%s] Create socket failed", __FUNCTION__);
		goto update_ota_exit;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = cfg->ip_addr;
	server_addr.sin_port = cfg->port;

	if(connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("\n\r[%s] socket connect failed", __FUNCTION__);
		goto update_ota_exit;
	}
	DBG_INFO_MSG_OFF(_DBG_SPI_FLASH_);

#if 1
	// The upgraded image2 pointer must 4K aligned and should not overlap with Default Image2
	flash_read_word(&flash, IMAGE_2, &Img2Len);
	IMAGE_x = IMAGE_2 + Img2Len + 0x10;
	flash_read_word(&flash, IMAGE_x, &ImgxLen);
	flash_read_word(&flash, IMAGE_x+4, &ImgxAddr);
	if(ImgxAddr==0x30000000){
		printf("\n\r[%s] IMAGE_3 0x%x Img3Len 0x%x", __FUNCTION__, IMAGE_x, ImgxLen);
	}else{
		printf("\n\r[%s] no IMAGE_3", __FUNCTION__);
		// no image3
		IMAGE_x = IMAGE_2;
		ImgxLen = Img2Len;
	}
#if WRITE_OTA_ADDR
	if((ota_addr > IMAGE_x) && ((ota_addr < (IMAGE_x+ImgxLen))) ||
            (ota_addr < IMAGE_x) ||
            ((ota_addr & 0xfff) != 0)||
	      (ota_addr == ~0x0)){
		printf("\n\r[%s] illegal ota addr 0x%x", __FUNCTION__, ota_addr);
		goto update_ota_exit;
	}else
	    write_ota_addr_to_system_data( &flash, ota_addr);
#endif
	//Get upgraded image 2 addr from offset
	flash_read_word(&flash, OFFSET_DATA, &NewImg2Addr);
	if((NewImg2Addr > IMAGE_x) && ((NewImg2Addr < (IMAGE_x+ImgxLen))) ||
            (NewImg2Addr < IMAGE_x) ||
            ((NewImg2Addr & 0xfff) != 0)||
	      (NewImg2Addr == ~0x0)){
		printf("\n\r[%s] Invalid OTA Address 0x%x", __FUNCTION__, NewImg2Addr);
		goto update_ota_exit;
	}
#else
	//For test, hard code addr
	NewImg2Addr = 0x80000;	
#endif
	
	//Clear file_info
	memset(file_info, 0, sizeof(file_info));
	
	if(file_info[0] == 0){
		printf("\n\r[%s] Read info first", __FUNCTION__);
		read_bytes = read(server_socket, file_info, sizeof(file_info));
		// !X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X
		// !W checksum !W padding 0 !W file size !W
		// !X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X!X
		printf("\n\r[%s] info %d bytes", __FUNCTION__, read_bytes);
		printf("\n\r[%s] tx chechsum 0x%x, file size 0x%x", __FUNCTION__, file_info[0],file_info[2]);
		if(file_info[2] == 0){
			printf("\n\r[%s] No checksum and file size", __FUNCTION__);
			goto update_ota_exit;
		}
	}
	
#if SWAP_UPDATE
	uint32_t SigImage0,SigImage1;
	uint32_t Part1Addr=0xFFFFFFFF, Part2Addr=0xFFFFFFFF, ATSCAddr=0xFFFFFFFF;
	uint32_t OldImg2Addr;
	flash_read_word(&flash, 0x18, &Part1Addr);
	Part1Addr = (Part1Addr&0xFFFF)*1024;	// first partition
	Part2Addr = NewImg2Addr;
	
	// read Part1/Part2 signature
	flash_read_word(&flash, Part1Addr+8, &SigImage0);
	flash_read_word(&flash, Part1Addr+12, &SigImage1);
	printf("\n\r[%s] Part1 Sig %x", __FUNCTION__, SigImage0);
	if(SigImage0==0x30303030 && SigImage1==0x30303030)
		ATSCAddr = Part1Addr;		// ATSC signature
	else if(SigImage0==0x35393138 && SigImage1==0x31313738)	
		OldImg2Addr = Part1Addr;	// newer version, change to older version
	else
		NewImg2Addr = Part1Addr;	// update to older version	
	
	flash_read_word(&flash, Part2Addr+8, &SigImage0);
	flash_read_word(&flash, Part2Addr+12, &SigImage1);
	printf("\n\r[%s] Part2 Sig %x", __FUNCTION__, SigImage0);
	if(SigImage0==0x30303030 && SigImage1==0x30303030)
		ATSCAddr = Part2Addr;		// ATSC signature
	else if(SigImage0==0x35393138 && SigImage1==0x31313738)
		OldImg2Addr = Part2Addr;
	else
		NewImg2Addr = Part2Addr;
	
	// update ATSC clear partitin first
	if(ATSCAddr != ~0x0){
		OldImg2Addr = NewImg2Addr;
		NewImg2Addr = ATSCAddr;
	}
	
	printf("\n\r[%s] New %x, Old %x", __FUNCTION__, NewImg2Addr, OldImg2Addr);
	
	if( NewImg2Addr==Part1Addr ){
		if( file_info[2] > (Part2Addr-Part1Addr) ){	// firmware size too large
			printf("\n\r[%s] Part1 size < OTA size", __FUNCTION__);
			goto update_ota_exit;
			// or update to partition2
			// NewImg2Addr = Part2Addr;	
		}
	}
		
#endif

	//Erase upgraded image 2 region
	if(NewImg2Len == 0){
		NewImg2Len = file_info[2];
		printf("\n\r[%s] NewImg2Len %d  ", __FUNCTION__, NewImg2Len);
		if((int)NewImg2Len > 0){
			NewImg2BlkSize = ((NewImg2Len - 1)/4096) + 1;
			printf("\n\r[%s] NewImg2BlkSize %d  0x%8x", __FUNCTION__, NewImg2BlkSize, NewImg2BlkSize);
			for( i = 0; i < NewImg2BlkSize; i++)
				flash_erase_sector(&flash, NewImg2Addr + i * 4096);
		}else{
			printf("\n\r[%s] Size INVALID", __FUNCTION__);
			goto update_ota_exit;
		}
	}	
	
	printf("\n\r[%s] NewImg2Addr 0x%x", __FUNCTION__, NewImg2Addr);
        
        // reset
        file_checksum.u = 0;
	// Write New Image 2 sector
	if(NewImg2Addr != ~0x0){
		address = NewImg2Addr;
		printf("\n\r");
		while(1){
			memset(buf, 0, BUF_SIZE);
			read_bytes = read(server_socket, buf, BUF_SIZE);
			if(read_bytes == 0) break; // Read end
			if(read_bytes < 0){
				printf("\n\r[%s] Read socket failed", __FUNCTION__);
				goto update_ota_exit;
			}
				checksum += file_checksum.c[0];              // not read end, this is not attached checksum
				checksum += file_checksum.c[1];
				checksum += file_checksum.c[2];
				checksum += file_checksum.c[3];
			//printf("\n\r[%s] read_bytes %d", __FUNCTION__, read_bytes);
			
			#if 1
			if(flash_stream_write(&flash, address + size, read_bytes, buf) < 0){
				printf("\n\r[%s] Write sector failed", __FUNCTION__);
				goto update_ota_exit;
			}
			size += read_bytes;
			for(i = 0; i < read_bytes-4; i ++)
				checksum += buf[i];
			file_checksum.c[0] = buf[read_bytes-4];      // checksum attached at file end
			file_checksum.c[1] = buf[read_bytes-3];
			file_checksum.c[2] = buf[read_bytes-2];
			file_checksum.c[3] = buf[read_bytes-1];
			#else
			size += read_bytes;
			for(i = 0; i < read_bytes-4; i ++){
				checksum += buf[i];				
			}	
			file_checksum.c[0] = buf[read_bytes-4];      // checksum attached at file end
			file_checksum.c[1] = buf[read_bytes-3];
			file_checksum.c[2] = buf[read_bytes-2];
			file_checksum.c[3] = buf[read_bytes-1];
			#endif			
		}
		printf("\n\r");
		printf("\n\rUpdate file size = %d  checksum 0x%x  attached checksum 0x%x", size, checksum, file_checksum.u);
#if CONFIG_WRITE_MAC_TO_FLASH
		//Write MAC address
		if(!(mac[0]==0xff&&mac[1]==0xff&&mac[2]==0xff&&mac[3]==0xff&&mac[4]==0xff&&mac[5]==0xff)){
			if(flash_write_word(&flash, FLASH_ADD_STORE_MAC, mac, ETH_ALEN) < 0){
				printf("\n\r[%s] Write MAC failed", __FUNCTION__);
				goto update_ota_exit;
			}	
		}
#endif
		//printf("\n\r checksum 0x%x  file_info 0x%x  ", checksum, *(file_info));
#if CONFIG_CUSTOM_SIGNATURE
		for(i = 0; i < 8; i ++){
		    flash_read_word(&flash, NewImg2Addr + 0x28 + i *4, read_custom_sig + i);
		}
		printf("\n\r[%s] read_custom_sig %s", __FUNCTION__ , (char*)read_custom_sig);
#endif
		// compare checksum with received checksum
		//if(!memcmp(&checksum,file_info,sizeof(checksum))
		if( (file_checksum.u == checksum)
#if CONFIG_CUSTOM_SIGNATURE
			&& !strcmp((char*)read_custom_sig,custom_sig)
#endif
			){
			
			//Set signature in New Image 2 addr + 8 and + 12
			uint32_t sig_readback0,sig_readback1;
			flash_write_word(&flash,NewImg2Addr + 8, 0x35393138);
			flash_write_word(&flash,NewImg2Addr + 12, 0x31313738);
			flash_read_word(&flash, NewImg2Addr + 8, &sig_readback0);
			flash_read_word(&flash, NewImg2Addr + 12, &sig_readback1);
			printf("\n\r[%s] signature %x,%x,  checksum 0x%x", __FUNCTION__ , sig_readback0, sig_readback1, checksum);
#if SWAP_UPDATE
			if(OldImg2Addr != ~0x0){
				flash_write_word(&flash,OldImg2Addr + 8, 0x35393130);
				flash_write_word(&flash,OldImg2Addr + 12, 0x31313738);
				flash_read_word(&flash, OldImg2Addr + 8, &sig_readback0);
				flash_read_word(&flash, OldImg2Addr + 12, &sig_readback1);
				printf("\n\r[%s] old signature %x,%x", __FUNCTION__ , sig_readback0, sig_readback1);
			}
#endif			
			printf("\n\r[%s] Update OTA success!", __FUNCTION__);
			
			ret = 0;
		}
	}
update_ota_exit:
	if(buf)
		update_free(buf);
	if(server_socket >= 0)
		close(server_socket);
	if(param)
		update_free(param);
	TaskOTA = NULL;
	printf("\n\r[%s] Update task exit", __FUNCTION__);	
	if(!ret){
		printf("\n\r[%s] Ready to reboot", __FUNCTION__);	
		ota_platform_reset();
	}
	vTaskDelete(NULL);	
	return;

}

//---------------------------------------------------------------------
int update_ota_local(char *ip, int port)
{
#ifdef PHYTREX
        phytrex_update_ota_local(ip, port);
#else
	update_cfg_local_t *pUpdateCfg;
	
	if(TaskOTA){
		printf("\n\r[%s] Update task has created.", __FUNCTION__);
		return 0;
	}
	pUpdateCfg = update_malloc(sizeof(update_cfg_local_t));
	if(pUpdateCfg == NULL){
		printf("\n\r[%s] Alloc update cfg failed", __FUNCTION__);
		return -1;
	}
	pUpdateCfg->ip_addr = inet_addr(ip);
	pUpdateCfg->port = ntohs(port);

	TaskOTA = sys_thread_new("OTA_server", update_ota_local_task, pUpdateCfg, STACK_SIZE, TASK_PRIORITY);
	if(TaskOTA == NULL){
	  	update_free(pUpdateCfg);
		printf("\n\r[%s] Create update task failed", __FUNCTION__);
	}
#endif
	return 0;
}

//---------------------------------------------------------------------

static void update_ota_cloud_task(void *param)
{
  return;
}

void ES_phytrex_update_ota_amazon_cloud_task(char *param, int len)
{
    struct sockaddr_in server_addr;
    static uint32_t NewImg2Addr = 0, OldImg2Addr = 0;
    int ret = 0;
    MyVersion read_ver = {0};
    static int read_size = 0,
    resource_size = 0,
    header_removed = 0,
    content_len = 0,
    crc32 = 0,
    _crc32 = 0;
    char  *body = NULL,
            *fw_ver_pos = NULL, *hw_ver_pos = NULL, *sw_ver_pos = NULL,
            *crc32_pos = NULL,
            *content_len_pos = NULL;
    char *header = NULL;


    int k = 0;
    int outlen = 1500;
    unsigned char valbuf[1500];
    memset(valbuf, 0xFF, 1500);
    base64_decode(valbuf, &outlen, param, len);

    if (7 == outlen)
    {
        #if 0
        if(!phytrex_write_ota_addr_to_system_data(OTA_SECTOR)){
        printf("\n\nwrite_ota_addr NewImg2Addr error: 0x%x\n\n", NewImg2Addr);
        ret = 2;
        goto update_ota_exit_1;
        }
        #endif
        if(!phytrex_read_ota_addr_to_system_data(&NewImg2Addr)) {
        ret = 2;
        goto update_ota_exit_1;
        }
        //NewImg2Addr = OTA_SECTOR;	
        //printf("\r\nNewImg2Addr: 0x%x\n", NewImg2Addr);
        content_len = 500000;
        phytrex_ReadSwapAddr(&NewImg2Addr, &OldImg2Addr);
        phytrex_FlashUpdateErase(NewImg2Addr, content_len);
        //printf("\r\nNewImg2Addr: 0x%x\n", NewImg2Addr);
        //printf("\r\nOldImg2Addr: 0x%x\n", OldImg2Addr);
    }
    else if (3 == outlen)
    {
        //CRC32 checksum
        _crc32 = phytrex_CRC32_checksum(NewImg2Addr, resource_size);
        phytrex_read_version(&read_ver);
        phytrex_WriteSwapSig(NewImg2Addr, OldImg2Addr);

        printf("\n\rhttp content-length:    %d bytes", content_len);
        printf("\n\rcrc32 checksum:         0x%08x", _crc32);
        printf("\n\rfirmware revision:      %s", cur_ver.firmware);
        printf("\n\rsoftware revision:      %s", cur_ver.software);
        printf("\n\rdownload resource size: %d bytes", resource_size);
        ret = 0;
#if WRITE_OTA_ADDR
        if(!phytrex_write_ota_addr_to_system_data(OTA_SECTOR))
            return;
#endif
        phytrex_reset(4);
    }  
    else
    {
        read_size = outlen-2;
        // Write data to ota sectors
        phytrex_FlashUpdateWrite(NewImg2Addr, resource_size, read_size, &valbuf[2]);
        resource_size += read_size;
        printf("\rUpdate file size = %d/%d bytes  %3.1f %%",
               resource_size, content_len, (float)resource_size/(float)content_len*100.0f);
    }
    

update_ota_exit:

update_ota_exit_1:
    if(ret == 2) {
    printf("\nphytrex_read_ota_addr_to_system_data ERROE\n");
    }
    else if(ret == 3) {
    printf("\nphytrex_write_ota_addr_to_system_data ERROE\n");
    }
    else {
    }
    return;
}

void ES_mcu_ota_task(char *param, int len)
{
    flash_t flash;
    int i;
    static int size = 0;
    int outlen = 1500;
    unsigned char valbuf[1500];
    
    memset(valbuf, 0xFF, 1500);
    base64_decode(valbuf, &outlen, param, len);

    if (7 == outlen)
    {
        for(i=0;i<8;i++)
        {
            flash_erase_sector(&flash, MCU_OTA_ADDR+i*0x1000);
        }
        printf("\n\r[%s] mcu_ota_task begin\n", __FUNCTION__);
    }
    else if (3 == outlen)
    {
        creatFristPacket();
    }  
    else
    {
        if(flash_stream_write(&flash, MCU_OTA_ADDR + size, outlen-2, &valbuf[2]) < 0){
            printf("\n\r[%s] Write sector failed", __FUNCTION__);
            return;
        }
        size += outlen-2;
        printf("\r read len : %d  %d", (int)(valbuf[1]<<8)+(int)valbuf[0], size);
    }

}
//---------------------------------------------------------------------
int update_ota_cloud(char *repository, char *file_path)
{
#ifdef PHYTREX
	phytrex_update_cfg_cloud_t res = {0};
	strcpy(res.host_name, repository);
	strcpy(res.file_path, file_path);
	phytrex_update_ota_cloud(&res);
#else
	update_cfg_cloud_t *pUpdateCfg;
	
	if(TaskOTA){
		printf("\n\r[%s] Update task has created.", __FUNCTION__);
		return 0;
	}
	pUpdateCfg = update_malloc(sizeof(update_cfg_cloud_t));
	if(pUpdateCfg == NULL){
		printf("\n\r[%s] Alloc update cfg failed.", __FUNCTION__);
		goto exit;
	}
	if(strlen(repository) > (REPOSITORY_LEN-1)){
		printf("\n\r[%s] Repository length is too long.", __FUNCTION__);
		goto exit;
	}
	if(strlen(file_path) > (FILE_PATH_LEN-1)){
		printf("\n\r[%s] File path length is too long.", __FUNCTION__);
		goto exit;
	}
	strcpy((char*)pUpdateCfg->repository, repository);
	strcpy((char*)pUpdateCfg->file_path, file_path);

	TaskOTA = sys_thread_new("OTA_server", update_ota_cloud_task, pUpdateCfg, STACK_SIZE, TASK_PRIORITY);
	if(TaskOTA == NULL){	  	
		printf("\n\r[%s] Create update task failed", __FUNCTION__);
		goto exit;
	}

exit:
	update_free(pUpdateCfg);
#endif
	return 0;
}

//---------------------------------------------------------------------
void cmd_update(int argc, char **argv)
{
  if(argc == 4) {
    if(strcmp(argv[3], "1") == 0) {
	int port;
	port = atoi(argv[2]);
	update_ota_local(argv[1], port);
    }
    else if(strcmp(argv[3], "2") == 0) {
	update_ota_cloud(argv[1], argv[2]);
    }
  }
  else {
        printf("\n\r[%s] Usage: update IP PORT", __FUNCTION__);
        printf("\n\r[%s] Usage: update REPOSITORY FILE_PATH", __FUNCTION__);
  }
}

//---------------------------------------------------------------------

