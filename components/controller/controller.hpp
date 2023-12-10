#pragma once

#include "tinyfsm.hpp"
#include "temperatures.hpp"
#include "pumps.hpp"

namespace ctrl {

enum ctrl_id_t {
    CTRL_GROUND_FLOOR,
    CTRL_FLOOR_1
};

struct temp_update_evt : public tinyfsm::Event {
    temp_update_evt(const float temp)
        : temp(temp) {}

    const float temp;
};

struct manual_pump_ctrl_evt : public tinyfsm::Event {
    manual_pump_ctrl_evt(const bool on)
        : on(on) { }

    const bool on;
};

template <ctrl_id_t ID, pumps::pump_id_t PUMP_ID>
class controller : public tinyfsm::Fsm<controller<ID, PUMP_ID>> {
public:
    /** Default reaction for unhandled events */
    void react(tinyfsm::Event const &) { }

    virtual void react(const temp_update_evt & e);
    virtual void react(const manual_pump_ctrl_evt & e);

    /* Entry actions in some states */
    virtual void entry() { }

    /* Entry actions in some states */
    virtual void exit()  { }

protected:
    using pump_fsm = pumps::pump<PUMP_ID>;
};

using ctrl_ground_floor_fsm = ctrl::controller<ctrl::CTRL_GROUND_FLOOR, pumps::PUMP_GROUND_FLOOR>;
using ctrl_floor1_fsm = ctrl::controller<ctrl::CTRL_FLOOR_1, pumps::PUMP_FLOOR_1>;

} /* namespace */
