#include "encoder.hpp"
#include "motor.hpp"
#include "pid.hpp"
#include "tst_common.hpp"
#include <mutex>
#include <thread>

#define MAX_PPD 4038286 // aproiximate Nm per pulse (approx 5 kph)

#define PWM_A 18
#define DIR_A 23
#define EN_P1_A 9
#define TIMEOUT 0
#define MIN_INTERVAL 150

// 100Hz (100 ms)
#define PID_FREQUENCY 10

// PID
#define KP 0.05
#define KI 0
#define KD 0
#define PID_MIN 0  // aproiximate Nm per pulse (approx 5 kph)
#define PID_MAX 85 // maximum of around 85% of power

#define MIN_DUTY 65

/**
 * This class manages GPIO, For production versions it will be used for all communiation with
 * the GPIOs to ensure that things are done in a standard manner. It will be strictly an
 * action based solution, expecting a return for each command that it can perform. But 
 * for now this will be very stupid.
 */
class GpIoManager
{
  public:
    CallbackReturn on_configure()
    {
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate()
    {
        {
            int rev = gpioHardwareRevision();
            if (rev == 0) {
                std::cout << "pigpiod has an unknown revision\n";
                return CallbackReturn::FAILURE;
            }
            else {
                // This will be printed to the log, but we may want a way of detecting the hardware.
                unsigned model = (rev >> 4) & 0xFF;

                std::cout << "Hardware revision: 0x" << std::hex << rev << std::dec << std::endl;
                std::cout << "Model number: " << model << std::endl;
                std::cout << "Expected model for Pi4B: 17" << std::endl;
            }
        }

        int pi = gpioInitialise();
        if (pi < 0) {
            std::cout << "Failed to connect to pigpiod\n";
            return CallbackReturn::FAILURE;
        }
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate()
    {
        gpioTerminate();
        return CallbackReturn::SUCCESS;
    }
};

class MotorController
{
  public:
    CallbackReturn on_configure(const int pwm_pin,
        const int dir_pin,
        const int en_pin,
        int timeout,
        uint32_t min_interval_us,
        uint32_t pid_frequency_rate,
        double kp,
        double ki,
        double kd,
        double p_min,
        double p_max)
    {
        callback_ = [this](int gpio_pin, uint32_t delta_us, uint32_t tick, TickStatus tick_status) {
            this->encoder_cb_(gpio_pin, delta_us, tick, tick_status);
        };

        // configure motor, pid and encoder.
        if (motor_.on_configure(pwm_pin, dir_pin, 0) == CallbackReturn::FAILURE ||
              encoder_.on_configure(en_pin, callback_, timeout, min_interval_us) == CallbackReturn::FAILURE) {
              callback_ = nullptr;
              return CallbackReturn::FAILURE;
        }
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate() {
        if (motor_.on_activate() != CallbackReturn::SUCCESS) {
            return CallbackReturn::FAILURE;
        }

        // dont rollback, this will call two deactivates for motor, 
        // which may not be what is needed.
        if (encoder_.on_activate() != CallbackReturn::SUCCESS) {
            return CallbackReturn::FAILURE;
        }
        running_.store(true, std::memory_order_release);
        
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate() {
        running_.store(false, std::memory_order_release);
        velocity_.store(0.0, std::memory_order_release);
        auto enc_result = encoder_.on_deactivate();  // stop interrupts first
        auto motor_result = motor_.on_deactivate();  // then stop PWM
        callback_ = nullptr;
        delta_ct_ = 0;
        delta_us_ct_ = 0;
        delta_us_accum_ = 0;

        return (enc_result == CallbackReturn::SUCCESS && motor_result == CallbackReturn::SUCCESS)
            ? CallbackReturn::SUCCESS : CallbackReturn::FAILURE;
    }

    void publish(DIRECTION direction, double duty_cycle) {
        motor_.set_direction(direction);
        motor_.set_pwm(duty_cycle);
    }

    void subscribe() {
        std::cout << "velocity: " << velocity_.load(std::memory_order_acquire) << "\n";
    }

  protected:
    void encoder_cb_(
        int gpio_pin,
        uint32_t delta_us,
        uint32_t tick,
        TickStatus tick_status)
    {
        // only worry about this condition during testing, under
        // production conditions if not healthy the code will need
        // to deal with it, in worse case this could mean stopping.
        //
        // Note that each encoder has two pins, therefore we can use one
        // as redundancy and accept that directon is probally known. So under
        // error conditions it is possilble to reset the en pin to the redundant
        // one.

        (void)gpio_pin;
        (void)tick;

        if (!running_.load(std::memory_order_acquire)) {
            return;
        }
        

        delta_ct_++;
        if (tick_status == TickStatus::HEALTHY && delta_us > MIN_DELTA_US && delta_us < MAX_DELTA_US) {
            // keep it really light only get mutex if something is changing.
            delta_us_ct_++;
            delta_us_accum_ += delta_us;
        }

        if (delta_ct_ >= PPR_) {
            delta_ct_ = 0;
            if (delta_us_accum_ > 0 && delta_us_ct_ > 0) {
                double avg_us = delta_us_accum_ / static_cast<double>(delta_us_ct_);
                velocity_.store(1'000'000.0 / (avg_us * PPR_), std::memory_order_release);
            }
            delta_us_accum_ = 0;
            delta_us_ct_ = 0;
        }
    }

  private:
    // output variables
    std::atomic<double> velocity_ {0}; // velocity per rotation.

    // state variables
    int delta_ct_ {0};     // count for each delta that has arrive (regardless of its in range or not)
    int delta_us_ct_ {0};  // count of healthy delta ticks.
    double delta_us_accum_ {0}; // accumulate deltas

    // Drivers
    MotorEncoder encoder_;
    Motor motor_;

    // callbacks
    EncoderTickCallback callback_ {nullptr};

    // state control
    std::atomic<bool> running_ {false};

    // limit variables
    const double MIN_DELTA_US{300};
    const double MAX_DELTA_US{3000};
    const int    PPR_{8};
};

// class MotorController_1
// {
//   public:
//     CallbackReturn on_configure(const int pwm_pin,
//         const int dir_pin,
//         const int en_pin,
//         int timeout,
//         uint32_t min_interval_us,
//         uint32_t pid_frequency_rate,
//         double kp,
//         double ki,
//         double kd,
//         double p_min,
//         double p_max)
//     {
//         last_pid_tick_ = std::chrono::steady_clock::now();

//         if (pid_frequency_rate <= 0) {
//             // invalid frequency
//             return CallbackReturn::FAILURE;
//         }
//         pid_frequency_rate_ = pid_frequency_rate;

//         callback_ = [this](int gpio_pin, uint32_t delta_us, uint32_t tick, TickStatus tick_status) {
//             this->encoder_cb_(gpio_pin, delta_us, tick, tick_status);
//         };

//         // configure motor, pid and encoder.
//         if (motor_.on_configure(pwm_pin, dir_pin, 0) == CallbackReturn::FAILURE ||
//             encoder_.on_configure(en_pin, callback_, timeout, min_interval_us) == CallbackReturn::FAILURE ||
//             pid_.on_configure(kp, ki, kd, p_min, p_max) == CallbackReturn::FAILURE) {
//             callback_ = nullptr;
//             return CallbackReturn::FAILURE;
//         }
//         // hang onto the PWM pin for publising info
//         pwm_pin_ = pwm_pin;

//         // set motor to '0' (full stop)
//         motor_.set_pwm(0);
//         return CallbackReturn::SUCCESS;
//     }


//     // Perform activation
//     CallbackReturn on_activate()
//     {
//         last_pid_tick_ = std::chrono::steady_clock::now();

//         // activate motor and encoder.
//         if (motor_.on_activate() != CallbackReturn::SUCCESS) {
//             return CallbackReturn::FAILURE;
//         }
//         if (encoder_.on_activate() != CallbackReturn::SUCCESS) {
//             return CallbackReturn::FAILURE;
//         }

//         // setup thread for PID, note that motor is already in a thread using PIGPIO so no
//         // need to create a thread for it.
//         running_ = true;
//         control_thread_ = std::thread(&MotorController::pid_cb_, this);
//         return CallbackReturn::SUCCESS;
//     }

//     void start_motor()
//     {
//         motor_started_ = true;
//     }

//     void stop_motor()
//     {
//         motor_started_ = false;
//         motor_.set_pwm(0);
//     }

//     // Begin shutdown procedure
//     CallbackReturn on_deactivate()
//     {
//         running_ = false; // Atomic, no lock needed

//         if (control_thread_.joinable()) {
//             control_thread_.join();
//         }

//         // Now safe to clean up with lock if needed
//         std::lock_guard<std::mutex> lock(cb_mutex_);
//         motor_.set_pwm(0);
//         callback_ = nullptr;

//         CallbackReturn rt = CallbackReturn::SUCCESS;
//         rt = motor_.on_deactivate();
//         if (rt == CallbackReturn::SUCCESS) {
//             return encoder_.on_deactivate();
//         }
//         encoder_.on_deactivate();
//         return CallbackReturn::FAILURE;
//     }

//     /*
//      * Waits for a external command which may change direction or delta_us, for testing
//      * this is a command but in ROS2 it will be subscribed to a topic.
//      * 
//      * Values wil be recieved from the e_control_unit
//      */
//     void subscribe(double delta_us, DIRECTION direction)
//     {
//         std::lock_guard<std::mutex> lock(cb_mutex_);
//         if (direction != direction_ || delta_us == 0) {
//             pid_.reset();
//             direction_ = direction;
//             motor_.set_direction(direction_);
//         }
//         // this will affectively change the veloicty.
//         target_nm_ = delta_us;
//     }

//     // This will publish feedback to the e_control_unit node.
//     void publish()
//     {
//         // this is minimal, the e_control_unit node will provide feedback that upstream
//         // such as ros2_control can use. Just give the ECU enough to make decisions for multiple
//         // motors.
//         int dc = gpioGetPWMdutycycle(pwm_pin_);
//         std::cout << "duty cycle = " << dc << " direction = " << direction_ << "\n";
//     }

//   private:
//     // feedback
//     int pwm_pin_ = -1;

//     // command variables
//     double target_nm_ = 0;
//     DIRECTION direction_ = DIRECTION::FORWARD;

//     // flow control variables
//     std::thread control_thread_;
//     std::atomic<bool> running_ {false};
//     std::atomic<bool> motor_started_ {false};
//     uint32_t pid_frequency_rate_ = 0; // rate in milliseconds that PID will be executed.

//     // delta_us_accum has last delta_us added to it, for each call of encoder_cb_, this helps
//     // create the mean of deltas_us_ since last PID call.
//     double delta_us_accum_ = 0;
//     uint32_t delta_us_count_ = 0;
//     std::chrono::steady_clock::time_point last_pid_tick_;
//     std::mutex cb_mutex_;

//     EncoderTickCallback callback_ {nullptr};

//     // Current command variables
//     MotorEncoder encoder_;
//     PID pid_;
//     Motor motor_;

//     const double MIN_DELTA_US = 300;
//     const double MAX_DELTA_US = 3000;

//     /*
//      * Connects to encoder and keeps statitics which is used to update
//      * pid
//      */
//     void encoder_cb_(
//         int gpio_pin,
//         uint32_t delta_us,
//         uint32_t tick,
//         TickStatus tick_status)
//     {
//         // only worry about this condition during testing, under
//         // production conditions if not healthy the code will need
//         // to deal with it, in worse case this could mean stopping.
//         //
//         // Note that each encoder has two pins, therefore we can use one
//         // as redundancy and accept that directon is probally known. So under
//         // error conditions it is possilble to reset the en pin to the redundant
//         // one.

//         (void)gpio_pin;
//         (void)tick;
//         if (tick_status == TickStatus::HEALTHY && delta_us > MIN_DELTA_US && delta_us < MAX_DELTA_US) {
//             // keep it really light only get mutex if something is changing.
//             std::lock_guard<std::mutex> lock(cb_mutex_);
//             delta_us_accum_ += delta_us;
//             delta_us_count_++;
//         }
//     }

//     /*
//      * ran within timer to recompute motor duty cycle
//      */
//     void pid_cb_()
//     {
//         while (running_) {
//             if (!motor_started_) {
//                 std::this_thread::sleep_for(std::chrono::milliseconds(pid_frequency_rate_));
//                 continue;
//             }
//             uint32_t measurement = 0;
//             double target = 0;
//             uint32_t delta_us_count = 0;
//             {
//                 std::lock_guard<std::mutex> lock(cb_mutex_);
//                 delta_us_count = delta_us_count_;
//                 measurement = (delta_us_count_ > 0) ? (delta_us_accum_ / delta_us_count_) : 0;
//                 delta_us_accum_ = 0;
//                 delta_us_count_ = 0;
//                 target = target_nm_;
//             }

//             auto now = std::chrono::steady_clock::now();
//             double dt = std::chrono::duration<double>(now - last_pid_tick_).count();
//             last_pid_tick_ = now;

//             if (delta_us_count > 0) {
//                 int duty = pid_.compute(target, measurement, dt);
//                 if (duty < MIN_DUTY) {
//                     duty = MIN_DUTY; // Don't let it drop below spinning threshold
//                 }
//                 motor_.set_pwm(duty);
//             }
//             else {
//                 /**
//                  * If there is no duty cycle at all, set to the minimum that 
//                  */
//                 motor_.set_pwm(MIN_DUTY);
//             }
//             std::this_thread::sleep_for(std::chrono::milliseconds(pid_frequency_rate_));
//         }
//     }
// };


int main()
{
    // start motor controller.
    // MotorController cntl;
    // GpIoManager gpio;

    // std::cout << "configuring robot\n";
    // if (cntl.on_configure(PWM_A, DIR_A, EN_P1_A, TIMEOUT, MIN_INTERVAL, PID_FREQUENCY, KP, KI, KD, PID_MIN, PID_MAX) == CallbackReturn::FAILURE) {
    //     std::cout << "failed on configuration\n";
    //     return 1;
    // }

    // std::cout << "activating robot\n";
    // if (gpio.on_activate() == CallbackReturn::FAILURE) {
    //     std::cout << "ERROR: Failed to initialize hardware\n";
    //     return 1;
    // }
    // if (cntl.on_activate() == CallbackReturn::FAILURE) {
    //     std::cout << "failed on activation\n";
    //     cntl.on_deactivate();
    //     return 1;
    // }

    // std::cout << "spinning motor 65%\n";
    // cntl.start_motor();
    // // run for 5 seconds @50%
    // for (auto i = 0; i < 5; i++) {
    //     cntl.subscribe(1000, DIRECTION::FORWARD);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     cntl.publish();
    // }
    // std::cout << "stopping motor\n";
    // cntl.stop_motor();
    // std::cout << "spinning motor 70%\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // cntl.start_motor();
    // for (auto i = 0; i < 5; i++) {
    //     cntl.subscribe(700, DIRECTION::FORWARD);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     cntl.publish();
    // }
    // std::cout << "spinning motor 80%\n";
    // for (auto i = 0; i < 5; i++) {
    //     cntl.subscribe(400, DIRECTION::FORWARD);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     cntl.publish();
    // }

    // cntl.on_deactivate();
    // gpio.on_deactivate();
    return 0;
}