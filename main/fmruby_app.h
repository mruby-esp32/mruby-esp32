#pragma once

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


void terminal_init(void);
void menu_app(void);

void mruby_init(void);
void mruby_engine(char* code_string);

