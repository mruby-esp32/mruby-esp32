#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "fmruby_fabgl.h"
#include "fmruby_app.h"

void setup(){

}

void loop(){
  nvs_flash_init();
  printf("nvs_flash_init() done\n");
  fabgl_init();
  printf("fabgl_init() done\n");

  terminal_func();
  //xTaskCreatePinnedToCore(&terminal_task, "terminal_task", 8192, NULL, 1, NULL, 1);  
  //xTaskCreate(&terminal_task, "terminal_task", 8192, NULL, 5, NULL);
  //printf("xTaskCreate(terminal_task) done\n");
 
  //xTaskCreate(&mruby_task, "mruby_task", 8192, NULL, 5, NULL);
  while(1){
	  vTaskDelay(1000);
  }

}
