#include "driver/mcpwm.h"
#include "driver/gpio.h"

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
//Configure MCPWM
//mcpwm_set_pin(MCPWM_UNIT_0,&mcpwm_pin_cfg);
mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0A,GPIO_NUM_25);
mcpwm_init(MCPWM_UNIT_0,MCPWM_TIMER_0,&mcpwm_cfg);
//Operate
//mcpwm_set_signal_high(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM0A);
//mcpwm_set_signal_low(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM0B);
mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
//mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, 50.0);
//mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A);
//mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, MCPWM_DUTY_MODE_1);
//mcpwm_set_frequency(MCPWM_UNIT_0,MCPWM_TIMER_0,32000);
//mcpwm_deadtime_enable(MCPWM_UNIT_0,MCPWM_TIMER_0,);