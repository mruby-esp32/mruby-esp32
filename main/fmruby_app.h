#pragma once

#include "fmruby_editor.h"

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

void menu_app(void);

void mruby_init(void);
void mruby_engine(char* code_string);

