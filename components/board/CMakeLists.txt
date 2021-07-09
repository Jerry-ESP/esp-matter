set(COMPONENT_ADD_INCLUDEDIRS include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES led_driver button_driver)

set(COMPONENT_SRCS ${BOARD_PATH}/board.c)

register_component()
