#include <mruby.h>
#include <mruby/value.h>

#include <stdio.h>

#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static mrb_value
mrb_esp32_system_delay(mrb_state *mrb, mrb_value self) {
  mrb_int delay;
  mrb_get_args(mrb, "i", &delay);

  vTaskDelay(delay / portTICK_PERIOD_MS);

  return self;
}

static mrb_value
mrb_esp32_system_available_memory(mrb_state *mrb, mrb_value self) {
  return mrb_fixnum_value(system_get_free_heap_size());
}

static mrb_value
mrb_esp32_system_sdk_version(mrb_state *mrb, mrb_value self) {
  return mrb_str_new_cstr(mrb, system_get_sdk_version());
}

static mrb_value
mrb_esp32_system_restart(mrb_state *mrb, mrb_value self) {
  system_restart();
  return self;
}

static mrb_value
mrb_esp32_system_restore(mrb_state *mrb, mrb_value self) {
  system_restore();
  return self;
}

static mrb_value
mrb_esp32_system_time(mrb_state *mrb, mrb_value self) {
  return mrb_fixnum_value(system_get_time());
}

static mrb_value
mrb_esp32_system_rtc_time(mrb_state *mrb, mrb_value self) {
  return mrb_fixnum_value(system_get_rtc_time());
}

static mrb_value
mrb_esp32_system_mac(mrb_state *mrb, mrb_value self) {
  uint8_t mac[6];
  ESP_ERROR_CHECK( system_efuse_read_mac(mac) );
  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return mrb_str_new_cstr(mrb, mac_str);
}

static mrb_value
mrb_esp32_system_deep_sleep_for(mrb_state *mrb, mrb_value self) {
  mrb_int sleep_time;
  mrb_get_args(mrb, "i", &sleep_time);

  system_deep_sleep(sleep_time);

  return self;
}

void
mrb_mruby_esp32_system_gem_init(mrb_state* mrb) {
  struct RClass *esp32_module = mrb_define_module(mrb, "ESP32");

  struct RClass *esp32_system_module = mrb_define_module_under(mrb, esp32_module, "System");

  mrb_define_module_function(mrb, esp32_system_module, "delay", mrb_esp32_system_delay, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, esp32_system_module, "available_memory", mrb_esp32_system_available_memory, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "sdk_version", mrb_esp32_system_sdk_version, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "restart", mrb_esp32_system_restart, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "restore", mrb_esp32_system_restore, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "time", mrb_esp32_system_time, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "rtc_time", mrb_esp32_system_rtc_time, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "mac", mrb_esp32_system_mac, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, esp32_system_module, "deep_sleep_for", mrb_esp32_system_deep_sleep_for, MRB_ARGS_REQ(1));
}

void
mrb_mruby_esp32_system_gem_final(mrb_state* mrb) {
}
