COMPONENT_OWNBUILDTARGET := build
COMPONENT_OWNCLEANTARGET := clean

COMPONENT_ADD_INCLUDEDIRS := mruby/include/ mruby/build/esp32/include/

CFLAGS += -Wno-char-subscripts -Wno-pointer-sign

.PHONY: build
build: 
	cd $(COMPONENT_PATH)/mruby && MRUBY_CONFIG=../esp32_build_config.rb $(MAKE)
	cp $(COMPONENT_PATH)/mruby/build/esp32/lib/libmruby.a $(COMPONENT_LIBRARY)

.PHONY: clean
clean: 
	cd $(COMPONENT_PATH)/mruby && MRUBY_CONFIG=../esp32_build_config.rb $(MAKE) clean

