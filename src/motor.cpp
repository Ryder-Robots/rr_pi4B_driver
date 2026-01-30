#include "motor.hpp"

/**
 * In final versions this should be done as an interface that can be used by concrete classes (may be plugins).
 *
 * The idea being it is not predictable exactly which implementation is required for a given bit of hardware.
 *
 * For example normally H-Bridges will accept EN0, and EN1 which will allow for different direction control
 * but others will not. Also I2C will use a protocol language rather than pushing pins to HIGH and LOW, and 
 * providing a duty cycle on a PWM enable pin, thus the interface can remain the same.
 *
 * A higher value in pwm_pin will increase velocity, and dir_pin will define as spinning a motor clockwise
 * when true, and spinning a motor anti-clockwise when false.
 */


/**
* Performs configuration, this does not engage the hardware, just sets variables that will
* be used at later stages.
*
*/
CallbackReturn Motor::on_configure(uint pwm_pin, uint dir_pin, int pi) {

    if (pi < 0) {
        std::cout << "ERROR: pi daemon is not correct\n";
        return CallbackReturn::FAILURE;
    }
    pi_ = pi;

    switch(pwm_pin) {
        case 12:
            break;
        case 13:
            break;
        case 18:
            break;
        case 19:
            break;
        default:
            std::cout << "ERROR: non PWM pin\n";
            return CallbackReturn::FAILURE;
    }

    pwm_pin_ = pwm_pin; 

    if (dir_pin == pwm_pin_) {
        std::cout << "ERROR: pin assigned previously\n";
        return CallbackReturn::FAILURE;           
    }
    dir_pin_ = dir_pin;
    return CallbackReturn::SUCCESS;
}

// create links with hardware. Perform error checking, and fail if something goes wrong.
CallbackReturn Motor::on_activate() {

    if (set_mode_internal(dir_pin_, PI_OUTPUT) != OK) {
        std::cout << "ERROR: pin " << dir_pin_ << "had errors\n";
        return CallbackReturn::FAILURE;
    }
    

    // set to ALT5 for PWM hardware output.
    // could add support for soft PWM, but not in this test
    if (set_mode_internal(pwm_pin_, PI_ALT5) != OK) {
        std::cout << "ERROR: pin " << pwm_pin_ << "had errors\n";
        return CallbackReturn::FAILURE;
    }

    {
        int r = OK;
        if ((r = gpioWrite(dir_pin_, LOW)) != OK) {
            switch(r) {
                case PI_BAD_GPIO:
                    std::cout << "ERROR: pin " << dir_pin_ << " returned PI_BAD_GPIO\n";
                    break;
                case PI_BAD_LEVEL:
                    std::cout << "ERROR: pin " << dir_pin_ << " returned PI_BAD_LEVEL\n";
                    break;
                case PI_NOT_PERMITTED:
                    std::cout << "ERROR: pin " << dir_pin_ << " returned PI_NOT_PERMITTED\n"; 
            }
            std::cout << "ERROR: pin " << dir_pin_ << "had errors!!!\n";
            return CallbackReturn::FAILURE;
        }
    }
    if (set_pwm(0, 0) != OK) return CallbackReturn::FAILURE;
    return CallbackReturn::SUCCESS;
}

CallbackReturn Motor::on_deactivate() {
    CallbackReturn exit_res = CallbackReturn::SUCCESS;
    
    // dont exit early, attempt to set DIR_PIN to low, event if PWM fails.
    // but do motor first, its better for hardware that we shutdown motors before direction.
    if (set_pwm(0, 0) != OK) { 
        exit_res = CallbackReturn::FAILURE;
    }

    {
        int r = OK;
        if ((r = gpioWrite(dir_pin_, LOW)) != OK) {
            // note fallthrough is delibrate here
            switch(r) {
                case PI_BAD_GPIO:
                    std::cout << "ERROR: pin " << dir_pin_ << "returned PI_BAD_GPIO\n";
                    [[fallthrough]];
                case PI_BAD_LEVEL:
                    std::cout << "ERROR: pin " << dir_pin_ << "returned PI_BAD_LEVEL\n";
                        [[fallthrough]];
                default:
                    return CallbackReturn::FAILURE; 
            }
        }
    }
    
    return exit_res;
}

// set the duty cycle to something that will work for testing.
int Motor::set_pwm(int duty) {
    return set_pwm(freq_, duty);
}

    
CallbackReturn Motor::set_direction(DIRECTION dir) {
    int r = OK;
    if ((r = gpioWrite(dir_pin_, dir)) != OK) {
        switch(r) {
            case PI_BAD_GPIO:
                std::cout << "ERROR: pin " << dir_pin_ << "returned PI_BAD_GPIO\n";
                [[fallthrough]];
            case PI_BAD_LEVEL:
                std::cout << "ERROR: pin " << dir_pin_ << "returned PI_BAD_LEVEL\n";
                [[fallthrough]];
            default:
                return CallbackReturn::FAILURE; 
        }
    }
    return CallbackReturn::SUCCESS; 
}



int Motor::set_pwm(int freq, int duty) {
    int r =  gpioHardwarePWM (pwm_pin_, freq, duty*DUTY_OFFSET);
    switch(r) {
        // note fallthrough is delibrate here
        case OK:
            return r;
        case PI_BAD_GPIO:
            std::cout << "ERROR: pin " << pwm_pin_ << "returned PI_BAD_GPIO\n";
            [[fallthrough]];
        case PI_NOT_HPWM_GPIO:
            std::cout << "ERROR: pin " << pwm_pin_ << "returned PI_NOT_HPWM_GPIO\n";
            [[fallthrough]];
        case PI_BAD_HPWM_DUTY:
            std::cout << "ERROR: pin " << pwm_pin_ << "returned PI_BAD_HPWM_DUTY\n";
            [[fallthrough]];
        case PI_BAD_HPWM_FREQ:
            std::cout << "ERROR: pin " << pwm_pin_ << "returned PI_BAD_HPWM_FREQ\n";
            [[fallthrough]];
        case PI_HPWM_ILLEGAL:
            std::cout << "ERROR: pin " << pwm_pin_ << "returned PI_HPWM_ILLEGAL\n";
            [[fallthrough]];
        default:
            std::cout << "ERROR: pin " << pwm_pin_ << "errors where found on pin!!!\n";
    }
    return r;    
}

int Motor::set_mode_internal(uint pin, uint mode) {
    switch (int r = gpioSetMode(pin, mode)) {
        case OK:
            return r;
        case PI_BAD_GPIO:
            std::cout << "ERROR: pin " << pin << "returned PI_BAD_GPIO\n";
            [[fallthrough]];
        case PI_BAD_MODE:
            std::cout << "ERROR: pin " << pin << "returned PI_BAD_MODE\n";
            [[fallthrough]];
        case PI_NOT_PERMITTED:
            std::cout << "ERROR: pin " << pin << "returned PI_NOT_PERMITTED\n";
            [[fallthrough]];
        default:
            std::cout << "ERROR: pin " << pin << "had errors!!!\n";
            return r;
    }
}
