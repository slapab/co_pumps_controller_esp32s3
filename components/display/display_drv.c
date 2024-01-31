#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_timer.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "bsp.h"
#include "touch_drv.h"
#include "display_ui_c_export.h"
#include "ui.h"

#include <esp_log.h>
#define TAG "disp_drv"

#define EXAMPLE_LVGL_TASK_STACK_SIZE   (4u * 1024u)
#define EXAMPLE_LVGL_TASK_PRIORITY     2u


#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

// PCLK frequency can't go too high as the limitation of PSRAM bandwidth
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (2 * 1000 * 1000)

#define EXAMPLE_PIN_NUM_DATA0          GPIO_NUM_39
#define EXAMPLE_PIN_NUM_DATA1          GPIO_NUM_40
#define EXAMPLE_PIN_NUM_DATA2          GPIO_NUM_41
#define EXAMPLE_PIN_NUM_DATA3          GPIO_NUM_42
#define EXAMPLE_PIN_NUM_DATA4          GPIO_NUM_45
#define EXAMPLE_PIN_NUM_DATA5          GPIO_NUM_46
#define EXAMPLE_PIN_NUM_DATA6          GPIO_NUM_47
#define EXAMPLE_PIN_NUM_DATA7          GPIO_NUM_48

#define EXAMPLE_PIN_NUM_RD             GPIO_NUM_9
#define EXAMPLE_PIN_NUM_WR             GPIO_NUM_8
#define EXAMPLE_PIN_NUM_CS             GPIO_NUM_6
#define EXAMPLE_PIN_NUM_DC             GPIO_NUM_7
#define EXAMPLE_PIN_NUM_RST            GPIO_NUM_5
#define EXAMPLE_PIN_NUM_BK_LIGHT       GPIO_NUM_38
#define LCD_PIN_NUM_POWER_ON           GPIO_NUM_15

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              BSP_LCD_H_RES
#define EXAMPLE_LCD_V_RES              BSP_LCD_V_RES

// Bit number used to represent command and parameter
// #if CONFIG_EXAMPLE_LCD_I80_CONTROLLER_ST7789
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8
// #endif


// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
// TODO vierify with our PSRAM if 64 i supported, 128 can be used?
#define EXAMPLE_PSRAM_DATA_ALIGNMENT   64

#define EXAMPLE_LVGL_TICK_PERIOD_MS    UINT32_C(5)

static void config_lcd_bl();
static void example_init_i80_bus(esp_lcd_panel_io_handle_t *io_handle, void *user_ctx);
static void example_init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel);
static void init_lvgl_touch_device(lv_disp_t *lvgl_disp, esp_lcd_touch_handle_t touch_handle);
static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                            esp_lcd_panel_io_event_data_t *edata,
                                            void *user_ctx);
static void my_disp_flush_lvgl_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void example_increase_lvgl_tick(void *arg);
// static void example_lvgl_port_task(void *arg);

/* Contains internal graphic buffer(s) called draw buffer(s) */
static lv_disp_draw_buf_t disp_buf;
/* Contains callback functions */
static lv_disp_drv_t disp_drv;
/* Input LVGL device driver */
static lv_indev_drv_t indev_drv;

/* LVGL Task handle */
static TaskHandle_t lvgl_task = NULL;

#if 0
static uint16_t bitmap[20*20];
#endif

esp_err_t display_drv_init(void)
{
#warning "FIXME"
    esp_err_t err = ESP_OK; // FIXME

    config_lcd_bl();

    esp_lcd_panel_io_handle_t io_handle = NULL;
    example_init_i80_bus(&io_handle, &disp_drv);

    esp_lcd_panel_handle_t panel_handle = NULL;
    example_init_lcd_panel(io_handle, &panel_handle);

    // Stub: user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    esp_lcd_touch_handle_t touch_handle = NULL;
    err                                 = ESP_ERROR_CHECK_WITHOUT_ABORT(touch_drv_init(&touch_handle));
#if 0
    for (uint32_t idx = 0u; idx < sizeof(bitmap) / sizeof(bitmap[0]); ++idx) {
        if (idx < (sizeof(bitmap) / sizeof(bitmap[0]) / 2)) {
          bitmap[idx] = 0x87FF;
        } else {
          bitmap[idx] =  0xFFF0;
        }
    }
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 20, 20, bitmap));

