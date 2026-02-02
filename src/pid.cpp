#include "pid.hpp"

CallbackReturn PID::on_configure(double kp, double ki, double kd, double output_min, double output_max) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    output_min_ = output_min;
    output_max_ = output_max;
    integral_ = 0.0;
    previous_error_ = 0.0;
    first_run_ = true;
    return CallbackReturn::SUCCESS;
}

CallbackReturn PID::on_activate() {
    // Reset integral and previous error on activation
    integral_ = 0.0;
    previous_error_ = 0.0;
    first_run_ = true;
    return CallbackReturn::SUCCESS;
}

// act as a reset
CallbackReturn PID::on_deactivate() {
    reset();
    return CallbackReturn::SUCCESS;
}

void PID::reset() {
    integral_ = 0.0;
    previous_error_ = 0.0;
    first_run_ = true;   
}

double PID::compute(double setpoint, double measurement, double dt) {
    double error = setpoint - measurement;

        // Proportional
        double p_term = kp_ * error;

        // Integral with anti-windup
        integral_ += error * dt;
        integral_ = std::clamp(integral_, output_min_ / ki_, output_max_ / ki_);
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