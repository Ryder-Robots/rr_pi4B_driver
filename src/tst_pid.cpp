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

    // TODO: Ignore PID variables kp, ki, kd, p_min, and p_max for now these will be re-introduced.
    CallbackReturn on_configure(
        const int pwm_pin,
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
        pwm_pin_ = pwm_pin;
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
        std::this_thread::sleep_for(std::chrono::microseconds(100));

        delta_ct_.store(0, std::memory_order_release);
        delta_us_ct_.store(0, std::memory_order_release);
        delta_us_accum_.store(0, std::memory_order_release);
        publish(DIRECTION::FORWARD, 0);

        return (enc_result == CallbackReturn::SUCCESS && motor_result == CallbackReturn::SUCCESS)
            ? CallbackReturn::SUCCESS : CallbackReturn::FAILURE;
    }

    void publish(DIRECTION direction, double duty_cycle) {
        motor_.set_direction(direction);
        motor_.set_pwm(duty_cycle);
    }

    // velocity, duty_cycle, direction
    void subscribe() {
        std::cout << velocity_.load(std::memory_order_acquire) << "," << gpioGetPWMdutycycle(pwm_pin_) << "\n";
    }

  protected:
    void encoder_cb_(
        const int gpio_pin,
        const uint32_t delta_us,
        const uint32_t tick,
        const TickStatus tick_status)
    {
        (void)gpio_pin;
        (void)tick;

        if (!running_.load(std::memory_order_acquire)) {
            return;
        }
        
        // Increment pulse count (returns OLD value)
        int old_delta_ct = delta_ct_.fetch_add(1, std::memory_order_acq_rel);
        int new_delta_ct = old_delta_ct + 1;
        
        // Accumulate valid timing measurements
        if (tick_status == TickStatus::HEALTHY && 
            delta_us > MIN_DELTA_US && 
            delta_us < MAX_DELTA_US) {
            
            delta_us_ct_.fetch_add(1, std::memory_order_acq_rel);
            delta_us_accum_.fetch_add(static_cast<uint64_t>(delta_us), std::memory_order_acq_rel);
        }

        // Check if we've completed a full rotation
        if (new_delta_ct >= PPR_) {
            int expected = new_delta_ct;

            if (delta_ct_.compare_exchange_strong(expected, 0,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire)) {
                // Load accumulated values once
                uint64_t accum = delta_us_accum_.load(std::memory_order_acquire);
                int ct = delta_us_ct_.load(std::memory_order_acquire);
                
                // Reset counters for next rotation
                delta_ct_.store(0, std::memory_order_release);
                delta_us_ct_.store(0, std::memory_order_release);
                delta_us_accum_.store(0, std::memory_order_release);
                
                // Calculate velocity if we have valid measurements
                if (accum > 0 && ct > 0) {
                    double avg_us = static_cast<double>(accum) / static_cast<double>(ct);
                    velocity_.store(1'000'000.0 / (avg_us * PPR_), std::memory_order_release);
                }
            }
        }
    }

  private:
    // output variables
    std::atomic<double> velocity_ {0}; // velocity per rotation.
    
    // state variables
    std::atomic<int> delta_ct_ {0};  // count for each delta that has arrive (regardless of its in range or not)
    std::atomic<int> delta_us_ct_{0};    // count of healthy delta ticks.
    std::atomic<uint64_t> delta_us_accum_{0}; // accumulate deltas
    int pwm_pin_ = -1;

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


int main()
{
    // start motor controller
    MotorController cntl;
    GpIoManager gpio;

    std::cout << "configuring robot\n";
    if (cntl.on_configure(PWM_A, DIR_A, EN_P1_A, TIMEOUT, MIN_INTERVAL, PID_FREQUENCY, KP, KI, KD, PID_MIN, PID_MAX) == CallbackReturn::FAILURE) {
        std::cout << "failed on configuration\n";
        return 1;
    }

    // std::cout << "activating robot\n";
    if (gpio.on_activate() == CallbackReturn::FAILURE) {
        std::cout << "ERROR: Failed to initialize hardware\n";
        return 1;
    }
    if (cntl.on_activate() == CallbackReturn::FAILURE) {
        std::cout << "failed on activation\n";
        gpio.on_deactivate();
        cntl.on_deactivate();
        return 1;
    }

    std::cout << "spinning motor 65%\n";
    // run for 5 seconds @65%
    for (auto i = 0; i < 8; i++) {
        cntl.publish(DIRECTION::FORWARD, 65);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cntl.subscribe();
    }
    cntl.publish(DIRECTION::FORWARD, 0);

    cntl.on_deactivate();
    gpio.on_deactivate();
    return 0;
}