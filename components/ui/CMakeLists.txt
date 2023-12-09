SET(UI_SOURCES
    components/ui_comp_homepumpstatuscomponent.c
    components/ui_comp.c
    screens/ui_ScreenHome.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ./my_ui_events/screen_home_events.cpp)


idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS "." "components"
    REQUIRES
    PRIV_REQUIRES lvgl
    )
