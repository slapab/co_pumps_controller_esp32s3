SET(UI_SOURCES
    screens/ui_ScreenHome.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
)


idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS "."
    REQUIRES
    PRIV_REQUIRES lvgl
    )
