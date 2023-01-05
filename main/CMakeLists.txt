set(MRBC_COMMAND "${COMPONENT_DIR}/../components/mruby_component/mruby/bin/mrbc")

idf_component_register(
  SRCS mruby_main.c
  INCLUDE_DIRS .
  REQUIRES mruby_component
  PRIV_REQUIRES nvs_flash
)

add_custom_command(
  OUTPUT example_mrb.h
  COMMAND ${MRBC_COMMAND} -B example_mrb -o example_mrb.h ${COMPONENT_DIR}/examples/${MRUBY_EXAMPLE}
  WORKING_DIRECTORY ${COMPONENT_DIR}
  VERBATIM
)

add_custom_target(main DEPENDS example_mrb.h)
add_dependencies(${COMPONENT_LIB} main)
