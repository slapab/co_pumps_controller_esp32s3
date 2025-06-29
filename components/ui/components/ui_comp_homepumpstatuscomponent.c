// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"


// COMPONENT HomePumpStatusComponent

lv_obj_t * ui_HomePumpStatusComponent_create(lv_obj_t * comp_parent)
{

    lv_obj_t * cui_HomePumpStatusComponent;
    cui_HomePumpStatusComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_HomePumpStatusComponent, 100);
    lv_obj_set_height(cui_HomePumpStatusComponent, 100);
    lv_obj_set_x(cui_HomePumpStatusComponent, 0);
    lv_obj_set_y(cui_HomePumpStatusComponent, -10);
    lv_obj_set_align(cui_HomePumpStatusComponent, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cui_HomePumpStatusComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_HomePumpStatusComponent, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(cui_HomePumpStatusComponent, LV_OBJ_FLAG_CHECKABLE);     /// Flags
    lv_obj_clear_flag(cui_HomePumpStatusComponent, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_pad_left(cui_HomePumpStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_HomePumpStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_HomePumpStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_HomePumpStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(cui_HomePumpStatusComponent, lv_color_hex(0x34ED22), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(cui_HomePumpStatusComponent, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(cui_HomePumpStatusComponent, 15, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(cui_HomePumpStatusComponent, 5, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t * cui_nameLabel;
    cui_nameLabel = lv_label_create(cui_HomePumpStatusComponent);
    lv_obj_set_width(cui_nameLabel, LV_SIZE_CONTENT);   /// -415
    lv_obj_set_height(cui_nameLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(cui_nameLabel, LV_ALIGN_CENTER);
    lv_label_set_text(cui_nameLabel, "Level");

    lv_obj_t * cui_tempLabel;
    cui_tempLabel = lv_label_create(cui_HomePumpStatusComponent);
    lv_obj_set_width(cui_tempLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(cui_tempLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(cui_tempLabel, LV_ALIGN_CENTER);
    lv_label_set_text(cui_tempLabel, "0 °C");
    lv_obj_set_style_text_font(cui_tempLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * cui_statusBar;
    cui_statusBar = lv_bar_create(cui_HomePumpStatusComponent);
    lv_obj_set_height(cui_statusBar, 10);
    lv_obj_set_width(cui_statusBar, lv_pct(100));
    lv_obj_set_align(cui_statusBar, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(cui_statusBar, lv_color_hex(0xCCEDD1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_statusBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(cui_statusBar, lv_color_hex(0x17C92F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_statusBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_HOMEPUMPSTATUSCOMPONENT_NUM);
    children[UI_COMP_HOMEPUMPSTATUSCOMPONENT_HOMEPUMPSTATUSCOMPONENT] = cui_HomePumpStatusComponent;
    children[UI_COMP_HOMEPUMPSTATUSCOMPONENT_NAMELABEL] = cui_nameLabel;
    children[UI_COMP_HOMEPUMPSTATUSCOMPONENT_TEMPLABEL] = cui_tempLabel;
    children[UI_COMP_HOMEPUMPSTATUSCOMPONENT_STATUSBAR] = cui_statusBar;
    lv_obj_add_event_cb(cui_HomePumpStatusComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_HomePumpStatusComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    ui_comp_HomePumpStatusComponent_create_hook(cui_HomePumpStatusComponent);
    return cui_HomePumpStatusComponent;
}

