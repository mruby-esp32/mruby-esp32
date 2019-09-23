COMPONENT_DEPENDS := mruby_component FabGL_component arduino

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/FabGL_component/src

COMPONENT_EXTRA_CLEAN := entry_mrb.h

MRUBY_ENTRY := entry_mrb.rb

family_mruby_main.o: entry_mrb.h

entry_mrb.h: $(COMPONENT_PATH)/mrb/$(MRUBY_ENTRY)
	$(COMPONENT_PATH)/../components/mruby_component/mruby/bin/mrbc -B entry_mrb -o $@ $^

.PHONY: entry_mrb.h
