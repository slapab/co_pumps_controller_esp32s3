#pragma once

#include "tinyfsm.hpp"
#include "bsp.h"

namespace pumps {

enum pump_id_t {
    PUMP_GROUND_FLOOR = BSP_PUMP_RELAY_GROUND_FLOOR_GPIO_NUM,
    PUMP_FLOOR_1 = BSP_PUMP_RELAY_FLOOR_1_GPIO_NUM,
};


struct evt_base : public tinyfsm::Event {};

struct on_evt : public evt_base {};

struct off_evt : public evt_base {};


template <pump_id_t ID>
class pump : public tinyfsm::Fsm<pump<ID>> {
public:
    void set_relay_state(const bool on);

    /** Default reaction for unhandled events */
    void react(tinyfsm::Event const &) { }

    virtual void react(const on_evt &) { }
    virtual void react(const off_evt &) { }

    /* Entry actions in some states */
    virtual void entry() { }

    /** No exit actions */
    void exit()  { }
};

using pump_ground_floor_fsm = pumps::pump<pumps::PUMP_GROUND_FLOOR>;
using pump_floor1_fsm = pumps::pump<pumps::PUMP_FLOOR_1>;

} /* namespace pumps */
