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
      int pin_{0};
      int timeout_{0};
      EncoderTickCallback tick_cb_{nullptr};
      uint32_t min_interval_us_{0};
      void isr_wrapper();
};