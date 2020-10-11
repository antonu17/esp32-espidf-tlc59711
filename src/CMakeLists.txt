# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"colors.c"
	"main.c"
	"tlc59711.c"
	"wifi.c"
	"http/http.c"
	"http/rest_server.c"
	"http/file_server.c"
	"mic.c"
	"spiffs.c"
	"framebuffer.c"
	"effects/cube_color_wheel.c"
	"effects/wave_color_wheel.c"
	"effects/effects.c"
)

idf_component_register(SRCS ${app_sources})
