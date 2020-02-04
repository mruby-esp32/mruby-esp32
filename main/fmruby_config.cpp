#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"



void* FmrbConfig::operator new(std::size_t size)
{
  printf("@SPI ALLOC\n");
  void* ptr = fmrb_spi_malloc(size);
    if(ptr == nullptr) {
        throw std::bad_alloc();
    }
    return ptr;
}


void FmrbConfig::operator delete(void* ptr)
{
  fmrb_free(ptr);
}

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