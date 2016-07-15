#include "es_update.h"


extern cJSON *MyOtaService;
extern PhytrexParameter_t ex_param;

void printf_update_res(phytrex_update_cfg_cloud_t *update_res)
{
    printf("==========update_res:===========\n");
    printf("full_path:%s\n",update_res->full_path);
    printf("crc32:%d\n",update_res->crc32);
    printf("method:%d\n",update_res->method);
    printf("host_name:%s\n",update_res->host_name);
    printf("file_path:%s\n",update_res->file_path);
    /*
    printf("update_result:%d\n",update_res->update_result[0]);
    printf("update_result:%d\n",update_res->update_result[1]);
    printf("update_result:%d\n",update_res->update_result[2]);
    printf("update_result:%d\n",update_res->update_result[3]);
    printf("update_result:%d\n",update_res->update_result[4]);
    */
    printf("=============================\n");
}
void ES_phytrex_OtaHandler(void)
{
    printf("OtaStart::\n");

    phytrex_update_cfg_cloud_t update_res = {0};
    //printf_update_res(&update_res);
    phytrex_update_set_result(&update_res, 0, 1, 2, 3, 4);
    //printf_update_res(&update_res);
    memcpy(update_res.full_path, ex_param.fota_path, sizeof(update_res.full_path));
    printf_update_res(&update_res);
    if(phytrex_get_latest_version(&update_res))
    {
        printf("Firmware verison is not latest\n");
        //printf_update_res(&update_res);
        phytrex_update_ota_cloud(&update_res);
    }
    else
    {
        //printf_update_res(&update_res);
        printf("Firmware verison is latest\n");
    }
}