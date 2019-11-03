#pragma once

#include <stdint.h>

#define EDITLINE_BLOCK_SIZE (16)

struct EditLine {
  char* text;
  uint16_t length;
  uint16_t flag;
  uint16_t lineno;
  uint16_t buff_size;
  EditLine* prev;
  EditLine* next;

  static EditLine* create_line(void);
  static EditLine* create_line(char*);
  int init(char* input);
  int insert(char c);  
  int insert(uint16_t pos,char c);
  int backdelete(uint16_t pos);
  char* cut(uint16_t start_pos, uint16_t end_pos);
private:

};

enum EDIT_STATUS{
  EDIT_NO_ERROR=0,
  EDIT_MEM_ERROR
};

class FmrbEditor {
public:
  FmrbEditor();
  int run(void);
  char* dump_script(void);

private:
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
  enum EDIT_STATUS m_error;
  
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

  void finalize(void);
  void print_csr_info(void);
};

