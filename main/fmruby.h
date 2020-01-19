#pragma once

#include "fabgl.h"

#define FMRB_VERSION "0.6"
#define FMRB_RELEASE "2020/02/15"

void* fmrb_spi_malloc(size_t size);
void* fmrb_spi_realloc(void* ptr, size_t size);
