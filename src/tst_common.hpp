#pragma once

#include <cstddef>
#include <iostream>
#include <pigpio.h> 
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint> 

#ifndef OK
#define OK 0
#endif

#ifndef LOW
#define LOW 0
#endif

#ifndef HIGH
#define HIGH 1
#endif

enum CallbackReturn {
    SUCCESS,
    FAILURE,
};

enum DIRECTION {
    BACKWARD = 0,
    FORWARD = 1,
};

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
 * @param delta_us Time elapsed since the last valid pulse in microseconds.
 *                 For OK status: time between valid pulses (use for velocity calculation)
 *                 For TIMEOUT status: time since last valid pulse to timeout
 *                 For NOISE_REJECTED status: the rejected (too-short) interval
 * 
 * @param timestamp_us Absolute hardware timestamp in microseconds from system clock.
 *                     Note: 32-bit value wraps approximately every 71.6 minutes.
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
    uint32_t timestamp_us,
    TickStatus tick_status
)>;
