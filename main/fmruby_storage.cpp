
#include <stdio.h>
#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"


#include "FS.h"
#include "SPIFFS.h"


FmrbFileService file_service;

#define DEFAULT_TEST_PATH "/spiffs/default.rb"
FmrbFileService::FmrbFileService(){
  m_opened=false;
}

#define FORMAT_SPIFFS_IF_FAILED false

static void suspend_int(){
  fabgl::suspendInterrupts();
  vTaskDelay(50);
}
static void resume_int(){
  fabgl::resumeInterrupts();
  vTaskDelay(50);
}


int FmrbFileService::init(){
  suspend_int();
  bool ret = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED,"/spiffs",1);
  resume_int();
  if(!ret){
    printf("SPIFFS Mount Failed\n");
    return -1;
  }
  printf("SPIFFS Mount OK\n");
  m_opened=true;
  return 0;
}

char* FmrbFileService::load(){
  printf("Reading file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_opened) return NULL;

  suspend_int();
  File file = SPIFFS.open(DEFAULT_TEST_PATH);
  if(!file || file.isDirectory()){
    printf("- failed to open file for reading\n");
    resume_int();
    return NULL;
  }
  printf("- read from file: size=%d\n",(int)file.size());
  char* buff = (char*)fmrb_spi_malloc((int)file.size()+1);
  if(!buff){
    printf("malloc error\n");
    resume_int();
    return NULL;
  }
  resume_int();
  file.read((uint8_t*)buff,(size_t)file.size());
  return buff;
}
int FmrbFileService::save(char* buff){
  printf("Writing file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_opened) return -1;

  suspend_int();
  File file = SPIFFS.open(DEFAULT_TEST_PATH, FILE_WRITE);
  if(!file){
    printf("- failed to open file for writing\n");
    resume_int();
    return -1;
  }
  if(file.print(buff)){
    printf("- file written\n");
  } else {
    printf("- write failed\n");
    resume_int();
    return -1;
  }
  resume_int();
  return 0;
}