#endif

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    // alloc draw buffers used by LVGL. We have got a lot of memory so allocate for the whole screen
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    uint32_t malloc_flags = (MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM); /* or use MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA
                                                                       instead of MALLOC_CAP_SPIRAM */
    size_t buff_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(lv_color_t) / 5u;
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(buff_size, malloc_flags);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(buff_size, malloc_flags);

    assert(buf1);
    assert(buf2);
    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, buff_size / sizeof(lv_color_t));

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res   = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res   = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb  = my_disp_flush_lvgl_cb;
    disp_drv.draw_buf  = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp    = lv_disp_drv_register(&disp_drv);

    init_lvgl_touch_device(disp, touch_handle);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    esp_timer_create_args_t lvgl_tick_timer_args = {};
    lvgl_tick_timer_args.callback                = &example_increase_lvgl_tick;
    lvgl_tick_timer_args.dispatch_method         = ESP_TIMER_TASK;
    lvgl_tick_timer_args.name                    = "lvgl_tick";

    esp_timer_handle_t lvgl_tick_timer = NULL;
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(
        esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * UINT64_C(1000)));

    /* Create the LVGL task. */
    if (ESP_OK == err)
    {
        xTaskCreate(
            display_ui_task, "LVGL-UI", EXAMPLE_LVGL_TASK_STACK_SIZE, disp, EXAMPLE_LVGL_TASK_PRIORITY, &lvgl_task);
    }

    return err;
}

static void config_lcd_bl()
{
    gpio_config_t bk_gpio_config = {};
    bk_gpio_config.pin_bit_mask  = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT;
    bk_gpio_config.mode          = GPIO_MODE_OUTPUT;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&bk_gpio_config));

    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);
    ESP_LOGD(TAG, "Configured LCD backlight. Is OFF");
}

static void example_init_i80_bus(esp_lcd_panel_io_handle_t *io_handle, void *user_ctx)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_DC,
        .wr_gpio_num = EXAMPLE_PIN_NUM_WR,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0,
            EXAMPLE_PIN_NUM_DATA1,
            EXAMPLE_PIN_NUM_DATA2,
            EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4,
            EXAMPLE_PIN_NUM_DATA5,
            EXAMPLE_PIN_NUM_DATA6,
            EXAMPLE_PIN_NUM_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = EXAMPLE_LCD_H_RES * 100 * sizeof(uint16_t),
        .psram_trans_align = EXAMPLE_PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = EXAMPLE_PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = user_ctx,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = !LV_COLOR_16_SWAP,  // Swap can be done in LvGL (default) or DMA
        }
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));

    /* Configure also the RD line. As the i80-8bit driver doesn't user RD that means id doesn't read back dispaly data or status,
        so according to the datasheet need to set it high. */
    gpio_config_t rd_gpio_config = {
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_RD,
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&rd_gpio_config));
    gpio_set_level(EXAMPLE_PIN_NUM_RD, 1);
}

static void example_init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel)
{
    esp_lcd_panel_handle_t panel_handle = NULL;
    // #if CONFIG_EXAMPLE_LCD_I80_CONTROLLER_ST7789
    ESP_LOGI(TAG, "Install LCD driver of st7789");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        // .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, seems like there was field in some old library
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = false
        },
        .vendor_config = NULL
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    esp_lcd_panel_invert_color(panel_handle, true);
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, /*35*/0, /*0*/35));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
// #endif
    *panel = panel_handle;
}

