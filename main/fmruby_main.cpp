#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define RUN_MAIN
//#define TEST_AUDIO
//#define TEST_SD

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

#ifdef RUN_MAIN
void setup(){
  nvs_flash_init();
  printf("nvs_flash_init() done\n");
  fabgl_init();
  printf("fabgl_init() done\n");
  vTaskDelay(100);
}

void loop(){
#ifdef TEST_AUDIO
  sound_test();
#endif
#ifdef TEST_SD
  sdcard_test();
#endif
  menu_app();
  while(true){
	  vTaskDelay(1000);
  }
}
#endif