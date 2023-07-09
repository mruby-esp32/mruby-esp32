#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_littlefs.h"

#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/dump.h"

#define TAG "mruby_task"

typedef mrb_value (*mrb_load_func)(mrb_state*, FILE*, mrbc_context*);

void mruby_task(void *pvParameter)
{
  mrb_state *mrb = mrb_open();
  mrbc_context *context = mrbc_context_new(mrb);
  int ai = mrb_gc_arena_save(mrb);
  ESP_LOGI(TAG, "%s", "Loading...");

  mrb_load_func load = mrb_load_detect_file_cxt;
  FILE *fp = fopen("/storage/main.rb", "r");
  if (fp == NULL) {
    load = mrb_load_irep_file_cxt;
    fp = fopen("/storage/main.mrb", "r");
    if (fp == NULL) {
      ESP_LOGI(TAG, "File is none.");
      goto exit;
    }
  }
  load(mrb, fp, context);
  if (mrb->exc) {
    ESP_LOGE(TAG, "Exception occurred");
    mrb_print_error(mrb);
    mrb->exc = 0;
  } else {
    ESP_LOGI(TAG, "%s", "Success");
  }
  mrb_gc_arena_restore(mrb, ai);
  mrbc_context_free(mrb, context);
  mrb_close(mrb);
  fclose(fp);

  // This task should never end, even if the
  // script ends.
exit:
  while (1) {
    vTaskDelay(1);
  }
}

void app_main()
{
  nvs_flash_init();

  esp_vfs_littlefs_conf_t conf = {
    .base_path = "/storage",
    .partition_label = "storage",
    .format_if_mount_failed = false,
  };
  ESP_ERROR_CHECK(esp_vfs_littlefs_register(&conf));

  xTaskCreate(&mruby_task, "mruby_task", 16384, NULL, 5, NULL);
}
