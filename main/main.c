#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "tps65381a.h"
#include "L9906.h"
#include "driver/timer.h"
#include "esp_task_wdt.h"
#include "driver/adc.h"
#include "driver/mcpwm.h"
#include "driver/gpio.h"

#define WDI GPIO_NUM_2

int wdi_level = 0;
//static QueueHandle_t xQueue;
static xQueueHandle s_timer_queue;

//vTaskDelay(6/portTICK_PERIOD_MS);

mcpwm_pin_config_t mcpwm_pin_cfg = {
    .mcpwm0a_out_num = GPIO_NUM_25,
    .mcpwm0b_out_num = GPIO_NUM_26,
    .mcpwm1a_out_num = GPIO_NUM_27,
    .mcpwm1b_out_num = GPIO_NUM_14,
    .mcpwm2a_out_num = GPIO_NUM_12,
    .mcpwm2b_out_num = GPIO_NUM_13,
};

mcpwm_config_t mcpwm_cfg = {
    .frequency = 16000,//Hz
    .cmpr_a = 50.0,//duty_cycle = 50%
    .cmpr_b = 50.0,//duty_cycle = 50%
    .duty_mode = MCPWM_DUTY_MODE_0,
    .counter_mode = MCPWM_UP_DOWN_COUNTER,
};

adc_digi_pattern_table_t adc_pattern = {
    .val = 0xCF,
};

adc_digi_config_t adc_cfg = {
    .conv_limit_en = 0,
    .conv_limit_num = 255,
    .adc1_pattern_len = 16,
    .adc2_pattern_len = 16,
    .adc1_pattern = &adc_pattern,
    .adc2_pattern = &adc_pattern,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = ADC_DIGI_FORMAT_12BIT,
};

void so1_dma_read(void)
{
    adc_digi_controller_config(&adc_cfg);
    adc_digi_init();
}

void read_so1(void)
{
    // esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    //     esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_6 , ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
    int val = adc1_get_raw(ADC1_CHANNEL_6);
    printf("SO1 Voltage is:%d;\r\n",val);
}

void read_so2(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
    int val = adc1_get_raw(ADC1_CHANNEL_7);
    printf("SO2 Voltage is:%d;\r\n",val);
}



static bool IRAM_ATTR pmic_wtd1(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    wdi_level = wdi_level^1;
    gpio_set_level(WDI, wdi_level);
    if (wdi_level == 1)
    {
        timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000);
    }
    else{timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 9000);}
    return high_task_awoken == pdTRUE;
}

void timer_task1(void)
{
  timer_config_t timer_cfg={
      .alarm_en = TIMER_ALARM_EN,
      .counter_en = TIMER_PAUSE,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = TIMER_AUTORELOAD_EN,
      .divider = 80,
      //.clk_src = TIMER_SRC_CLK_APB,
      };
  timer_init(TIMER_GROUP_0, TIMER_0, &timer_cfg);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 9000);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, pmic_wtd1, NULL, 0);
  timer_start(TIMER_GROUP_0, TIMER_0);
}

