
COMPONENT_OWNBUILDTARGET := true
COMPONENT_OWNCLEANTARGET := true

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/FabGL_component/src $(PROJECT_PATH)/main

COMPONENT_ADD_INCLUDEDIRS := mruby/include/

CFLAGS += -Wno-char-subscripts -Wno-pointer-sign

build: 
	cd $(COMPONENT_PATH)/mruby && COMPONENT_EXTRA_INCLUDES="$(COMPONENT_EXTRA_INCLUDES)" MRUBY_CONFIG=../esp32_build_config.rb $(MAKE)
	cp $(COMPONENT_PATH)/mruby/build/esp32/lib/libmruby.a $(COMPONENT_LIBRARY)

clean: 
	cd $(COMPONENT_PATH)/mruby && MRUBY_CONFIG=../esp32_build_config.rb $(MAKE) clean

