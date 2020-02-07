#pragma once

#include "fabgl.h"
#include "fabutils.h"
#include "fmruby_app.h"

void fabgl_init(void);
void fabgl_terminal_mode_init(FmrbConfig*);
void fabgl_mruby_mode_init(FmrbConfig*);

extern fabgl::VGAController VGAController;
extern fabgl::PS2Controller PS2Controller;
extern fabgl::Canvas        FMRB_canvas;