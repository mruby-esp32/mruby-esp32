COMPONENT_DEPENDS := mruby_component FabGL_component arduino

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/FabGL_component/src

CFLAGS += -D LOG_LOCAL_LEVEL=ESP_LOG_DEBUG
CPPFLAGS += -fpermissive

