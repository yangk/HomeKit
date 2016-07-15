#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>
#include "flash_api.h"
#include "other/outlet/outlet.h"
extern void console_init(void);


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */


#define HISMAGIC    0x5a5aaca7
void HisdataInit()
{
    int32_t data, i, hisaddr, j = 0;
    flash_t flash;
    unsigned char hisdata[746];
    char mondata[72] = {
    0x0f,0x03,0x00,0x00,0x12,0x34,
    0x0f,0x04,0x00,0x00,0x56,0x34,
    0x0f,0x05,0x00,0x00,0x12,0x34,
    0x0f,0x06,0x00,0x00,0x56,0x34,
    0x0f,0x07,0x00,0x00,0x12,0x34,
    0x0f,0x08,0x00,0x00,0x56,0x34,
    0x0f,0x09,0x00,0x00,0x12,0x34,
    0x0f,0x0a,0x00,0x00,0x56,0x34,
    0x0f,0x0b,0x00,0x00,0x12,0x34,
    0x0f,0x0c,0x00,0x00,0x56,0x34,
    0x10,0x01,0x00,0x00,0x76,0x34,
    0x10,0x02,0x00,0x00,0x26,0x34,
    };
    flash_read_word(&flash, OUTLET_A_HISADDR+0x7000-4, &data);
    printf("[%s] magic  0x%x \n", __FUNCTION__, data);
    if (data == HISMAGIC)
    {
        return;
    }
    printf("start write flash data!!!! \n", __FUNCTION__);
    hisaddr = OUTLET_A_HISADDR;
    flash_erase_sector(&flash, OUTLET_A_HISADDR);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x1000);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x2000);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x3000);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x4000);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x5000);
    flash_erase_sector(&flash, OUTLET_A_HISADDR+0x6000);
    //Get upgraded image 2 addr from offset
    for (i = 0; i < 24; i++)
    {
        hisdata[0] = 15;
        hisdata[1] = (i%12) + 1;
        if ((hisdata[1]==1)||(hisdata[1]==2))
        {
            //hisdata[1] = hisdata[1]-12;
            hisdata[0]++;
        }
        for (j = 2; j < 746; j++)
        {
            //hisdata[j] = j%24 + i*10;
            rtw_get_random_bytes(&hisdata[j], 1);
            hisdata[j] >>= 1;
        }
        flash_stream_write(&flash, hisaddr, 746, hisdata);
        hisaddr += 0x400;
    }
    flash_stream_write(&flash, OUTLET_A_HISADDR+0x6000, 72, mondata);
    flash_write_word(&flash, OUTLET_A_HISADDR+0x7000-4, HISMAGIC);
    //flash_read_word(&flash, 0xF9000, &data);
}



void main(void)
{
	if(rtl_cryptoEngine_init() != 0) {
		DiagPrintf("crypto engine init failed\r\n");
	}

	/* Initialize log uart and at command service */
	console_init();

	/* pre-processor of application example */
	pre_example_entry();

        init_gpio();

	/* wlan intialization */
#if defined(CONFIG_WIFI_NORMAL) && defined(CONFIG_NETWORK)
	wlan_network();
#endif

	/* Execute application example */
	example_entry();
	printf("=======================\n");
	printf("=======================\n");
	printf("=======================\n");
	printf("=======================\n");
	printf("==========8813==========\n");
	printf("=======================\n");
	printf("=======================\n");
	printf("=======================\n");
	/*Enable Schedule, Start Kernel*/
    //uart_test();
        uart_start();
        WACSetupDebug(1);
        HAPSetupDebug(1);
        //HisdataInit();
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
	#ifdef PLATFORM_FREERTOS
	vTaskStartScheduler();
	#endif
#else
	RtlConsolTaskRom(NULL);
#endif
}
