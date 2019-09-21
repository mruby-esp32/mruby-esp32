#if 1
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

#include "fmruby_fabgl.h"

#include "entry_mrb.h"

#define TAG "mruby_task"


void*
mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud)
{
  if (size == 0) {
    free(p);
    return NULL;
  }
  else {
    return heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
  }
}


void mruby_task(void *pvParameter)
{
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);
  int ai = mrb_gc_arena_save(mrb);
  ESP_LOGI(TAG, "%s", "Loading binary...");
  mrb_load_irep(mrb, entry_mrb);
  if (mrb->exc) {
    ESP_LOGE(TAG, "Exception occurred: %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    ESP_LOGI(TAG, "%s", "Success");
  }
  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

  ESP_LOGI(TAG, "%s", "End of mruby task");
  while (1) {
	  vTaskDelay(1);
  }
}

void setup(){

}

void loop(){
  nvs_flash_init();
  fabgl_init();
  
  xTaskCreate(&mruby_task, "mruby_task", 8192, NULL, 5, NULL);
  while(1){
	  vTaskDelay(1);
  }
}

#endif