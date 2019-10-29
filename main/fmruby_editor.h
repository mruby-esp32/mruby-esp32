#pragma once

class FmrbEditor {
public:
  FmrbEditor();
  void begin(void);
  void close(void);

private:
  char* m_buff_head;
  int m_height;
  int m_width;
  int m_x;
  int m_y;
  
  void load(const char* buf);
  void move(int x,int y);
};

