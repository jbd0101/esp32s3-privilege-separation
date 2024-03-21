set(src "protected_main.c")

idf_component_register(SRCS ${src}
                       INCLUDE_DIRS ".."
                       REQUIRES bootloader_support spi_flash esp_priv_access user_kernel_pipeline ws2812)

set(EXTRA_COMPONENT_DIRS
        ${CMAKE_CURRENT_LIST_DIR}/../components)