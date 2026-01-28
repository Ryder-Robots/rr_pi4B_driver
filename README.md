# rr_pi4B_driver
low level driver tests for Pi4B revision 1.5


## Installation

reference: http://abyz.me.uk/rpi/pigpio/download.html

```bash
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
sudo apt install python-setuptools python3-setuptools
```

## Inital Testing

### x_pigpio

```bash
sudo ./x_pigpio # check C I/F

```
Results:

```
Testing pigpio C I/F
pigpio version 79.
Hardware revision 10498325.
Mode/PUD/read/write tests.
TEST  1.1  PASS (set mode, get mode: 0)
TEST  1.2  PASS (set pull up down, read: 1)
TEST  1.3  PASS (set pull up down, read: 0)
TEST  1.4  PASS (write, get mode: 1)
TEST  1.5  PASS (read: 0)
TEST  1.6  PASS (write, read: 1)
PWM dutycycle/range/frequency tests.
TEST  2.1  PASS (set PWM range, set/get PWM frequency: 10)
TEST  2.2  PASS (get PWM dutycycle: 0)
TEST  2.3  PASS (set PWM dutycycle, callback: 0)
TEST  2.4  PASS (get PWM dutycycle: 128)
TEST  2.5  PASS (set PWM dutycycle, callback: 40)
TEST  2.6  PASS (set/get PWM frequency: 100)
TEST  2.7  PASS (callback: 400)
TEST  2.8  PASS (set/get PWM frequency: 1000)
TEST  2.9  PASS (callback: 4000)
TEST  2.10 PASS (get PWM range: 255)
TEST  2.11 PASS (get PWM real range: 200)
TEST  2.12 PASS (set/get PWM range: 2000)
TEST  2.13 PASS (get PWM real range: 200)
PWM/Servo pulse accuracy tests.
TEST  3.1  PASS (get servo pulsewidth: 500)
TEST  3.2  PASS (set servo pulsewidth: 40000)
TEST  3.3  PASS (get servo pulsewidth: 1500)
TEST  3.4  PASS (set servo pulsewidth: 13333)
TEST  3.5  PASS (get servo pulsewidth: 2500)
TEST  3.6  PASS (set servo pulsewidth: 8000)
TEST  3.7  PASS (set/get PWM frequency: 1000)
TEST  3.8  PASS (set PWM range: 200)
TEST  3.9  PASS (get PWM dutycycle: 20)
TEST  3.10 PASS (set PWM dutycycle: 200)
TEST  3.11 PASS (get PWM dutycycle: 40)
TEST  3.12 PASS (set PWM dutycycle: 400)
TEST  3.13 PASS (get PWM dutycycle: 60)
TEST  3.14 PASS (set PWM dutycycle: 600)
TEST  3.15 PASS (get PWM dutycycle: 80)
TEST  3.16 PASS (set PWM dutycycle: 800)
Pipe notification tests.
TEST  4.1  PASS (notify open/begin: 0)
TEST  4.2  PASS (notify pause: 0)
TEST  4.3  PASS (notify close: 0)
TEST  4.4  PASS (sequence numbers ok: 1)
TEST  4.5  PASS (gpio toggled ok: 1)
TEST  4.6  PASS (number of notifications: 80)
Waveforms & serial read/write tests.
TEST  5.1  PASS (callback, set mode, wave clear: 0)
TEST  5.2  PASS (pulse, wave add generic: 4)
TEST  5.3  PASS (wave tx repeat: 9)
TEST  5.4  PASS (callback: 50)
TEST  5.5  PASS (wave tx stop: 0)
TEST  5.6  PASS (serial read open: 0)
TEST  5.7  PASS (wave clear, wave add serial: 3405)
TEST  5.8  PASS (wave tx start: 6811)
TEST  5.9  PASS (NOT APPLICABLE: 0)
TEST  5.10 PASS (NOT APPLICABLE: 0)
TEST  5.11 PASS (wave tx busy, serial read: 0)
TEST  5.12 PASS (serial read close: 0)
TEST  5.13 PASS (wave get micros: 6158148)
TEST  5.14 PASS (wave get high micros: 6158148)
TEST  5.15 PASS (wave get max micros: 1800000000)
TEST  5.16 PASS (wave get pulses: 3405)
TEST  5.17 PASS (wave get high pulses: 3405)
TEST  5.18 PASS (wave get max pulses: 12000)
TEST  5.19 PASS (wave get cbs: 6810)
TEST  5.20 PASS (wave get high cbs: 6810)
TEST  5.21 PASS (wave get max cbs: 25016)
TEST  5.22 PASS (wave create pad, count==1, wid==: 0)
TEST  5.23 PASS (wave create pad, count==2, wid==: 1)
TEST  5.24 PASS (delete wid==0 success: 0)
TEST  5.25 PASS (No more CBs using wave create: -67)
TEST  5.26 PASS (wave create pad, count==3, wid==: 0)
TEST  5.27 PASS (wave chain [1,0]: 0)
TEST  5.28 PASS (callback count==: 5)
Trigger tests
TEST  6.1  PASS (gpio trigger count: 5)
TEST  6.2  PASS (gpio trigger pulse length: 150)
Watchdog tests.
TEST  7.1  PASS (set watchdog on count: 39)
TEST  7.2  PASS (set watchdog off count: 0)
Bank read/write tests.
TEST  8.1  PASS (read bank 1: 0)
TEST  8.2  PASS (read bank 1: 33554432)
TEST  8.3  PASS (clear bank 1: 0)
TEST  8.4  PASS (set bank 1: 1)
TEST  8.5  PASS (read bank 2: 0)
TEST  8.6  PASS (clear bank 2: 0)
TEST  8.7  PASS (NOT APPLICABLE: 0)
TEST  8.8  PASS (set bank 2: 0)
TEST  8.9  PASS (NOT APPLICABLE: 0)
Script store/run/status/stop/delete tests.
TEST  9.1  PASS (store/run script: 100)
TEST  9.2  PASS (run script/script status: 201)
TEST  9.3  PASS (run/stop script/script status: 110)
TEST  9.4  PASS (delete script: 0)
```
### x_pigpiod_if2

