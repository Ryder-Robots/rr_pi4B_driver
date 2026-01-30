#pragma once

#include <cstddef>
#include <iostream>
#include <pigpio.h> 
#include <thread>
#include <atomic>

#define OK 0
#define LOW 0
#define HIGH 1

enum CallbackReturn {
    SUCCESS,
    FAILURE,
};

enum DIRECTION {
    BACKWARD = LOW,
    FORWARD = HIGH,
};

