idf_component_register(
    SRCS
        "effect.c"
        "effect_functions/ball.c"
        "effect_functions/cube_color_wheel.c"
        "effect_functions/fireworks.c"
        "effect_functions/linespin.c"
        "effect_functions/mic_spectrum.c"
        "effect_functions/rain.c"
        "effect_functions/ripples.c"
        "effect_functions/shift_planes.c"
        "effect_functions/shift_suspend.c"
        "effect_functions/sidewaves.c"
        "effect_functions/sinelines.c"
        "effect_functions/vertical_travelers.c"
        "effect_functions/wave_color_wheel.c"
        "effect_functions/wireframes.c"
    INCLUDE_DIRS "." "effect_functions"
    REQUIRES cube_framebuffer ics43434 log
)
