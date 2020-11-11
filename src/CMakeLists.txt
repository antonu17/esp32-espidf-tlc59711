# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"main.c"
	"effect_loop.c"
	"event_loop.c"
	"spiffs.c"
	"colors.c"
	"draw.c"
	"tlc59711.c"
	"framebuffer.c"
	"wifi.c"
	"http/http.c"
	"http/rest_server.c"
	"http/file_server.c"
	"effects/cube_color_wheel.c"
	"effects/wave_color_wheel.c"
	"effects/mic_spectrum.c"
	"effects/shift_planes.c"
	"effects/rain.c"
	"effects/vertical_travelers.c"
	"effects/shift_suspend.c"
	"effects/wireframes.c"
	"effects/ripples.c"
	"effects/sidewaves.c"
	"effects/ball.c"
	"effects/test.c"
	"effects/fireworks.c"
	"effects/sinelines.c"
	"effects/linespin.c"
)

idf_component_register(SRCS ${app_sources}
					REQUIRES esp-dsp)
