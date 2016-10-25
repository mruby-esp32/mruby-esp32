
include $(IDF_PATH)/make/component_common.mk

COMPONENT_EXTRA_CLEAN := simplest_mrb.h

mruby_main.o: simplest_mrb.h

simplest_mrb.h: $(COMPONENT_PATH)/simplest_mrb.rb
	$(COMPONENT_PATH)/../components/mruby_component/mruby/bin/mrbc -B simplest_mrb -o $@ $^