static void init_lvgl_touch_device(lv_disp_t *lvgl_disp, esp_lcd_touch_handle_t touch_handle)
{
    lv_indev_drv_init(&indev_drv);
    /* Input device driver (Touch) */
    indev_drv.type      = LV_INDEV_TYPE_POINTER;
    indev_drv.disp      = lvgl_disp;
    indev_drv.read_cb   = lvgl_touch_cb;
    indev_drv.user_data = touch_handle;
    lv_indev_drv_register(&indev_drv);
}

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    /* When the i80 interface finished with flushing buffer inform the LVGL that buffer has been flushed */
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void my_disp_flush_lvgl_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)disp->user_data;
    int offsetx1                        = area->x1;
    int offsetx2                        = area->x2;
    int offsety1                        = area->y1;
    int offsety2                        = area->y2;
    // copy a buffer's content to a specific area of the display, note that + 1
    // for end x and y are because end x y are not included according to the API
    // doc.
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);
}

static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    lv_point_t point = { 0 };

    /* Read touch controller data */
    bool touchpad_pressed = touch_drv_read(&point.x, &point.y);

    if (true == touchpad_pressed)
    {
        data->point = point;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void example_increase_lvgl_tick(void *arg)
{
    (void)arg;
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

// static void example_lvgl_port_task(void *arg)
// {
//     lv_disp_t *disp = (lv_disp_t*)arg;
//     static lv_style_t style;
//     lv_style_init(&style);

//     ESP_LOGI(TAG, "Starting LVGL task");
//     uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_INTERVAL_MS;
//     uint32_t t_slicking_tp = UINT32_C(0);

//     ui_init();

//     bool display_sth = false;
//     while (true) {
//         // Lock the mutex due to the LVGL APIs are not thread-safe
//         if ((xTaskGetTickCount() - t_slicking_tp) >= pdMS_TO_TICKS(task_delay_ms)) {
//             t_slicking_tp = xTaskGetTickCount();
//             if (true == display_drv_lock(portMAX_DELAY)) {
//                 task_delay_ms = lv_timer_handler();
//                 // Release the mutex
//                 display_drv_unlock();

//                 if (true == display_sth) {
//                     display_sth = false;
//                     /* Create Screen object and load it (make it as active) */
//                     lv_obj_t *scr = lv_disp_get_scr_act(disp);
//                     if (NULL == scr) {
//                         lv_obj_t* scr = lv_obj_create(NULL);
//                         lv_scr_load(scr);
//                     }


//                     ESP_LOGI(TAG, "Created some object");
//                     /*Set a background color and a radius*/
//                     lv_style_set_radius(&style, 5);
//                     lv_style_set_bg_opa(&style, LV_OPA_COVER);
//                     lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));

//                     /*Add a shadow*/
//                     lv_style_set_shadow_width(&style, 55);
//                     lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));
//                     // lv_style_set_shadow_ofs_x(&style, 10);
//                     // lv_style_set_shadow_ofs_y(&style, 20);

//                     /*Create an object with the new style*/
//                     lv_obj_t * obj = lv_obj_create(scr);
//                     lv_obj_add_style(obj, &style, 0);
//                     lv_obj_set_size(obj, 70, 70);
//                     lv_obj_center(obj);

//                     lv_obj_t* btn = lv_btn_create(scr);
//                     lv_obj_t *lbl = lv_label_create(btn);
//                     lv_label_set_text_static(lbl, "Test");
//                     lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
//                     // lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 30, -30);
//                     lv_obj_set_pos(btn, 70, 70);
//                 }
//             }
//         }

//         if (task_delay_ms > EXAMPLE_LVGL_TASK_INTERVAL_MS) {
//             task_delay_ms = EXAMPLE_LVGL_TASK_INTERVAL_MS;
//             vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
//         } else if (0u == task_delay_ms) {
//             continue;
//         } else {
//             vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
//         }
//     }
// }
