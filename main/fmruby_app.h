/*
 Created by Katsuhiko KAGEYAMA(@kishima) - <https://silentworlds.info>
 Copyright (c) 2019-2020 Katsuhiko KAGEYAMA.
 All rights reserved.

 This file is part of Family mruby.

 Family mruby is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Family mruby is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Family mruby.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "fmruby.h"
#include "fmruby_editor.h"
#include "mruby.h"
#include <cstdlib>

/**
 * File service
 **/
enum class FmrbStorageType{
  NONE,
  SPIFFS,
  SD,
};

#define FMRB_MAX_PATH_LEN (256)
class FmrbFileService {
public:
  FmrbFileService();
  FMRB_RCODE init();
  FMRB_RCODE mount_sd();
  void umount_sd();

  char* load(const char* path,uint32_t &fsize,bool is_text=false,bool localmem=true);
  char* load_bitmap(const char* path,uint16_t &width,uint16_t &height,uint32_t &type);
  FMRB_RCODE save(char* buff,const char* path);

private:
  bool m_spiffs_opened;
  bool m_sd_opened;

  uint64_t m_sd_size; 

  void init_sd_spi();
  FMRB_RCODE mount_spiffs();
  FmrbStorageType check_stype_path(const char*);
  const char* to_data_path(const char* path);

};

/**
 * Config
 **/
#define FMRB_MODE_LINE_MAX (256)
class FmrbConfig  {
public:
  char main_mode_line[FMRB_MODE_LINE_MAX];
  char mruby_mode_line[FMRB_MODE_LINE_MAX];
  int main_screen_shift_x;
  int main_screen_shift_y;
  int mruby_screen_shift_x;
  int mruby_screen_shift_y;

  //Code Highlight

  OVERLOAD_SPI_ALLOCATOR
  FmrbConfig();
  FMRB_RCODE load(FmrbFileService *fs);
  FMRB_RCODE save(FmrbFileService *fs);

private:
};

/**
 * Dialog 
 **/
class FmrbDialog : public FmrbTerminalInput{
public:
  OVERLOAD_SPI_ALLOCATOR
  FmrbDialog(fabgl::VGAController *v,fabgl::Canvas* canvas,fabgl::Terminal*,FmrbCanvasConfig*);
  ~FmrbDialog();
  void open_message_dialog(const char* message,int timeout_sec);
  bool open_confirmation_dialog(const char* message);
  const char* open_text_select_dialog(const char* list[],int list_length);
  const char* open_file_select_dialog(const char* path);
  const char* open_text_input_dialog(const char* list[],int list_length);

private:
  fabgl::VGAController* m_vga;
  fabgl::Canvas *m_canvas;
  fabgl::Terminal *m_terminal;
  uint8_t *m_swap_buff;
  uint16_t m_screen_width;
  uint16_t m_screen_height;
  uint16_t m_dialog_width;
  uint16_t m_dialog_height;
  uint16_t m_x;
  uint16_t m_y;
  uint8_t m_line_height;
  uint8_t m_font_width;
  uint8_t m_font_height;
  RGB888 m_fg_color;
  RGB888 m_bg_color1;
  RGB888 m_bg_color2;
  FmrbCanvasConfig *m_canva_config_origin;

  int draw_window(int line);
};


/**
 * Menu 
 **/

class FmrbMenuModule;
typedef FMRB_RCODE (*FmrbMenuCallback)(uint32_t fid,FmrbMenuModule* menu);  

enum class FmrbMenuItemType{
  NONE,
  TITLE,
  SELECTABLE,
  UNSELECTABLE,
};

class FmrbMenuItem{
public:
  char* description;
  uint32_t fid;
  FmrbMenuCallback func;
  FmrbMenuItemType type;

  FmrbMenuItem* m_prev;
  FmrbMenuItem* m_next;
  FmrbMenuItem* m_parent;
  FmrbMenuItem* m_child;

//func
  OVERLOAD_SPI_ALLOCATOR
  FmrbMenuItem();
  FmrbMenuItem(char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  ~FmrbMenuItem();

  static FmrbMenuItem* add_item_tail(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  static FmrbMenuItem* add_child_item(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type);
  static FmrbMenuItem* retrieve_item(FmrbMenuItem* head_item,int line);
};

class FmrbMenuModule : public FmrbTerminalInput {
public:
  fabgl::VGAController *m_vga;
  fabgl::Canvas *m_canvas;
  fabgl::Terminal* m_terminal;
  FmrbCanvasConfig *m_canvas_config;
  void *m_param;

  OVERLOAD_SPI_ALLOCATOR
  FmrbMenuModule(fabgl::VGAController*, fabgl::Canvas* canvas,fabgl::Terminal* terminal,FmrbMenuItem *item);
  ~FmrbMenuModule();
  void begin(void *param_p);
  void set_storage(FmrbFileService *storage);

private:
  FmrbMenuItem* m_top;
  int m_offset_x;
  int m_offset_y;
  int m_mergin;
  FmrbFileService *m_storage;

  void draw_item(FmrbMenuItem* head_item,int line,bool invert);
  int draw_menu(FmrbMenuItem* head_item);
  void clear_draw_area(void);
  void exec_menu(FmrbMenuItem* head_item);
};

/**
 * mruby engine
 **/
#define FMRB_JOYPAD_MAP_LENGTH (13)
enum class FMRB_JPAD_KEY{
  NONE=0,A,B,X,Y,L,R,START,SELECT,
  UP,DOWN,LEFT,RIGHT,
};

#define FMRB_DBG_MSG_MAX_LEN (1024)
class FmrbMrubyEngine {
public:
  FmrbMrubyEngine();
  ~FmrbMrubyEngine();
  void run(char* code_string);
  uint8_t *get_joypad_map();
  const char *get_error_msg();
  int get_error_line();
  FMRB_RCODE get_result();

private:
  FMRB_RCODE m_exec_result;
  char* m_error_msg;
  int m_error_line;
  uint8_t *m_joypad_map;
 
  static void* mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud);
  void check_backtrace(mrb_state *mrb,mrb_value v);
  void prepare_env();
  void cleanup_env();
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
  fabgl::VGAController *m_vga;
  fabgl::PS2Controller *m_ps2;
  fabgl::Canvas *m_canvas;
  FmrbConfig *m_config;

  FmrbSystemApp(fabgl::VGAController*,fabgl::PS2Controller*,fabgl::Canvas*);
  void init(FmrbFileService* st);
  FMRB_RCODE run();
  FmrbMrubyEngine *mruby_engign();

  static void draw_img(fabgl::VGAController *vga,uint16_t x0,uint16_t y0,uint8_t* data,int mode);

private:
  char* m_script;
  FMRB_SYS_STATE m_state;
  FmrbEditor *m_editor;
  bool m_terminal_available;
  fabgl::Terminal m_terminal;
  FmrbMenuModule *m_main_menu;
  FmrbMrubyEngine m_mruby_engine;
  FmrbFileService *m_storage;

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

char* alloc_menu_text_mem(const char* input);
FmrbConfig* get_system_config(void);

FMRB_RCODE fmrb_subapp_resolution_test(FmrbMenuModule* menu);
FMRB_RCODE fmrb_subapp_select_main_resolution(FmrbMenuModule* menu);
FMRB_RCODE fmrb_subapp_select_mruby_resolution(FmrbMenuModule* menu);

