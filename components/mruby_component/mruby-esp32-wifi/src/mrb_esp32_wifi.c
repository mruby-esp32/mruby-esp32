#include <mruby.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#include <mruby/string.h>

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

const int CONNECTED_BIT = BIT0;

static void
mrb_eh_ctx_t_free(mrb_state *mrb, void *p) {
  esp_event_loop_set_cb(NULL, NULL);
  mrb_free(mrb, p);
}

static const struct mrb_data_type mrb_eh_ctx_t = {
  "$i_mrb_eh_ctx_t", mrb_eh_ctx_t_free
};

static EventGroupHandle_t wifi_event_group;
typedef struct eh_ctx_t {
  mrb_state *mrb;
  mrb_value on_connected_blk;
  mrb_value on_disconnected_blk;
} eh_ctx_t;

static esp_err_t 
event_handler(void *ctx, system_event_t *event)
{
  eh_ctx_t *ehc = (eh_ctx_t *)ctx;

  switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
      if (ehc != NULL) {
        int arena_index = mrb_gc_arena_save(ehc->mrb);

        mrb_value mrb_ip_str = mrb_str_buf_new(ehc->mrb, 13);
        char ip_str[13];
        sprintf(ip_str, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
        mrb_str_cat_cstr(ehc->mrb, mrb_ip_str, ip_str);

        if (!mrb_nil_p(ehc->on_connected_blk)) {
          mrb_assert(mrb_type(ehc->on_connected_blk) == MRB_TT_PROC);
          mrb_yield_argv(ehc->mrb, ehc->on_connected_blk, 1, &mrb_ip_str);
        }

        mrb_gc_arena_restore(ehc->mrb, arena_index);
      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      // This is a workaround as ESP32 WiFi libs don't currently auto-reassociate. 
      esp_wifi_connect();
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
      if (ehc != NULL) {
        int arena_index = mrb_gc_arena_save(ehc->mrb);

        if (!mrb_nil_p(ehc->on_disconnected_blk)) {
          mrb_assert(mrb_type(ehc->on_disconnected_blk) == MRB_TT_PROC);
          mrb_yield_argv(ehc->mrb, ehc->on_disconnected_blk, 0, NULL);
        }

        mrb_gc_arena_restore(ehc->mrb, arena_index);
      }
      break;
    default:
        break;
  }
  return ESP_OK;
}

static mrb_value
mrb_esp32_wifi_init(mrb_state *mrb, mrb_value self) {
  eh_ctx_t *ehc = mrb_malloc(mrb, sizeof(eh_ctx_t));
  ehc->mrb = mrb;
  ehc->on_connected_blk = mrb_nil_value();
  ehc->on_disconnected_blk = mrb_nil_value();

  mrb_data_init(self, ehc, &mrb_eh_ctx_t);
  
	wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, (eh_ctx_t *) DATA_PTR(self)) );

  return self;
}

static mrb_value
mrb_esp32_wifi_connect(mrb_state *mrb, mrb_value self) {
  char *ssid = NULL;
  char *password = NULL;

  mrb_get_args(mrb, "zz", &ssid, &password);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

	wifi_config_t wifi_config;
  memset((void *)&wifi_config, 0, sizeof(wifi_config_t));
  snprintf(wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
  snprintf(wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);

	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK( esp_wifi_start() );

  return self;
}

static mrb_value
mrb_esp32_wifi_disconnect(mrb_state *mrb, mrb_value self) {
  ESP_ERROR_CHECK( esp_wifi_stop() );
  
  return self;
}

static mrb_value
mrb_esp32_wifi_set_on_connected(mrb_state *mrb, mrb_value self) {
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@on_connected_blk"), block);

  eh_ctx_t *ehc = (eh_ctx_t *) DATA_PTR(self);
  ehc->on_connected_blk = block;
  
  return self;
}

static mrb_value
mrb_esp32_wifi_set_on_disconnected(mrb_state *mrb, mrb_value self) {
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@on_disconnected_blk"), block);

  eh_ctx_t *ehc = (eh_ctx_t *) DATA_PTR(self);
  ehc->on_disconnected_blk = block;
  
  return self;
}

void
mrb_mruby_esp32_wifi_gem_init(mrb_state* mrb) {
  tcpip_adapter_init();

  struct RClass *esp32_module = mrb_define_module(mrb, "ESP32");
  struct RClass *esp32_wifi_class = mrb_define_class_under(mrb, esp32_module, "WiFi", mrb->object_class);

  mrb_define_method(mrb, esp32_wifi_class, "initialize", mrb_esp32_wifi_init, MRB_ARGS_NONE());

  mrb_define_method(mrb, esp32_wifi_class, "connect", mrb_esp32_wifi_connect, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, esp32_wifi_class, "disconnect", mrb_esp32_wifi_disconnect, MRB_ARGS_NONE());

  mrb_define_method(mrb, esp32_wifi_class, "on_connected", mrb_esp32_wifi_set_on_connected, MRB_ARGS_BLOCK());
  mrb_define_method(mrb, esp32_wifi_class, "on_disconnected", mrb_esp32_wifi_set_on_disconnected, MRB_ARGS_BLOCK());
}

void
mrb_mruby_esp32_wifi_gem_final(mrb_state* mrb) {
}
