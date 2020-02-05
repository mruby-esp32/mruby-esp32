#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"


FmrbConfig::FmrbConfig(FmrbFileService *file_service):
main_mode_line({0}),
mruby_mode_line({0}),
main_screen_shift_x(0),
main_screen_shift_y(0),
mruby_screen_shift_x(0),
mruby_screen_shift_y(0),
m_fservice(file_service)
{
}

FMRB_RCODE FmrbConfig::load()
{
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbConfig::save()
{
  return FMRB_RCODE::OK;
  
}