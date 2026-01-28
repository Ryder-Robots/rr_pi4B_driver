#!/bin/bash
# Robust dual motor control with error checking

set -e  # Exit on any error

# Pin definitions
ENA_PIN=18; IN1_PIN=23; IN2_PIN=24  # Motor A
ENB_PIN=19; IN3_PIN=22; IN4_PIN=5   # Motor B

echo "=== Dual Motor Test with Error Checking ==="

# Check pigpiod
if ! pgrep pigpiod > /dev/null; then
    echo "Starting pigpiod..."
    sudo pigpiod || { echo "Failed to start pigpiod"; exit 1; }
    sleep 1
fi

# Function to configure pin
config_pin() {
    pigs m $1 1 || { echo "Failed to configure GPIO $1"; exit 1; }
}

# Configure all pins
echo "Configuring pins..."
for pin in $ENA_PIN $IN1_PIN $IN2_PIN $ENB_PIN $IN3_PIN $IN4_PIN; do
    config_pin $pin
done
echo "✅ Pins configured"

# Function to set motor direction
set_motor_direction() {
    local motor=$1
    local direction=$2
    
    if [ "$motor" = "A" ]; then
        if [ "$direction" = "forward" ]; then
            pigs w $IN1_PIN 1; pigs w $IN2_PIN 0
        elif [ "$direction" = "reverse" ]; then
            pigs w $IN1_PIN 0; pigs w $IN2_PIN 1
        else
            pigs w $IN1_PIN 0; pigs w $IN2_PIN 0  # Brake
        fi
    elif [ "$motor" = "B" ]; then
        if [ "$direction" = "forward" ]; then
            pigs w $IN3_PIN 1; pigs w $IN4_PIN 0
        elif [ "$direction" = "reverse" ]; then
            pigs w $IN3_PIN 0; pigs w $IN4_PIN 1
        else
            pigs w $IN3_PIN 0; pigs w $IN4_PIN 0  # Brake
        fi
    fi
}

# Function to set motor speed
set_motor_speed() {
    local motor=$1
    local duty=$2  # 0-100
    local duty_micro=$((duty * 10000))  # Convert to 0-1000000
    
    if [ "$motor" = "A" ]; then
        pigs hp $ENA_PIN 20000 $duty_micro
    elif [ "$motor" = "B" ]; then
        pigs hp $ENB_PIN 20000 $duty_micro
    fi
}

# Test sequence
echo "Test 1: Both motors forward, 65% speed"
set_motor_direction "A" "forward"
set_motor_direction "B" "forward"
set_motor_speed "A" 65
set_motor_speed "B" 65
sleep 3

echo "Test 2: Both motors forward, 85% speed"
set_motor_speed "A" 85
set_motor_speed "B" 85
sleep 3

echo "Test 3: Motor A forward, Motor B reverse"
set_motor_direction "A" "forward"
set_motor_direction "B" "reverse"
sleep 3

echo "Test 4: Stop all motors"
set_motor_speed "A" 0
set_motor_speed "B" 0
set_motor_direction "A" "brake"
set_motor_direction "B" "brake"

echo "✅ All tests complete"