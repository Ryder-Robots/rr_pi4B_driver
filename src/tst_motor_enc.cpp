/**
 * Test controllers as well motor controllers, note that improvements can be made to 
 * motor controllers which is documented in tst_motor_ctl_pigiod.cpp when moving to production.
 */

#include <atomic>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <pigpio.h>
#include <thread>

#include "encoder.hpp"
#include "motor.hpp"
// #include "motor_encoder.hpp"

// motor controller pins
#define PWM_A 18
#define PWM_B 19
#define DIR_A 23
#define DIR_B 24

// direction needs to be confirmed.
// encoder pins
#define EN_P1_A 9 // phase A on motor A
//  #define EN_P2_A 9 // phase B on motor B, will be ignored for the moment.

// #define EN_P1_B 8
// #define EN_P2_B 9

// pulses per revolution (this is based upon FIT0450)
//  #define PPR 16

std::list<int> l_gpio_pin = {};
std::list<uint32_t> l_delta_us = {};
std::list<uint32_t> l_tick = {};
std::list<TickStatus> l_tick_status = {};
std::mutex cb_mutex;

static void cb(
    int gpio_pin,
    uint32_t delta_us,
    uint32_t tick,
    TickStatus tick_status)
{
    std::lock_guard<std::mutex> lock(cb_mutex);
    l_gpio_pin.push_back(gpio_pin);
    l_delta_us.push_back(delta_us);
    l_tick.push_back(tick);
    l_tick_status.push_back(tick_status);
}

int main()
{
    // pre-initialization step
    {
        int rev = gpioHardwareRevision();
        if (rev == 0) {
            std::cout << "pigpiod has an unknown revision\n";
            return 1;
        }
        else {
            unsigned model = (rev >> 4) & 0xFF;

            std::cout << "Hardware revision: 0x" << std::hex << rev << std::dec << std::endl;
            std::cout << "Model number: " << model << std::endl;
            std::cout << "Expected model for Pi4B: 17" << std::endl;
        }
    }    

    int pi = gpioInitialise();
    if (pi < 0) {
        std::cout << "Failed to connect to pigpiod\n";
        return 1;
    }
    Motor motor_a;
    MotorEncoder en_a;

    // reset lists so they are all 0 elements at this point.
    l_gpio_pin.clear();
    l_delta_us.clear();
    l_tick.clear();
    l_tick_status.clear();

    if (motor_a.on_configure(PWM_A, DIR_A, pi) == CallbackReturn::FAILURE || en_a.on_configure(EN_P1_A, &cb, 0, 0) == CallbackReturn::FAILURE) {
        gpioTerminate();
        std::cout << "FAILED TO CONFIGURE!!! exiting program\n";
        return 1;
    }


    if (motor_a.on_activate() == CallbackReturn::FAILURE || en_a.on_activate() == CallbackReturn::FAILURE) {
        gpioTerminate();
        std::cout << "FAILED TO ACTIVATE!!! exiting program\n";
        return 1;
    }

    std::cout << "Testing motor A\n";
    motor_a.set_direction(FORWARD);
    // motor_b.set_direction(FORWARD);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    if (motor_a.set_pwm(65) == OK) {
        // sleep for 3 seconds to test motor
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    else {
        std::cout << "FAILED to set PWM for 65 percent of duty cycle\n";
        // allow for reset
        motor_a.set_pwm(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Increase speed motor A and B
    if (motor_a.set_pwm(85) == OK) {
        // sleep for 3 seconds to test motor
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    else {
        std::cout << "FAILED to set PWM for 85 percent of duty cycle\n";
        // allow for reset
        motor_a.set_pwm(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // deactivate and terminate before printing stats, in case some sort of problem
    // leaves program in an unstable state.
    motor_a.on_deactivate();
    en_a.on_deactivate();
    gpioTerminate();

    std::lock_guard<std::mutex> lock(cb_mutex);
    // list sizes should match, but to be paranoid, find the smallest.
    size_t s = std::min({l_gpio_pin.size(), l_delta_us.size(),
        l_tick.size(), l_tick_status.size()});

    if (s == 0) {
        std::cout << "\nWARNING: No encoder pulses detected!\n";
        std::cout << "Check encoder wiring on GPIO " << EN_P1_A << "\n";
        return 1;
    }

    std::cout << "GPIO,"
              << "DELTA_US,"
              << "TICK_US,"
              << "STATUS" << "\n";

    for (size_t i = 0; i < s; i++) {
        std::cout << l_gpio_pin.front() << ","
                  << l_delta_us.front() << ","
                  <<  l_tick.front() << ","
                  <<  (int)l_tick_status.front() << "\n";

        l_gpio_pin.pop_front();
        l_delta_us.pop_front();
        l_tick.pop_front();
        l_tick_status.pop_front();
    }

    return 0;
}