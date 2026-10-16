idf_component_register(
    SRCS "colors.c" "draw.c" "framebuffer.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer tlc59711
)
