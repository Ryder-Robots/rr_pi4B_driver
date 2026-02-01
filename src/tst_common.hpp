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

enum CallbackReturn : int {
    SUCCESS = 0,
    FAILURE = 1,
};
