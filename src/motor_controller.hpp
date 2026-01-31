#pragma once


/**
 * Keeps record of velocity of motors.
 * When activated will begin record keeping, reporting pulses, and time duration in ms. 
 */
class MotorEncoder {
    public:

    /**
     * Get encoder counts since last call and reset counter
     * Useful for position tracking
     */
    int get_counts_reset();

    /**
     * Get current velocity in pulses per second 
     * Updated on every encoder edge for responsiveness
     *
     * signed for direction.
     */
    double velocity_pps() const;


    /**
     * Get current angular velocity in rad/s (signed for direction)
     * Calculated as: (velocity_pps / ppr) * 2π
     */
    double angular_velocity() const;

    /**
    * Get time since last encoder pulse in microseconds
    * Useful for detecting stalls (returns large value if stopped)
    */
    uint32_t time_since_last_pulse() const;

    /**
     * Check if encoder appears to be stalled
     * Returns true if no pulses detected for timeout_ms
     */
    bool is_stalled(uint32_t timeout_ms = 100) const;
    
    /**
     * Reset all counters and state
     */
    void reset();

    private:

    int ppr_ = 0;     // pulses per revolution
    int pin_a_ = 0;   // first pin hit if spinning clockwise
    int pin_b_ = 0;   // second pin hit if spinning counter-clockwise

    std::atomic<int> total_counts_{0};     // Cumulative count (signed)
    std::atomic<double> velocity_pps_{0.0}; // Current velocity (pulses/sec)
    std::atomic<uint32_t> last_tick_{0};    // Last pulse timestamp
    std::atomic<uint8_t> last_state_{0};    // Previous AB state
    
    // Quadrature decoding lookup table
    static const int8_t quadrature_table_[16];
    
    // Static callback for pigpio
    static void encoder_callback_a(int gpio, int level, uint32_t tick);
    static void encoder_callback_b(int gpio, int level, uint32_t tick);
    
    // Instance callback
    void processEncoder(uint32_t tick);
};

