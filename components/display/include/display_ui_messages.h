#pragma once

/**
 * @file Header that shall be C-compatible of messages being targeted for the DISPLAY_UI module.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A list of supported command names and their data types.
 * @note That all definitions of the data type shall have first item of @ref display_ui_msg_base_t
 * @param first -> a enum name,
 * @param second -> a type prefix name. To this name the preprocessor will add prefix: disp_ui_ and the postfix: _t postfix.
 */
#define DISPLAY_UI_MSGS_LIST_X \
    X(DISP_UI_MSG_TEMP_UPD, temp_upd)\
    X(DISP_UP_MSG_CTRL_STATE_CHANGED, ctrl_state_changed)

enum display_ui_msg_id_t {
    #define X(enum_name, data_name) enum_name,
    DISPLAY_UI_MSGS_LIST_X
    #undef X

    DISP_UI_MSG_NUM
};

struct display_ui_msg_base_t {
    display_ui_msg_id_t msg_id;
};

struct disp_ui_msg_temp_upd_t
{
    int temp_sensor_id; /**< Explicitly not using temperatures.hpp's @see temp_sensor_t, as wanted this header
                           to be C/C++ portable. */
    float temp;
};

struct disp_ui_msg_ctrl_state_changed_t
{
    int ctrl_id;    /**< Explicitly not using controller.hpp;s @see ctrl_id_t, as wanted to be C/C++ portable. */
    bool state;     /**< Controller's pump state, true = on, false = off. */
};

/* Helper type to define the maximum size for command. Required to define a queue's item size. */
struct disp_ui_msg_t {
    display_ui_msg_id_t msg_id;

    union {
        #define X(enum_name, data_name) disp_ui_msg_##data_name##_t data_name;
        DISPLAY_UI_MSGS_LIST_X
        #undef X
    };
};

#ifdef __cplusplus
}
#endif