```
sudo pigpiod    # start daemon
./x_pigpiod_if2 # check C      I/F to daemon
```

Result

```
Connected to pigpio daemon (0).

Testing pigpiod C I/F 2
pigpio version 79.
Hardware revision 10498325.
Mode/PUD/read/write tests.
TEST  1.1  PASS (set mode, get mode: 0)
TEST  1.2  PASS (set pull up down, read: 1)
TEST  1.3  PASS (set pull up down, read: 0)
TEST  1.4  PASS (write, get mode: 1)
TEST  1.5  PASS (read: 0)
TEST  1.6  PASS (write, read: 1)
TEST  1.7  PASS (pigpio_start with non-default arguments: 31)
PWM dutycycle/range/frequency tests.
TEST  2.1  PASS (set PWM range, set/get PWM frequency: 10)
TEST  2.2  PASS (get PWM dutycycle: 0)
TEST  2.3  PASS (set PWM dutycycle, callback: 0)
TEST  2.4  PASS (get PWM dutycycle: 128)
TEST  2.5  PASS (set PWM dutycycle, callback: 40)
TEST  2.6  PASS (set/get PWM frequency: 100)
TEST  2.7  PASS (callback: 400)
TEST  2.8  PASS (set/get PWM frequency: 1000)
TEST  2.9  PASS (callback: 4000)
TEST  2.10 PASS (get PWM range: 255)
TEST  2.11 PASS (get PWM real range: 200)
TEST  2.12 PASS (set/get PWM range: 2000)
TEST  2.13 PASS (get PWM real range: 200)
PWM/Servo pulse accuracy tests.
TEST  3.1  PASS (get servo pulsewidth: 500)
TEST  3.2  PASS (set servo pulsewidth: 40000)
TEST  3.3  PASS (get servo pulsewidth: 1500)
TEST  3.4  PASS (set servo pulsewidth: 13333)
TEST  3.5  PASS (get servo pulsewidth: 2500)
TEST  3.6  PASS (set servo pulsewidth: 8000)
TEST  3.7  PASS (set/get PWM frequency: 1000)
TEST  3.8  PASS (set PWM range: 200)
TEST  3.9  PASS (get PWM dutycycle: 20)
TEST  3.10 PASS (set PWM dutycycle: 200)
TEST  3.11 PASS (get PWM dutycycle: 40)
TEST  3.12 PASS (set PWM dutycycle: 400)
TEST  3.13 PASS (get PWM dutycycle: 60)
TEST  3.14 PASS (set PWM dutycycle: 600)
TEST  3.15 PASS (get PWM dutycycle: 80)
TEST  3.16 PASS (set PWM dutycycle: 800)
Pipe notification tests.
TEST  4.1  PASS (notify open/begin: 0)
TEST  4.2  PASS (notify pause: 0)
TEST  4.3  PASS (notify close: 0)
TEST  4.4  PASS (sequence numbers ok: 1)
TEST  4.5  PASS (gpio toggled ok: 1)
TEST  4.6  PASS (number of notifications: 80)
Waveforms & serial read/write tests.
TEST  5.1  PASS (callback, set mode, wave clear: 0)
TEST  5.2  PASS (pulse, wave add generic: 4)
TEST  5.3  PASS (wave tx repeat: 9)
TEST  5.4  PASS (callback: 50)
TEST  5.5  PASS (wave tx stop: 0)
TEST  5.6  PASS (serial read open: 0)
TEST  5.7  PASS (wave clear, wave add serial: 3405)
TEST  5.8  PASS (wave tx start: 6811)
TEST  5.9  PASS (callback: 0)
TEST  5.10 PASS (wave tx busy, callback: 1702)
TEST  5.11 PASS (wave tx busy, serial read: 0)
TEST  5.12 PASS (serial read close: 0)
TEST  5.13 PASS (wave get micros: 6158148)
TEST  5.14 PASS (wave get high micros: 6158148)
TEST  5.15 PASS (wave get max micros: 1800000000)
TEST  5.16 PASS (wave get pulses: 3405)
TEST  5.17 PASS (wave get high pulses: 3405)
TEST  5.18 PASS (wave get max pulses: 12000)
TEST  5.19 PASS (wave get cbs: 6810)
TEST  5.20 PASS (wave get high cbs: 6810)
TEST  5.21 PASS (wave get max cbs: 25016)
TEST  5.22 PASS (wave create pad, count==1, wid==: 0)
TEST  5.23 PASS (wave create pad, count==2, wid==: 1)
TEST  5.24 PASS (delete wid==0 success: 0)
TEST  5.25 PASS (No more CBs using wave create: -67)
TEST  5.26 PASS (wave create pad, count==3, wid==: 0)
TEST  5.27 PASS (wave chain [1,0]: 0)
TEST  5.28 PASS (callback count==: 5)
Trigger tests.
TEST  6.1  PASS (gpio trigger count: 5)
TEST  6.2  PASS (gpio trigger pulse length: 150)
Watchdog tests.
TEST  7.1  PASS (set watchdog on count: 39)
TEST  7.2  PASS (set watchdog off count: 0)
Bank read/write tests.
TEST  8.1  PASS (read bank 1: 0)
TEST  8.2  PASS (read bank 1: 33554432)
TEST  8.3  PASS (clear bank 1: 0)
TEST  8.4  PASS (set bank 1: 1)
TEST  8.5  PASS (read bank 2: 0)
TEST  8.6  PASS (clear bank 2: 0)
TEST  8.7  PASS (clear bank 2: -42)
TEST  8.8  PASS (set bank 2: 0)
TEST  8.9  PASS (set bank 2: -42)
Script store/run/status/stop/delete tests.
TEST  9.1  PASS (store/run script: 100)
TEST  9.2  PASS (run script/script status: 201)
TEST  9.3  PASS (run/stop script/script status: 410)
TEST  9.4  PASS (delete script: 0)
```