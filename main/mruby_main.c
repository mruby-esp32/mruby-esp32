#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "mruby.h"
#include "mruby/irep.h"

#include "example_mrb.h"

void mruby_task(void *pvParameter)
{
  mrb_state *mrb = mrb_open();
  mrb_load_irep(mrb, example_mrb);
  mrb_close(mrb);

  // This task should never end, even if the
  // script ends.
  while (1) {
  }
}

void app_main()
{
  nvs_flash_init();
  system_init();
  xTaskCreate(&mruby_task, "mruby_task", 8192, NULL, 5, NULL);
}
