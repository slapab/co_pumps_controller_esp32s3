#include "pumps.hpp"
#include "hal/gpio_types.h"
#include "tinyfsm.hpp"
#include "resource_config.h"
#include <driver/gpio.h>
#include <esp_log.h>

#define LOG_TAG CONFIG_LOG_TAG_PUMPS

namespace pumps {

/* Forward declarations */
template <pump_id_t ID>
class on_state;

template <pump_id_t ID>
class off_state;

static constexpr inline bool logic_lvl_to_gpio_transl(const bool on)
{
    /* One to one matching*/
    return !on;
}

extern "C" void pumps_bootstrap(void)
{
    pump_ground_floor_fsm::start();
    pump_floor1_fsm::start();
}

template <pump_id_t ID>
void pump<ID>::set_relay_state(const bool on)
{
    gpio_set_level(static_cast<gpio_num_t>(ID), logic_lvl_to_gpio_transl(on));
}

template <pump_id_t ID>
class init_state : public pump<ID> {
    public:
    void entry() override {
        ESP_LOGD(LOG_TAG, "Doing GPIO initialization and sending off Events %d\n", ID);

        /* Set GPIO level to state that will indicate the PUMP off */
        pump<ID>::set_relay_state(false);

        gpio_config_t io_conf = {};
        /* disable interrupt */
        io_conf.intr_type = GPIO_INTR_DISABLE;
        /* set as output mode */
        io_conf.mode = GPIO_MODE_OUTPUT;
        /* bit mask of the pins that you want to set,e.g.GPIO18/19 */
        io_conf.pin_bit_mask = (1u << static_cast<uint32_t>(ID));
        /* disable pull-down mode */
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        /* disable pull-up mode */
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        /* configure GPIO with the given settings */
        gpio_config(&io_conf);

        /* And transit to the OFF state*/
        pump<ID>::template transit< off_state<ID> >();
    }
    protected:
};

template <pump_id_t ID>
class off_state : public pump<ID> {
public:

protected:
    void react(const on_evt& e) override
    {
        ESP_LOGD(LOG_TAG, "Setting ON for %d\n", ID);
        pump<ID>::set_relay_state(true);
        pump<ID>::template transit< on_state<ID> >();
    }

    void react(const off_evt&) override
    {
        /* Do nothing */
    }
};

template <pump_id_t ID>
class on_state : public pump<ID> {
public:

protected:
    void react(const on_evt&) override
    {
        /* Do nothing */
    }

    void react(const off_evt& e) override
    {
        ESP_LOGD(LOG_TAG, "Setting OFF for %d\n", ID);
        pump<ID>::set_relay_state(false);
        pump<ID>::template transit< off_state<ID> >();
    }
};

} /* namespace */

/* Define the FSM and set initial state of all supported pumps */
FSM_INITIAL_STATE(pumps::pump<pumps::PUMP_GROUND_FLOOR>, pumps::init_state<pumps::PUMP_GROUND_FLOOR>);
FSM_INITIAL_STATE(pumps::pump<pumps::PUMP_FLOOR_1>, pumps::init_state<pumps::PUMP_FLOOR_1>);
