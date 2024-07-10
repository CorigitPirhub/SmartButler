#include <stdio.h>

#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "hi_errno.h"

#include "../lib/shared.h"
float global_inte = 0;
static void *PhotoTask(const char *arg)
{
    unsigned short data;
    int ret;

    while (1) {
        ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_4, &data, 
                        HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0);
        if(data == 0){
            global_inte = 100*116;
        }
        else{
            global_inte = 100*116/data;
        }
        usleep(100000);
    }

    return NULL;
}

static void PhotoExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "PhotoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)PhotoTask, NULL, &attr) == NULL) {
        printf("[PhotoExample] Falied to create PhotoTask!\n");
    }
}

SYS_RUN(PhotoExampleEntry);