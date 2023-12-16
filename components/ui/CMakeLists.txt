SET(UI_SOURCES
    components/ui_comp_homepumpstatuscomponent.c
    components/ui_comp_floorsettingscomponent.c
    components/ui_comp.c
    screens/ui_ScreenHome.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ./my_ui_events_actions/screen_home_events_actions.cpp)


idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS
        "."
        "components"
        "my_ui_events_actions/include"
    REQUIRES
    PRIV_REQUIRES
        lvgl
        controller
        config
        display
    )
