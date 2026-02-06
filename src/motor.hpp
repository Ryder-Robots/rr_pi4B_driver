#pragma once

#include <cstddef>
#include <iostream>
#include <pigpio.h> 
#include <thread>
#include <atomic>
#include "tst_common.hpp"


enum DIRECTION {
    BACKWARD = 0,
    FORWARD = 1,
};

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
class Motor {
    public:

    /**
    * Performs configuration, this does not engage the hardware, just sets variables that will
    * be used at later stages.
    *
    */
    CallbackReturn on_configure(uint pwm_pin, uint dir_pin, int pi);

    // create links with hardware. Perform error checking, and fail if something goes wrong.
    CallbackReturn on_activate();

    CallbackReturn on_deactivate();

    // set the duty cycle to something that will work for testing.
    int set_pwm(int duty);

    int set_pwm(int freq, int duty);

    
    CallbackReturn set_direction(DIRECTION dir);

    // This is not correct, but gives an idea of a method that interacts with
    // the hardware, this will be performed mostly likely through a ROS2 action.

 private:
    uint pwm_pin_ = 0; // set speed of the pin
    uint dir_pin_ = 0; // set direction.
    int pi_ = -1;

    // TC78H660FTG have an adjustable OSCM which means that the frequency can be anything, and since 
    // 
    // The TC78H660FTG brushed DC motor driver accepts input PWM frequencies from DC up to 400 kHz max. For optimal 
    // performance with small DC motors, use 500 Hz to 1 kHz. So going with a frequency of 1000Hz (1kHz), but want to 
    // make this adjustable, it could be something that PID algorithm adjusts on the fly.
    // int freq_ = 1000; // hardware real range for PWM pin 
    int freq_ = 700;
    
    // Keep the range of the between 0-100, if its found that more granulatity is needed this can always
    // be scaled down to give more range, say 0-1000 etc.
    const int DUTY_OFFSET = 10000;


    int set_mode_internal(uint pin, uint mode);
};