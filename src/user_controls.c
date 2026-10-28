#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#define REFRESH 15
#define TEMP 26
#define FAN_H 20
#define FAN_L 21
#define BACKLIGHT 0
static uint8_t brightness=220, refresh=0, fan_mode=0, fan_on=0;
static uint16_t raw_adc=0, stream_interval=0;
static uint32_t last_sensor=0;
static bool pwm_ready=false;
void user_controls_init_before_power_on(void) {
  gpio_init(REFRESH); gpio_set_dir(REFRESH,GPIO_OUT); gpio_put(REFRESH,refresh);
  gpio_init(FAN_H); gpio_set_dir(FAN_H,GPIO_OUT); gpio_put(FAN_H,0);
  gpio_init(FAN_L); gpio_set_dir(FAN_L,GPIO_OUT); gpio_put(FAN_L,0);
  adc_init(); adc_gpio_init(TEMP); adc_select_input(0);
}
void user_controls_run_after_power_on(void) {
  gpio_set_function(BACKLIGHT,GPIO_FUNC_PWM);
  uint slice=pwm_gpio_to_slice_num(BACKLIGHT);
  pwm_set_clkdiv(slice,1.0f); pwm_set_wrap(slice,255);
  pwm_set_chan_level(slice,pwm_gpio_to_channel(BACKLIGHT),brightness);
  pwm_set_enabled(slice,true); pwm_ready=true;
}
void uxe_poll(void) {
  uint32_t now=to_ms_since_boot(get_absolute_time());
  if ((uint32_t)(now-last_sensor)<1000) return;
  last_sensor=now;
  raw_adc=adc_read();
  uint32_t mv=(uint32_t)raw_adc*3300u/4095u;
  if (fan_mode==1) fan_on=0;
  else if (fan_mode==2) fan_on=1;
  else { if (!fan_on && mv>2100) fan_on=1; else if (fan_on && mv<1950) fan_on=0; }
  gpio_put(FAN_H,fan_on); gpio_put(FAN_L,0);
  printf("Thermistor: %lu mV, fan=%u\n",(unsigned long)mv,fan_on);
}
uint16_t uxe_get_adc(void){return raw_adc;}
uint16_t uxe_get_mv(void){return (uint32_t)raw_adc*3300u/4095u;}
uint8_t uxe_get_brightness(void){return brightness;}
uint8_t uxe_get_refresh(void){return refresh;}
uint8_t uxe_get_fan_mode(void){return fan_mode;}
uint8_t uxe_get_fan_on(void){return fan_on;}
uint16_t uxe_get_stream_interval(void){return stream_interval;}
void uxe_set_brightness(uint8_t v){brightness=v;if(pwm_ready)pwm_set_gpio_level(BACKLIGHT,v);}
void uxe_set_refresh(uint8_t v){refresh=v;gpio_put(REFRESH,v);}
void uxe_set_fan_mode(uint8_t v){fan_mode=v;if(v==1)fan_on=0;else if(v==2)fan_on=1;gpio_put(FAN_H,fan_on);gpio_put(FAN_L,0);}
void uxe_set_stream_interval(uint16_t v){stream_interval=v;}
