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



#include <cstddef>
#include <iostream>
#include <pigpio.h> 
#include <thread>

#define OK 0
#define LOW 0
#define HIGH 1
#define PWM_A 18
#define PWM_B 19
#define DIR_A 23
#define DIR_B 24

enum CallbackReturn {
    SUCCESS,
    FAILURE,
};

enum DIRECTION {
    BACKWARD = LOW,
    FORWARD = HIGH,
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
    CallbackReturn on_configure(uint pwm_pin, uint dir_pin, int pi) {

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
    CallbackReturn on_activate() {

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

    CallbackReturn on_deactivate() {
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
    int set_pwm(int duty) {
        return set_pwm(freq_, duty);
    }

    
    CallbackReturn set_direction(DIRECTION dir) {
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
    int freq_ = 1000; // hardware real range for PWM pin 
    
    // Keep the range of the between 0-100, if its found that more granulatity is needed this can always
    // be scaled down to give more range, say 0-1000 etc.
    const int DUTY_OFFSET = 10000;

    int set_pwm(int freq, int duty) {
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

    int set_mode_internal(uint pin, uint mode) {
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
};


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