idf_component_register(SRCS "v_engine.c" "v_primitives.c"
                       INCLUDE_DIRS "include"
                       REQUIRES v_hal v_math esp_timer)
