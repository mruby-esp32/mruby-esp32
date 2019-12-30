#ifndef __FMRUBY_APP__
#define __FMRUBY_APP__

#include "fabgl.h"
extern TerminalClass Terminal;

void terminal_init(void);
void menu_app(void);

void mruby_init(void);
void mruby_engine(char* code_string);

#endif
