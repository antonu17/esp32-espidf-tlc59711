# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"main.c"
	"effect_list.c"
	"event_loop.c"
	"spiffs.c"
	"colors.c"
	"draw.c"
	"tlc59711.c"
	"framebuffer.c"
	"wifi.c"
	"stats.c"
	"coob/coob_state.c"
	"coob/loop_state.c"
	"coob/solo_state.c"
	"coob/coob.c"
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

set(includedirs
	"coob/"
	"effects//"
)

idf_component_register(SRCS ${app_sources}
					INCLUDE_DIRS ${includedirs}
					REQUIRES esp-dsp)
