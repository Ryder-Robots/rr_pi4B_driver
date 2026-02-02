#pragma once

#include <cmath>
#include <algorithm>
#include "tst_common.hpp"

/**
 * 
 */
class PID {
    public:
        CallbackReturn on_configure(double kp, double ki, double kd, double output_min, double output_max);

        CallbackReturn on_activate();

        CallbackReturn on_deactivate();

        double compute(double setpoint, double measurement, double dt);

        // called when robot is idle.
        void reset();

    private:
        double kp_, ki_, kd_;
        double output_min_, output_max_;
        double integral_;
        double previous_error_;
        bool first_run_;
        double prev_error_ = 0.0;
};