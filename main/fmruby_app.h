#pragma once

#include "fmruby_editor.h"
#include "mruby.h"

class FmrbFileService {
public:
  FmrbFileService();
  int init();
  char* load(const char* path,uint32_t &fsize,bool is_text=false,bool localmem=true);
  char* load_bitmap(const char* path,uint16_t &width,uint16_t &height,uint32_t &type);
  int save(char* buff,const char* path);
private:
  bool m_spiffs_opened;
  bool m_sd_opened;
};

extern FmrbFileService file_service;

class FmrbSystemApp {
public:
  FmrbSystemApp();
  int run();
private:
  char* m_script;
  FmrbEditor m_editor;
  fabgl::Terminal m_terminal;
  int init_terminal();
  int print_system_info();
  int show_splash();
  int run_editor();
  int run_mruby();

};

class FmrbMrubyEngine {
public:
  FmrbMrubyEngine();
  void run(char* code_string);
private:
  static constexpr int DBG_MSG_MAX_LEN = 128;
  int m_exec_result;
  char* m_error_msg;
  int m_error_line;
  static void* mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud);
  void check_backtrace(mrb_state *mrb);
};

void menu_app(void);
