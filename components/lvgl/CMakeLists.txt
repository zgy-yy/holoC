file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE LV_DEV_SOURCES lvgl_driver/*.c)
file(GLOB_RECURSE DEMOS_SOURCES demos/*.c)
idf_component_register(SRCS ${SOURCES}
        SRCS ${LV_DEV_SOURCES}
        SRCS ${DEMOS_SOURCES}
        INCLUDE_DIRS . src ../ ./lvgl_driver ./demos
        PRIV_REQUIRES drivers esp_timer fatfs)