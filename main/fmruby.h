#pragma once

#include "fabgl.h"

#define FMRB_VERSION "0.5"
#define FMRB_RELEASE "2019/12/31"

void* fmrb_spi_malloc(size_t size);
void* fmrb_spi_realloc(void* ptr, size_t size);
