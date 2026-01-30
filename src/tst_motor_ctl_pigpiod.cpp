/**
 * Provides simple inteface that will test motors, this can be used as a template for
 * creating motor controller NOdes in ROS2.
         // pwma_pin_ = 18;
        // pwmb_pin_ = 19;
        // dira_pin_ = 23;
        // dirb_pin_ = 24;
 */

 /*
 
 For safety real code will need the following added

 Duty boundsMODERATE30-40%Motor damage❌ 
 MissingDirection changeMODERATE40-50%H-bridge/motor stress⚠️ 
 No protectionThread safetyHIGH (ROS2)60-70%Unpredictable behavior⚠️ 
 Not thread-safeCurrent limitUNKNOWN?Motor overheating❌ 
 No monitoring
 */

#include "motor.hpp"

#define PWM_A 18
#define PWM_B 19
#define DIR_A 23
#define DIR_B 24


void test_motor(Motor &motor) {
    motor.set_direction(FORWARD);

    // should set motor to 65%
    std::this_thread::sleep_for(std::chrono::microseconds(10)); 
    if (motor.set_pwm(65) == OK) {  
        // sleep for 3 seconds to test motor
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    motor.on_deactivate();
}


int main() {

    int pi = gpioInitialise();
    if (pi < 0) {
        std::cout << "Failed to connect to pigpiod\n";
        return 1;
    }

    Motor motor_a, motor_b;
    if (motor_a.on_configure(PWM_A, DIR_A, pi) == CallbackReturn::FAILURE) {
        gpioTerminate();
        std::cout << "FAILED TO CONFIGURE!!! exiting program\n";
        return 1;
    }

    if (motor_b.on_configure(PWM_B, DIR_B, pi) == CallbackReturn::FAILURE) {
        gpioTerminate();
        std::cout << "FAILED TO CONFIGURE!!! exiting program\n";
        return 1;
    }

    if (motor_a.on_activate() == CallbackReturn::FAILURE) {
        gpioTerminate(); 
        std::cout << "FAILED TO ACTIVATE!!! exiting program\n";
        return 1;
    }

    if (motor_b.on_activate() == CallbackReturn::FAILURE) {
        gpioTerminate(); 
        std::cout << "FAILED TO ACTIVATE!!! exiting program\n";
        return 1;
    }

    std::cout << "Testing motor A\n";
    test_motor(motor_a);

    std::cout << "Testing motor B\n";
    test_motor(motor_b);

    // Test Motor A and B
    motor_a.on_activate();
    motor_b.on_activate();
    motor_a.set_direction(FORWARD);
    motor_b.set_direction(FORWARD);
    std::this_thread::sleep_for(std::chrono::microseconds(10)); 
    if (motor_a.set_pwm(65) == OK && motor_b.set_pwm(65) == OK) {  
        // sleep for 3 seconds to test motor
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

    // Increase speed motor A and B
    if (motor_a.set_pwm(85) == OK && motor_b.set_pwm(85) == OK) {  
        // sleep for 3 seconds to test motor
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }


    motor_a.on_deactivate();
    motor_b.on_deactivate();

    gpioTerminate(); 
    return 0;
}