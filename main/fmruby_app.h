#pragma once

#include "fmruby_editor.h"
#include "mruby.h"

/**
 * File service
 **/

class FmrbFileService {
public:
  FmrbFileService();
  FMRB_RCODE init();
  char* load(const char* path,uint32_t &fsize,bool is_text=false,bool localmem=true);
  char* load_bitmap(const char* path,uint16_t &width,uint16_t &height,uint32_t &type);
  FMRB_RCODE save(char* buff,const char* path);
private:
  bool m_spiffs_opened;
  bool m_sd_opened;
};

/**
 * Menu 
 **/

class FmrbMenuModule;
typedef FMRB_RCODE (*FmrbMenuCallback)(uint32_t fid,FmrbMenuModule* menu);  

enum class FmrbMenuItemType{
  TITLE,
  SELECTABLE,
  UNSELECTABLE,
};
struct FmrbMenuItem{
  char* description;
  uint32_t fid;
  FmrbMenuCallback func;
  FmrbMenuItemType type;

  FmrbMenuItem* m_prev;
  FmrbMenuItem* m_next;
  FmrbMenuItem* m_parent;
  FmrbMenuItem* m_child;

  static FmrbMenuItem* create_item(void);
  static FmrbMenuItem* create_item(char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  static FmrbMenuItem* add_item_tail(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  static FmrbMenuItem* add_child_item(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  static FmrbMenuItem* retrieve_item(FmrbMenuItem* head_item,int line);
  static void free(FmrbMenuItem*);
};

class FmrbMenuModule {
public:
  fabgl::Canvas* m_canvas;
  fabgl::Terminal* m_terminal;

  FmrbMenuModule();
  void init(fabgl::Canvas* canvas,fabgl::Terminal* terminal,FmrbMenuItem *item);
  void begin();
  void clear();

private:
  FmrbMenuItem* m_top;

  int m_offset_x;
  int m_offset_y;
  int m_mergin;

  void draw_item(FmrbMenuItem* head_item,int line,bool invert);
  int draw_menu(FmrbMenuItem* head_item);
  void clear_draw_area(void);
  void exec_menu(FmrbMenuItem* head_item);
};

/**
 * mruby engine
 **/

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

/**
 * System app
 **/

enum class FMRB_SYS_STATE{
  INIT=0,
  SHOW_MENU,
  DO_EDIT,
  EXEC_FROM_EDIT,
  EXEC_FROM_FILE,
};

class FmrbSystemApp {
public:
  FmrbSystemApp();
  FMRB_RCODE run();

private:
  char* m_script;
  FMRB_SYS_STATE m_state;
  FmrbEditor m_editor;
  bool m_terminal_available;
  fabgl::Terminal m_terminal;
  FmrbMenuModule m_main_menu;
  FmrbMrubyEngine m_mruby_engine;

  void wait_key(char target,int timeout);
  FMRB_RCODE init_terminal();
  FMRB_RCODE close_terminal();
  FMRB_RCODE print_system_info();
  FMRB_RCODE show_splash();
  FMRB_RCODE clear_splash();
  FmrbMenuItem* prepare_top_menu();
  FMRB_RCODE run_main_menu();
  FMRB_RCODE run_editor();
  FMRB_RCODE run_mruby();

};

extern FmrbFileService FMRB_storage;

FMRB_RCODE fmrb_subapp_resolution_test(FmrbMenuModule* menu);

