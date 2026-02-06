#include "encoder.hpp"

CallbackReturn MotorEncoder::on_configure(uint pin, EncoderTickCallback tick_cb, int timeout, uint32_t min_interval_us) {
    if (pin > 27) {
        // outside of GPIO range.
        return CallbackReturn::FAILURE;
    }
    pin_ = pin;
    
    if (tick_cb == nullptr) {
        return CallbackReturn::FAILURE;
    }
    tick_cb_ = tick_cb;
    timeout_ = timeout;
    min_interval_us_ = min_interval_us;
    return CallbackReturn::SUCCESS;
}

CallbackReturn MotorEncoder::on_activate() {

    if (tick_cb_ == nullptr) {
        return CallbackReturn::FAILURE;
    }

    if (pin_ < 0) {
        return CallbackReturn::FAILURE;
    }

    // For production, this should use a switch which provides feedback.
    if (gpioSetMode(pin_, PI_INPUT) != 0) {
        return CallbackReturn::FAILURE;
    }

    // For production, this should use a switch which provides feedback.
    if (gpioSetPullUpDown(pin_, PI_PUD_DOWN) != 0) {
        return CallbackReturn::FAILURE;
    }

    last_tick_ = gpioTick();

    switch (gpioSetISRFuncEx(
            pin_,
            RISING_EDGE,  
            timeout_,            
            &MotorEncoder::gpio_isr_func,
            this
        )) {
        case 0:
            break;
        case PI_BAD_GPIO:
            return CallbackReturn::FAILURE;
        case PI_BAD_EDGE:
            return CallbackReturn::FAILURE;
        case PI_BAD_ISR_INIT:
            return CallbackReturn::FAILURE;
        default:
            return CallbackReturn::FAILURE;
    }


    return CallbackReturn::SUCCESS;
}

CallbackReturn  MotorEncoder::on_deactivate() {
    gpioSetISRFuncEx(pin_, expected_level_, 0, nullptr, nullptr);
    return CallbackReturn::SUCCESS;
}


// Static wrapper - required for C function pointer compatibility
void MotorEncoder::gpio_isr_func(int gpio, int level, uint32_t tick, void *userdata) {
    auto* self = static_cast<MotorEncoder*>(userdata);
    self->handle_interrupt(gpio, level, tick);
}

void MotorEncoder::handle_interrupt(int gpio, int level, uint32_t tick) {
    /*
     0 = change to low (a falling edge)
     1 = change to high (a rising edge)
     2 = no level change (interrupt timeout)
    */

    TickStatus status = TickStatus::HEALTHY;
    if (level != expected_level_) {
        status = TickStatus::NOISE_REJECTED;
        if (level == 2) {
            status = TickStatus::TIMEOUT;
        }
    }
    int32_t delta_us = tick - last_tick_;
    last_tick_ = tick;
    tick_cb_(gpio, delta_us, tick, status);
}