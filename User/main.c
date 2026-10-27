#include "main.h"

int main(void)
{
  systick_init();
  g_us_init();

  ctrl_init();
  led_init();
  btn_init();

  adc_init();
  hall_init();
  bldc_init();
  spd_init();

  while (1)
  {
    g_us_process();
    ctrl_process();
    led_process();
    btn_process();

    adc_process();
    hall_process();
    bldc_process();
    spd_process();
    comm_process();
  }
}


