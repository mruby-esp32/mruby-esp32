#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"

#include "entry_mrb.h"

#define TAG "mruby_task"

void mruby_task(void *pvParameter)
{
  mrb_state *mrb = mrb_open();
  mrbc_context *context = mrbc_context_new(mrb);
  int ai = mrb_gc_arena_save(mrb);
  ESP_LOGI(TAG, "%s", "Loading binary...");
  mrb_load_irep_cxt(mrb, entry_mrb, context);
  if (mrb->exc) {
    ESP_LOGE(TAG, "Exception occurred: %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    ESP_LOGI(TAG, "%s", "Success");
  }
  mrb_gc_arena_restore(mrb, ai);
  mrbc_context_free(mrb, context);
  mrb_close(mrb);

  while (1) {
	  vTaskDelay(1);
  }
}

void setup(){

}

void loop(){
  nvs_flash_init();
  xTaskCreate(&mruby_task, "mruby_task", 8192, NULL, 5, NULL);
  while(1){
	  vTaskDelay(1);
  }
}