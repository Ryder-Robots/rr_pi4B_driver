#!/bin/bash

pin_under_test=${1}

pgrep pigpiod || sudo pigpiod

# 2. Set GPIO 18 as output
pigs m ${pin_under_test} 1
echo "Set as output: $?"

# 3. Write HIGH
pigs w ${pin_under_test} 1
echo "Write HIGH: $?"

# 4. Read back value
pigs r ${pin_under_test}
# Should return: 1

# 5. Write LOW
pigs w ${pin_under_test} 0
echo "Write LOW: $?"

# 6. Read back value
pigs r ${pin_under_test}
# Should return: 0

# 7. Toggle test (repeat 5 times)
for i in {1..5}; do
    pigs w ${pin_under_test} 1
    sleep 0.5
    pigs w ${pin_under_test} 0
    sleep 0.5
    echo "Toggle $i complete"
done