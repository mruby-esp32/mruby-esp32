#pragma once

struct EditLine{
  char* text;
  int length;
  int flag;
  EditLine* prev;
  EditLine* next;
};

enum EDIT_STATUS{
  EDIT_NO_ERROR=0,
  EDIT_MEM_ERROR
};

class FmrbEditor {
public:
  FmrbEditor();
  int run(void);

private:
  EditLine* m_buff_head;
  int m_height;
  int m_width;
  int m_x;
  int m_y;
  int m_disp_head_line;
  enum EDIT_STATUS m_error;
  
  void update(void);
  void load(const char* buf);
  EditLine* load_line(const char* in);
  EditLine* seek_line(int n);
  void move(int x,int y);
  void finalize(void);
};

