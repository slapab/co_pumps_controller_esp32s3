#include "display_ui_messages.h"
#include "resource_config.h"
#include "controller.hpp"
#include "pumps.hpp"
#include "display_ui.hpp"

#include <esp_log.h>

#define LOG_TAG CONFIG_LOG_TAG_CTRL

namespace ctrl {

/* Forward declarations */
template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class manual_state;

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class normal_state_off;

/* Static variables definitions */
template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
float controller<ID, PUMP_ID>::sett_threshold{30.0f};
template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
float controller<ID, PUMP_ID>::activate_threshold{30.0f};
template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
float controller<ID, PUMP_ID>::hysteresis{1.0f};
template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
float controller<ID, PUMP_ID>::temp{0.0f};

extern "C" void controller_bootstrap(void)
{
    ctrl_ground_floor_fsm::start();
    ctrl_floor1_fsm::start();
}

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
void controller<ID, PUMP_ID>::react(const temp_update_evt &e)
{
    /* Just save temperature as this is a common implementation! */
    temp = e.temp;

    ESP_LOGD(LOG_TAG, "ctrl[%d] got temp update %0.2f", ID, e.temp);
}

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
void controller<ID, PUMP_ID>::react(const manual_pump_ctrl_evt & e)
{
    /* Setup action to send proper event to bounded PUMP. */
    auto send_evt_to_pump_action = [&]()
    {
        if (true == e.on) {
            pump_fsm::dispatch(pumps::on_evt());
        }
        else {
            pump_fsm::dispatch(pumps::off_evt());
        }
    };

    /* Change state and as action send event to the bounded pump FMS*/
    if (true == e.on) {
        controller<ID, PUMP_ID>::template transit< manual_state<ID, PUMP_ID> >(send_evt_to_pump_action);
    }
    else
    {
        controller<ID, PUMP_ID>::template transit< normal_state_off<ID, PUMP_ID> >();
    }
}

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
void controller<ID, PUMP_ID>::send_ctrl_state_changed_msg(const bool new_state)
{
    const disp_ui_msg_t msg = {
        .msg_id = DISP_UP_MSG_CTRL_STATE_CHANGED,
        .ctrl_state_changed = {
            .ctrl_id = static_cast<int>(ID),
            .state = new_state
        }
    };
    DisplayUI::instance().send_msg(msg);
}

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class init_state : public controller<ID, PUMP_ID> {
    void entry() override
    {
        // TODO

        /* And go to the normal state */
        controller<ID, PUMP_ID>::template transit<normal_state_off<ID, PUMP_ID>>();
    }

    /* Intentionally override to skip events when not initialized yet. */
    void react(const temp_update_evt & e) override {}
    void react(const manual_pump_ctrl_evt & e) override {}

};

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class normal_state_on : public controller<ID, PUMP_ID> {
    using ctrl_t = controller<ID, PUMP_ID>;
    using pump_fsm = ctrl_t::pump_fsm;

public:

protected:
    void entry() override
    {
        ESP_LOGD(LOG_TAG, "[%d] entered NORMAL_ON_STATE", ID);
        pump_fsm::dispatch(pumps::on_evt());

        /* And change the activate_threshold to the sett threshold plus half of
         * hysteresis. This activates hysteresis. */
        ctrl_t::activate_threshold = ctrl_t::sett_threshold + (ctrl_t::hysteresis / 2.0f);

        ctrl_t::send_ctrl_state_changed_msg(true);
    }

    void react(const temp_update_evt & e) override
    {
        /* Use base implementation to store the temperature */
        ctrl_t::react(e);

        /* If temperature drops below the sett_threshold switch to the normal off state.
         * activate threshold to the settings threshold. This effectively means
         * that hysteresis did its job.  */
        if (e.temp < ctrl_t::sett_threshold)
        {
            ctrl_t::template transit< normal_state_off<ID, PUMP_ID> >();
        }
    }
};

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class normal_state_off : public controller<ID, PUMP_ID> {
    using ctrl_t = controller<ID, PUMP_ID>;
    using pump_fsm = ctrl_t::pump_fsm;

public:

protected:
    void entry() override
    {
        ESP_LOGD(LOG_TAG, "[%d] entered NORMAL_OFF_STATE", ID);
        pump_fsm::dispatch(pumps::off_evt());
        ctrl_t::send_ctrl_state_changed_msg(false);
    }

    void react(const temp_update_evt & e) override
    {
        /* Use base implementation to store the temperature */
        ctrl_t::react(e);

        /* If temperature drops below the sett_threshold - hist/2 then update
         * activate threshold to the settings threshold. This effectively means
         * that hysteresis did its job.  */
        if (e.temp <= (ctrl_t::sett_threshold - (ctrl_t::hysteresis / 2.0f)))
        {
            ctrl_t::activate_threshold = ctrl_t::sett_threshold;
        }

        /* Compare temp to the active threshold, if above then change state to the NORMAL_ON */
        if (e.temp >= ctrl_t::activate_threshold)
        {
            ctrl_t::template transit< normal_state_on<ID, PUMP_ID> >();
        }
    }
};

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class manual_state : public controller<ID, PUMP_ID>
{
public:

protected:
    void entry() override
    {
        ESP_LOGD(LOG_TAG, "[%d] entered MANUAL_STATE", ID);
    }

    /* Do nothing. The default implementation of manual control will handle state switching. */
    /* And temperatures updates will be just saved by the default handler. */
};


} /* namespace */

/* Define the FSM and set initial state of all supported pumps */

using initial_ground_floor_state = ctrl::init_state<ctrl::CTRL_GROUND_FLOOR, pumps::PUMP_GROUND_FLOOR>;
FSM_INITIAL_STATE(ctrl::ctrl_ground_floor_fsm, initial_ground_floor_state);

using initial_floor1_state = ctrl::init_state<ctrl::CTRL_FLOOR_1, pumps::PUMP_FLOOR_1>;
FSM_INITIAL_STATE(ctrl::ctrl_floor1_fsm, initial_floor1_state);
