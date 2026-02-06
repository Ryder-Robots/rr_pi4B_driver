/**
 * Base connection to hardware, this code simply reads the delta in microsecond timestamps
 * for each pulse. This is done using harware clock.
 *
 * Note for multi-phase encoders such as the ones provided by DFRobot FIT04
 */


#pragma once

#include "tst_common.hpp"
#include <functional>
#include <cstdint>

/**
 * Encoder event status codes
 */
enum class TickStatus : uint8_t {
    HEALTHY = 0,          // Valid rising edge detected
    TIMEOUT = 1,          // No edge within configured timeout period
    NOISE_REJECTED = 2,   // Edge rejected (interval too short, likely electrical noise)
    UNEXPECTED = 3,       // Condition occurred that was unexpected, this should be treated immeidate termination.
};



/**
 * Callback invoked by MotorEncoder on each hardware event (pulse or timeout).
 * 
 * Executed in interrupt context - keep processing minimal and avoid blocking operations.
 * 
 * @param gpio_pin  GPIO pin that sample is taken from.
 * @param delta_us Time elapsed since the last valid pulse in microseconds.
 *                 For OK status: time between valid pulses (use for velocity calculation)
 *                 For TIMEOUT status: time since last valid pulse to timeout
 *                 For NOISE_REJECTED status: the rejected (too-short) interval
 * 
 * @param tick   Current tick.
 * 
 * @param tick_status Event status indicating the nature of this callback:
 *                    - TickStatus::HEALTHY: Valid rising edge detected on encoder phase
 *                    - TickStatus::TIMEOUT: No pulse received within configured timeout period
 *                    - TickStatus::NOISE_REJECTED: Pulse rejected (interval shorter than physical limits)
 * 
 * Note: The encoder reports all events neutrally. Application logic must interpret
 * whether a timeout represents a fault condition based on expected motion state.
 */
using EncoderTickCallback = std::function<void(
    int gpio_pin,
    uint32_t delta_us,
    uint32_t tick,
    TickStatus tick_status
)>;


class MotorEncoder {
    public:

    /**
     * set initial pin and creates the initial tick.
     * 
     * @param pin, pin that will be used to detect phase.
     * @param 
     */
    CallbackReturn on_configure(uint pin, EncoderTickCallback tick_cb, int timeout, uint32_t min_interval_us);

    /**
     * Activates callback algorithm. on_activate must check that tick_cb has been defined,
     * before it can be activate, if it has not or pin is not set then it return an error.
     */
    CallbackReturn on_activate();

    /**
     * resets encoder, so that robot can be cleanly shutdown.
     */
    CallbackReturn on_deactivate();
      

    private:
      /**
       * Called after each pulse. This method will trigger tick_cb which will trigger handler.
       */
      static void gpio_isr_func(int gpio, int level, uint32_t tick, void *userdata);

      void handle_interrupt(int gpio, int level, uint32_t tick);

      // last tick, this should be set during configuration for initial tick.
      uint32_t last_tick_{0};
      int pin_{-1};
      int timeout_{0};
      EncoderTickCallback tick_cb_{nullptr};
      uint32_t min_interval_us_{0};

      int expected_level_ = RISING_EDGE;
};