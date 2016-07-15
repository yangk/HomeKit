#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>

extern void wlan_netowrk(void);
extern void console_init(void);


void user_wifi_beacon_hdl( char* buf, int buf_len, int flags, void* userdata)
{
	//printf("Beacon!\n");
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	if(rtl_cryptoEngine_init() != 0) {
		DiagPrintf("crypto engine init failed\r\n");
	}

	/* Initialize log uart and at command service */
	console_init();

	// Setup init sequence for test
	setup_init_sequence();

	/* wlan intialization */
#if defined(CONFIG_WIFI_NORMAL) && defined(CONFIG_NETWORK)
	wlan_network();
#endif

	/*Enable Schedule, Start Kernel*/
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
	#ifdef PLATFORM_FREERTOS
	vTaskStartScheduler();
	#endif
#else
	RtlConsolTaskRom(NULL);
#endif
}
