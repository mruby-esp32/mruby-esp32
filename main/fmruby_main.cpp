#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"

void* fmrb_spi_malloc(size_t size)
{
  return heap_caps_malloc(size,MALLOC_CAP_SPIRAM);
}

void* fmrb_spi_realloc(void* ptr, size_t size)
{
  return heap_caps_realloc(ptr,size,MALLOC_CAP_SPIRAM);
}


void setup(){
  nvs_flash_init();
  printf("nvs_flash_init() done\n");
  fabgl_init();
  printf("fabgl_init() done\n");
}

void loop(){
  menu_app();
  while(true){
	  vTaskDelay(1000);
  }
}
