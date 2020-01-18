#ifndef __FMRUBY_FABGL__
#define __FMRUBY_FABGL__

#include "fabgl.h"

void fabgl_init(void);
void fabgl_terminal_mode_init(void);
void fabgl_mruby_mode_init(void);

extern fabgl::VGAController VGAController;
extern fabgl::PS2Controller PS2Controller;
extern fabgl::Terminal      Terminal;

#endif