void app_main(void)
{
    pmic_handle_t pmic_dev;
    predrv_handle_t predrv_dev;
    s_timer_queue = xQueueCreate(10, sizeof(int));
    //blink();
    
    pmic_spi_init((pmic_context_t**)&pmic_dev);
    predrv_spi_init((predrv_context_t**)&predrv_dev);

    wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_CHECK_CTRL, 0x06);
    wr_pmic((pmic_context_t*)pmic_dev, SW_UNLOCK, 0x55);
    //Init Watchdog Window
    wr_pmic((pmic_context_t*)pmic_dev, WR_WD_WIN1_CFG, 0x0e);//14*0.55*1.05 = 8.05ms -- 13*0.55*0.95 = 6.79 ms
    wr_pmic((pmic_context_t*)pmic_dev, WR_WD_WIN2_CFG, 0x06);//(6+1)*0.55*1 = 3.85ms
    timer_task1();
    // wr_pmic((pmic_context_t*)pmic_dev, WR_DEV_CFG1, 0x80);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_DEV_CFG2, 0xc0);
    wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_ERR_CFG, 0x00);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_FUNC_CFG, 0x09);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_ERR_PWM_H, 0x00);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_ERR_PWM_L, 0x00);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_PWD_THR_CFG, 0x01);
    // wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_CFG_CRC, 0x10);
    wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_CHECK_CTRL, 0x05);
    //wr_pmic((pmic_context_t*)pmic_dev, WR_SENS_CTRL, 0x00);//关闭5V

    //wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_BIST_CTRL, 0xe0);

    wr_predrv((predrv_context_t*)predrv_dev, CMD0, WR_CMD0);
    int cmd0_response = wr_predrv((predrv_context_t*)predrv_dev, CMD0, 0x0800);
    wr_predrv((predrv_context_t*)predrv_dev, CMD1, WR_CMD1);
    int cmd1_response = wr_predrv((predrv_context_t*)predrv_dev, CMD1, 0x0000);
    wr_predrv((predrv_context_t*)predrv_dev, CMD2, WR_CMD2);
    int cmd2_response = wr_predrv((predrv_context_t*)predrv_dev, CMD2, 0x0000);
    wr_predrv((predrv_context_t*)predrv_dev, CMD3, WR_CMD3);
    int cmd3_response = wr_predrv((predrv_context_t*)predrv_dev, CMD3, 0x0800);
    wr_predrv((predrv_context_t*)predrv_dev, CMD4, WR_CMD4);
    int cmd4_response = wr_predrv((predrv_context_t*)predrv_dev, CMD4, 0x0000);
    printf("Data in CMD0 Register is: %d;\r\n",cmd0_response);
    printf("Data in CMD1 Register is: %d;\r\n",cmd1_response);
    printf("Data in CMD2 Register is: %d;\r\n",cmd2_response);
    printf("Data in CMD3 Register is: %d;\r\n",cmd3_response);
    printf("Data in CMD4 Register is: %d;\r\n",cmd4_response);
    printf("cmd0 is %d;\r\n",WR_CMD0);
    printf("cmd1 is %d;\r\n",WR_CMD1);
    printf("cmd2 is %d;\r\n",WR_CMD2);
    printf("cmd3 is %d;\r\n",WR_CMD3);
    printf("cmd4 is %d;\r\n",WR_CMD4);
    wr_pmic((pmic_context_t*)pmic_dev, WR_SAFETY_CHECK_CTRL, 0x25);

    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0A,GPIO_NUM_25);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0B,GPIO_NUM_26);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM1A,GPIO_NUM_27);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM1B,GPIO_NUM_14);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM2A,GPIO_NUM_12);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM2B,GPIO_NUM_13);
    //mcpwm_set_pin(MCPWM_UNIT_0,&mcpwm_pin_cfg);
    mcpwm_init(MCPWM_UNIT_0,MCPWM_TIMER_0,&mcpwm_cfg);
    mcpwm_init(MCPWM_UNIT_0,MCPWM_TIMER_1,&mcpwm_cfg);
    mcpwm_init(MCPWM_UNIT_0,MCPWM_TIMER_2,&mcpwm_cfg);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_B, MCPWM_DUTY_MODE_1);
    //mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM1B, MCPWM_DUTY_MODE_1);
    //mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM2B, MCPWM_DUTY_MODE_1);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_1);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_2);
    int n =0;
    while (n<40)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, 60.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_B, 60.0);
        vTaskDelay(500/portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, 50.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_B, 50.0);
        //vTaskDelay(500/portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_A, 60.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_B, 60.0);
        vTaskDelay(500/portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_A, 50.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_B, 50.0);
        //vTaskDelay(500/portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_A, 60.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_B, 60.0); 
        vTaskDelay(500/portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_A, 50.0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_B, 50.0);
        vTaskDelay(500/portTICK_PERIOD_MS);
        n++;
    }
    while(1)
    {
    int dev_id = rd_pmic((pmic_context_t*)pmic_dev, RD_DEV_ID);  
    int dev_rev = rd_pmic((pmic_context_t*)pmic_dev, RD_DEV_REV);   
    int dev_state = rd_pmic((pmic_context_t*)pmic_dev, RD_DEV_STAT);
    int vmon_state_1 = rd_pmic((pmic_context_t*)pmic_dev, RD_VMON_STAT_1);
    int vmon_state_2 = rd_pmic((pmic_context_t*)pmic_dev, RD_VMON_STAT_2);
    
    int safety_state_1 = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_STAT_1);  
    int safety_state_2 = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_STAT_2);
    int safety_state_3 = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_STAT_3);
    int safety_state_4 = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_STAT_4);
    int safety_state_5 = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_STAT_5);

    int safety_error_state = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_ERR_STAT);
    int watchdog_state = rd_pmic((pmic_context_t*)pmic_dev, RD_WD_STATUS);

    int safety_err_cfg = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_ERR_CFG);
    int safety_pwd_thr_cfg = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_PWD_THR_CFG);
    int safety_func_cfg= rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_FUNC_CFG);
    int safety_cfg_crc = rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_CFG_CRC);

    //cmd0_response = wr_predrv((predrv_context_t*)predrv_dev, CMD0, 0x0800);
    //cmd1_response = wr_predrv((predrv_context_t*)predrv_dev, CMD1, 0x0000);

    int diag1_response = wr_predrv((predrv_context_t*)predrv_dev, 0x06, 0x0800);
    int diag2_response = wr_predrv((predrv_context_t*)predrv_dev, 0x07, 0x0000);
    printf("content in diag1 register is %d;\r\n",diag1_response);
    printf("content in diag2 register is %d;\r\n",diag2_response);


    read_so1();
    read_so2();

    //int predrv_response = wr_predrv((predrv_context_t*)predrv_dev, CMD0, 0x800);
    // printf("Data sent is :%d;\r\n", WR_CMD0);
    // printf("Data in CMD0 Register is: %d;\r\n",predrv_response);

    //printf("Predriver's CMD register is: %d;\r\n", wr_predrv((predrv_context_t*)predrv_dev, WR_CMD0, 0x01FF));

    // printf("Device ID is %d;\r\n",dev_id);
    // printf("Device Revision is %d;\r\n", dev_rev);
    // printf("Device State is %d;\r\n",dev_state);
    // printf("Device Safety State 1: %d;\r\n",safety_state_1);
    // printf("Device Safety State 2: %d;\r\n",safety_state_2);
    // printf("Device Safety State 3: %d;\r\n",safety_state_3);
    // printf("Device Safety State 4: %d;\r\n",safety_state_4);
    // printf("Device Safety State 5: %d;\r\n",safety_state_5);
    // printf("Device Vmon State 1: %d;\r\n",vmon_state_1);
    // printf("Device Vmon State 2: %d;\r\n",vmon_state_2 );
    // printf("Device Safety Error State : %d;\r\n",safety_error_state);
    // printf("Device Watchdog status: %d;\r\n",watchdog_state);
    // printf("Device Safety Error Configuration: %d;\r\n",safety_err_cfg);
    // printf("Device Safety Power down Threshold: %d;\r\n",safety_pwd_thr_cfg);
    // printf("Device Safety Function Configuration: %d;\r\n",safety_func_cfg);
    // printf("Device Safety Configuration CRC: %d;\r\n",safety_cfg_crc);
    // printf("WIN1_CFG: %d;\r\n", rd_pmic((pmic_context_t*)pmic_dev, RD_WD_WIN1_CFG));
    // printf("WIN2_CFG: %d;\r\n", rd_pmic((pmic_context_t*)pmic_dev, RD_WD_WIN2_CFG));
    // printf("Safety Control: %d;\r\n", rd_pmic((pmic_context_t*)pmic_dev, RD_SAFETY_CHECK_CTRL));
    printf("FS FLAG level is: %d;\r\n", gpio_get_level(GPIO_NUM_4));
    //printf("Data in CMD0 Register is: %d;\r\n", cmd0_response);
    //printf("Data in CMD1 Register is: %d;\r\n", cmd1_response);
    //read_endrv();
    vTaskDelay(2000/portTICK_PERIOD_MS);
    }
    //xTaskCreate(print1, "print1", 8192, NULL, 1, NULL);
    //xTaskCreate(print2, "print2", 8192, NULL, 1, NULL);

}
