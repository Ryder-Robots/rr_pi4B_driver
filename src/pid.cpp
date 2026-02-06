#include "pid.hpp"

// 2600µs per pulse corresponds to 3 m/s. This should the max speed. We want around half of that for turning.

CallbackReturn PID::on_configure(double kp, double ki, double kd, double output_min, double output_max)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    output_min_ = output_min;
    output_max_ = output_max;
    integral_ = 0.0;
    prev_error_ = 0.0;
    first_run_ = true;
    return CallbackReturn::SUCCESS;
}

CallbackReturn PID::on_activate()
{
    // Reset integral and previous error on activation
    integral_ = 0.0;
    prev_error_ = 0.0;
    first_run_ = true;
    return CallbackReturn::SUCCESS;
}

// act as a reset
CallbackReturn PID::on_deactivate()
{
    reset();
    return CallbackReturn::SUCCESS;
}

void PID::reset()
{
    integral_ = 0.0;
    prev_error_ = 0.0;
    first_run_ = true;
}

/**
 * @fn compute
 * 
 * Compute will be called to control the motor, when jitter or another outcome occurs.
 * 
 * @param setpoint target nm per pulse
 * @param measurement current nm per pulse
 * @param dt time delta since last goal. This is a fixed rate, or close enough to it set by
 *           ROS2 timer which is hopefully using the internal clock.
 * 
 *           For ros this would mean 
 *           // 100 Hz control loop
 *            timer_ = create_wall_timer(
 *                  std::chrono::milliseconds(10),
 *                std::bind(&MotorController::control_loop, this)
 *              );
 *            actual time will be a calculation on each cycle to allow for some jitter.
 * @return duty cycles to achieve outcome.
 */
double PID::compute(double setpoint, double measurement, double dt)
{
    /**
     * Error sign convention: We're controlling pulse period (time between pulses),
     * not velocity directly. As velocity increases, pulse period DECREASES.
     * Therefore: error = measurement - setpoint
     * 
     * - If measurement > setpoint: pulses are too slow (motor too slow) → positive error → increase output
     * - If measurement < setpoint: pulses are too fast (motor too fast) → negative error → decrease output
     */
    double error = measurement - setpoint;

    // Proportional
    double p_term = kp_ * error;

    // Integral with anti-windup
    integral_ += error * dt;
    if (ki_ != 0) {
        integral_ = std::clamp(integral_, output_min_ / ki_, output_max_ / ki_);
    }
    double i_term = ki_ * integral_;

    // Derivative (on error, with first-run protection)
    double d_term = 0.0;
    if (!first_run_ && dt > 0.0) {
        d_term = kd_ * (error - prev_error_) / dt;
    }
    first_run_ = false;
    prev_error_ = error;

    // Sum and clamp output
    double output = p_term + i_term + d_term;
    return std::clamp(output, output_min_, output_max_);
}