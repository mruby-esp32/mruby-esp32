#pragma once

#include "fmruby_editor.h"

class FmrbFileService {
public:
  FmrbFileService();
  int init();
  char* load();
  int save(char* buff);
private:
  bool m_opened;
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

