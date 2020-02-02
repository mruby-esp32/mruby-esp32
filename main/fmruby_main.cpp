#include <stdio.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define RUN_MAIN
//#define TEST_AUDIO
//#define TEST_SD
//#define TEST_SPIFFS

#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"


void* fmrb_spi_malloc(size_t size)
{
  return heap_caps_malloc(size,MALLOC_CAP_SPIRAM);
}

void* fmrb_spi_realloc(void* ptr, size_t size)
{
  return heap_caps_realloc(ptr,size,MALLOC_CAP_SPIRAM);
}

void fmrb_free(void* ptr){
  free(ptr);
}

void fmrb_dump_mem_stat(){

  FMRB_DEBUG(FMRB_LOG::INFO,"-- mem dump ------------------------\n");
  FMRB_DEBUG(FMRB_LOG::INFO,"| Free size(DMA)     = %d\n",heap_caps_get_free_size(MALLOC_CAP_DMA));
  FMRB_DEBUG(FMRB_LOG::INFO,"| Free size(32bit)   = %d\n",heap_caps_get_free_size(MALLOC_CAP_32BIT));
  FMRB_DEBUG(FMRB_LOG::INFO,"| Max free size(DMA) = %d\n",heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
  //heap_caps_print_heap_info(MALLOC_CAP_DMA);
}

void fmrb_debug_print(FMRB_LOG lv,const char *fmt,const char* func,int line, ...)
{
  va_list arg;
  switch(lv){
    case FMRB_LOG::DEBUG:
    printf("[DBG]");break;
    case FMRB_LOG::MSG:
    printf("[MSG]");break;
    case FMRB_LOG::INFO:
    printf("[INF]");break;
    case FMRB_LOG::ERR:
    printf("[ERR]");break;
    case FMRB_LOG::RAW:
    break;
  }

  if(lv==FMRB_LOG::ERR){
    printf("[%s:%d]",func,line);
  } 

  va_start(arg, fmt);
  vprintf(fmt, arg);
  va_end(arg);
}

#ifdef TEST_AUDIO
SoundGenerator soundGenerator;
SquareWaveformGenerator square;

void sound_test()
{
  WaveformGenerator * curGen = nullptr;
  //SamplesGenerator mario = SamplesGenerator(marioSamples, sizeof(marioSamples));
  soundGenerator.play(true);
  soundGenerator.setVolume(100);
  curGen = &square;
  //curGen = &mario;
  soundGenerator.attach(curGen);
  curGen->enable(true);
  curGen->setFrequency(500);
  //while(true){
  vTaskDelay(5000);
  //}
  soundGenerator.detach(curGen);
  //curGen->enable(false);
  printf("sound_test done\n");
}
#endif

#ifdef TEST_SD
#include "SD.h"
#include "SPI.h"
SPIClass hspi(HSPI);

void setup_sd(){
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  //hspi = new SPIClass(HSPI);
  
  //initialise hspi with default pins
  //SCLK = 14, MISO = 12, MOSI = 13, SS = 15
  gpio_pullup_en(GPIO_NUM_12);
  hspi.begin(14,12,13,15); 
  pinMode(15, OUTPUT); //HSPI SS

  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
}

void readFile(fs::FS &fs, const char * path){
    printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        printf("Failed to open file for reading\n");
        return;
    }

    printf("Read from file: ");
    while(file.available()){
        printf("%c",(char)file.read());
    }
    printf("\n");
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        printf("Failed to open file for writing\n");
        return;
    }
    if(file.print(message)){
        printf("File written\n");
    } else {
        printf("Write failed\n");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        printf("Failed to open file for appending\n");
        return;
    }
    if(file.print(message)){
        printf("Message appended\n");
    } else {
        printf("Append failed\n");
    }
    file.close();
}

void sdcard_test()
{
  //uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5
  setup_sd();
  vTaskDelay(500);

  if(!SD.begin(15,hspi,4000000,"/sd",5)){
      printf("Card Mount Failed\n");
      return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      printf("No SD card attached\n");
      return;
  }

  printf("SD Card Type: ");
  if(cardType == CARD_MMC){
      printf("MMC\n");
  } else if(cardType == CARD_SD){
      printf("SDSC\n");
  } else if(cardType == CARD_SDHC){
      printf("SDHC\n");
  } else {
      printf("UNKNOWN\n");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  printf("SD Card Size: %lluMB\n", cardSize);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!!\n");
  readFile(SD, "/hello.txt");
}
#endif

#ifdef TEST_SPIFFS
static void test_spiffs(){
  printf("File service init\n");
  file_service.init();
}
#endif

#ifdef RUN_MAIN
void setup(){
#ifndef TEST_SD
  //SPI SD pins
  //pinMode(12, OUTPUT);
  //pinMode(13, OUTPUT);
  //pinMode(14, OUTPUT);
  pinMode(15, OUTPUT); //SS
  //digitalWrite(15,HIGH);//HIGH => NOT selected
  gpio_pullup_en(GPIO_NUM_12); //=> 12 must be low during BOOT
  //hspi.begin(14,12,13,15); 
#endif

  nvs_flash_init();
  FMRB_DEBUG(FMRB_LOG::INFO,"nvs_flash_init() done\n");
#ifdef TEST_SPIFFS
  test_spiffs();
#endif
  FMRB_DEBUG(FMRB_LOG::DEBUG,"fabgl_init()\n");
  fabgl_init();
  FMRB_DEBUG(FMRB_LOG::INFO,"fabgl_init() done\n");
  vTaskDelay(100);
}

#endif

TaskHandle_t mainTaskHandle = NULL;

FmrbSystemApp SystemApp;

void mainTask(void *pvParameters)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"=======================================\n");
  FMRB_DEBUG(FMRB_LOG::INFO," Family mruby ver:%s (%s)\n",FMRB_VERSION,FMRB_RELEASE);
  FMRB_DEBUG(FMRB_LOG::INFO,"=======================================\n");
  fmrb_dump_mem_stat();
  setup();
  fmrb_dump_mem_stat();

#ifdef TEST_AUDIO
  sound_test();
#endif
#ifdef TEST_SD
  sdcard_test();
#endif

#if 0
  char* buff = (char*)heap_caps_malloc(4,MALLOC_CAP_DMA);
  printf("MALLOC_CAP_DMA:%p\n",buff);
  free(buff);
  buff = (char*)fmrb_spi_malloc(4);
  printf("MALLOC_CAP_SPIRAM:%p\n",buff);
  free(buff);
#endif
  SystemApp.run();

  //Never comes here
  while(true){
	  vTaskDelay(1000);
  }
}

extern "C" void app_main()
{
  //Follow Arduino manner
  initArduino();
  xTaskCreateUniversal(mainTask, "mainTask", FMRB_MAIN_TASK_STACK_SIZE, NULL, 1, &mainTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
}

