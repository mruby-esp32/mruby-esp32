#pragma once

#include "fabgl.h"

void fabgl_init(void);
void fabgl_terminal_mode_init(void);
void fabgl_mruby_mode_init(void);

extern fabgl::VGAController VGAController;
extern fabgl::PS2Controller PS2Controller;
extern fabgl::Canvas        FMRB_canvas;