#pragma once

#include "fmruby.h"
#include <stdint.h>

#define LEXER_WORDS_MAX (200)
class FmrbSimpleLineLexer{
public:
    void init();
    void free();
    void set_line(const char* line);
    void move_head();
    void itr_dump(const char** buff,int* len);
private:
    const char* m_string;
    const char* m_head;
    int m_total_words;
    int m_word_pos;
    const char* m_word_p[LEXER_WORDS_MAX];
    unsigned char m_word_len[LEXER_WORDS_MAX];
    int m_dump_stat;

    bool is_delimiter(const char);
    void push_word(const char*,int);
    void push_word_prim(const char*,int);
    const char* last_word_tail();
    int is_highlight(const char* ,int);
};

#define EDITLINE_BLOCK_SIZE (16)

struct EditLine {
  char* text;
  uint16_t length;
  uint16_t flag;
  uint16_t lineno;
  uint16_t buff_size;
  EditLine* prev;
  EditLine* next;

  OVERLOAD_SPI_ALLOCATOR
  EditLine(void);
  EditLine(char*);
  ~EditLine();
  int insert(char c);  
  int insert(uint16_t pos,char c);
  int backdelete(uint16_t pos);
  char* cut(uint16_t start_pos, uint16_t end_pos);
  void clear(void);

private:

};

enum EDIT_STATUS{
  EDIT_NO_ERROR=0,
  EDIT_MEM_ERROR
};

class FmrbMrubyEngine;

class FmrbEditor : public FmrbTerminalInput {
public:
  OVERLOAD_SPI_ALLOCATOR
  FmrbEditor(fabgl::VGAController* vga,fabgl::Canvas* canvas,fabgl::Terminal* terminal);
  ~FmrbEditor();
  FMRB_RCODE begin(FmrbMrubyEngine*);
  FMRB_RCODE run(char* input_script);
  FMRB_RCODE release(void);
  char* dump_script(void);

private:
  fabgl::VGAController *m_vga;
  fabgl::Canvas* m_canvas;
  fabgl::Terminal* m_term;
  EditLine* m_buff_head;
  int m_height;
  int m_disp_height;
  int m_width;
  int m_disp_width;
  int m_lineno_shift;
  int m_x;
  int m_y;
  int m_disp_head_line;
  int m_total_line;
  FmrbSimpleLineLexer* m_line_lexer_p;
  enum EDIT_STATUS m_error;
  FmrbMrubyEngine* m_mruby_engine;

  
  void wait_key(char);
  void load_file();
  void save_file();
  void load_demo_file(int);
  void clear_buffer();

  void load(const char* buf);
  EditLine* load_line(const char* in);
  EditLine* seek_line(int n);

  void update(void);
  void update_lineno(void);
  void draw_line(int disp_y,EditLine* line);

  void move_edit_cursor(int dir);
  void move_cursor(int x,int y);
  void move(int x,int y);

  void insert_ch(char c);
  void insert_ret();
  void delete_ch();
  void page_up();
  void page_down();
  void delete_line();

  void print_csr_info(void);
};

