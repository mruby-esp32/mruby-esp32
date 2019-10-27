#pragma once

class FmrbEditor {
  public:
  FmrbEditor();
  void begin(void);
  void close(void);

  private:
  char* m_buff_head;
  
  void load(const char* buf);
};
