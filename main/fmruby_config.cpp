#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"


FmrbConfig::FmrbConfig(FmrbFileService *file_service)
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