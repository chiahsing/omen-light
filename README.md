# omen-light
A command line tool to control LED lights for HP Omen machines


# Build
Install hidapi

```
sudo apt install libhidapi-dev
```

then run:
```
g++ -o omen_light omen_light.cc -lhidapi-libusb
```

# Run
Run the tool to see the usage. Run the tool as root to change the settings.

# Risk
This is not an official app, and is only tested on my own machine. It may damage the device. Use at your own risk.

# Credit
Inspired by u/ravicc's prior work: https://www.reddit.com/r/HPOmen/comments/nirgwa/a_way_to_change_hp_omen_lighting_with_a_python/
