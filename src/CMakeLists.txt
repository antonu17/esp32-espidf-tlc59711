# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"main.c"
	"nvs.c"
	"effect.c"
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
	"coob/switching_state.c"
	"coob/coob.c"
	"coob/persistence.c"
	"coob/effect_launcher.c"
	"http/http.c"
	"http/rest_server.c"
	"effect_functions/cube_color_wheel.c"
	"effect_functions/wave_color_wheel.c"
	"effect_functions/mic_spectrum.c"
	"effect_functions/shift_planes.c"
	"effect_functions/rain.c"
	"effect_functions/vertical_travelers.c"
	"effect_functions/shift_suspend.c"
	"effect_functions/wireframes.c"
	"effect_functions/ripples.c"
	"effect_functions/sidewaves.c"
	"effect_functions/ball.c"
	"effect_functions/test.c"
	"effect_functions/fireworks.c"
	"effect_functions/sinelines.c"
	"effect_functions/linespin.c"
)

set(includedirs
	"coob/"
	"effect_functions/"
)

idf_component_register(SRCS ${app_sources}
					INCLUDE_DIRS ${includedirs}
					REQUIRES esp-dsp)
