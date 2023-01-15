idf_component_register(
  SRCS mruby_main.c
  INCLUDE_DIRS .
  REQUIRES mruby_component
  PRIV_REQUIRES nvs_flash spiffs
)

spiffs_create_partition_image(storage ./spiffs FLASH_IN_PROJECT)
