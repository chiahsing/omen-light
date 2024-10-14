#include <hidapi/hidapi.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

constexpr uint16_t VENDOR_ID = 0x103c;
constexpr uint16_t PRODUCT_ID = 0x84fd;
constexpr uint8_t VERSION = 0x12;
constexpr uint8_t MAX_BRIGHTNESS = 100;
constexpr uint8_t N_LEDS = 8;

struct __attribute__((packed)) Report {
  enum class Mode : uint8_t { STATIC = 1, OFF = 5, BREATHING, CYCLE, BLINKING };

  enum class Type : uint8_t {
    OFF = 0,
    STATIC = 2,
    CHANGING = 10,
  };

  enum class Power : uint8_t { ON = 1, SUSPEND = 2 };

  enum class Theme : uint8_t {
    OFF = 0,
    CUSTOM = 0,
    GALAXY,
    VOLCANO,
    JUNGO,
    OCEAN
  };

  enum class Speed : uint8_t { OFF = 0, SLOW, MEDIUM, FAST };

  struct __attribute__((packed)) Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  uint8_t report_id;  // always 0
  uint8_t reserved;
  uint8_t version;
  Mode mode;
  uint8_t custom_color_count;  // total number of custom colors
  uint8_t custom_color_id;     // the custom color to set. Starting with 1
  uint8_t reserved2[2];
  Color colors[N_LEDS];
  uint8_t reserved3[40 - N_LEDS * 3];
  uint8_t brightness;  // 0-100
  Type type;
  uint8_t reserved4[4];
  uint8_t led;
  Power power;
  Theme theme;
  Speed speed;
};

void ShowUsage(char* arg0) {
  std::cout << R"Usage(
Usage:
  omen_light <led> <power> <mode> [options..]

  led:   the led module to control. Can be 'front', 'chase' or a number from 1 to 8.
  power: the power state to which the setting is applied. Can be 'on', 'suspend'
  mode:  color mode. Can be 'off', 'static', 'breathing', 'cycle', 'blinking'
  
Options for static mode:
  omen_light <led> <power> static <r> <g> <b>

    r, g, b: the value of red, green, and blue. From 0 to 255. 

Options for breathing, cycle, and blinking mode:
  omen_light <led> <power> breathing|... <speed> <theme> [<r> <g> <b>]...

    speed: the color changing speed. Can be 'slow', 'medium', 'fast.
    theme: the theme of the colors. Can be 'galaxy, volcano', 'jungo', 'ocean, or 'custom'.
           For custom theme, it needs to be followed by 1 to 4 sets of r, g, b values.

Example:

# Turn off the chase led when the power is on.
$ omen_light chase on off

# Set the front led to breath in volcano theme slowly when suspend
$ omen_light front suspend breathing slow galaxy

# Set the front led to blink between red and blue when on
$ omen_light front on blinking medium custom 255 0 0 0 0 255

# Set led 4 to a static green color.
$ omen_light 4 on static 0 255 0

)Usage";
}

bool SendReport(const Report& report) {
  if (hid_init()) {
    std::cerr << "hid_init() failed" << std::endl;
    return false;
  }

  hid_device* handle = hid_open(VENDOR_ID, PRODUCT_ID, nullptr);
  if (!handle) {
    std::cerr << "hid_open() failed" << std::endl;
    return false;
  }

  auto* report_ptr = reinterpret_cast<const uint8_t*>(&report);
  if (hid_write(handle, report_ptr, sizeof(report)) < 0) {
    std::cerr << "hid_write() failed" << std::endl;
    return false;
  }

  hid_close(handle);

  if (hid_exit()) {
    std::cerr << "hid_exit() failed" << std::endl;
    return false;
  }

  return true;
}

bool ParseLed(const std::string& arg, uint8_t* led) {
  if (arg == "front") {
    *led = 1;
  } else if (arg == "chase") {
    *led = 2;
  } else {
    try
    {
      *led = std::stoi(arg);
      if ((*led < 1) || (*led > N_LEDS))
      {
        std::cerr << "led number out of range: " << arg << std::endl;
        return false;
      }
    }
    catch(const std::exception& e)
    {
      // probably not an int, move on
      std::cerr << "unknown led: " << arg << std::endl;
      return false;
    }
  }
  return true;
}

bool ParsePower(const std::string& arg, Report::Power* power) {
  if (arg == "on") {
    *power = Report::Power::ON;
  } else if (arg == "suspend") {
    *power = Report::Power::SUSPEND;
  } else {
    std::cerr << "unknown power: " << arg << std::endl;
    return false;
  }
  return true;
}

bool ParseMode(const std::string& arg, Report::Mode* mode) {
  if (arg == "off") {
    *mode = Report::Mode::OFF;
  } else if (arg == "static") {
    *mode = Report::Mode::STATIC;
  } else if (arg == "breathing") {
    *mode = Report::Mode::BREATHING;
  } else if (arg == "cycle") {
    *mode = Report::Mode::CYCLE;
  } else if (arg == "blinking") {
    *mode = Report::Mode::BLINKING;
  } else {
    std::cerr << "unknown mode: " << arg << std::endl;
    return false;
  }
  return true;
}

void ParseColor(char** argv, Report* report) {
  Report::Color* c = nullptr;
  c = &report->colors[report->led - 1];

  // Don't care about parsing error.
  c->r = std::atoi(argv[0]);
  c->g = std::atoi(argv[1]);
  c->b = std::atoi(argv[2]);
}

bool ParseSpeed(const std::string& arg, Report::Speed* speed) {
  if (arg == "slow") {
    *speed = Report::Speed::SLOW;
  } else if (arg == "medium") {
    *speed = Report::Speed::MEDIUM;
  } else if (arg == "fast") {
    *speed = Report::Speed::FAST;
  } else {
    std::cerr << "unknown speed: " << arg << std::endl;
    return false;
  }
  return true;
}

bool ParseTheme(const std::string& arg, Report::Theme* theme) {
  if (arg == "custom") {
    *theme = Report::Theme::CUSTOM;
  } else if (arg == "galaxy") {
    *theme = Report::Theme::GALAXY;
  } else if (arg == "volcano") {
    *theme = Report::Theme::VOLCANO;
  } else if (arg == "jungo") {
    *theme = Report::Theme::JUNGO;
  } else if (arg == "ocean") {
    *theme = Report::Theme::OCEAN;
  } else {
    std::cerr << "unknown theme: " << arg << std::endl;
    return false;
  }
  return true;
}

bool Run(int argc, char** argv) {
  Report report{};  // zero-initialized.
  report.version = VERSION;

  if (argc < 4) return false;
  if (!ParseLed(argv[1], &report.led)) return false;
  if (!ParsePower(argv[2], &report.power)) return false;
  if (!ParseMode(argv[3], &report.mode)) return false;

  argc -= 4, argv += 4;  // consume arguments
  switch (report.mode) {
    case Report::Mode::OFF:
      return SendReport(report);

    case Report::Mode::STATIC:
      if (argc != 3) return false;
      ParseColor(argv, &report);
      report.custom_color_count = 1;
      report.custom_color_id = 1;
      report.brightness = MAX_BRIGHTNESS;
      report.type = Report::Type::STATIC;
      return SendReport(report);

    case Report::Mode::BREATHING:
    case Report::Mode::CYCLE:
    case Report::Mode::BLINKING:
      if (argc < 2) return false;
      if (!ParseSpeed(argv[0], &report.speed)) return false;
      if (!ParseTheme(argv[1], &report.theme)) return false;

      report.brightness = MAX_BRIGHTNESS;
      report.type = Report::Type::CHANGING;

      if (report.theme == Report::Theme::CUSTOM) {
        // custom theme
        argc -= 2, argv += 2;  // consume arguments
        if (argc != 3 && argc != 6 && argc != 9 && argc != 12) return false;
        report.custom_color_count = argc / 3;
        for (int i = 0; i < report.custom_color_count; ++i) {
          report.custom_color_id = i + 1;
          ParseColor(argv + i * 3, &report);
          if (!SendReport(report)) return false;
        }
        return true;
      } else {
        // pre-defined theme
        report.custom_color_count = 1;
        report.custom_color_id = 1;
        return SendReport(report);
      }
      break;
  }

  return true;
}

int main(int argc, char** argv) {
  if (!Run(argc, argv)) {
    ShowUsage(argv[0]);
    return 1;
  }
  return 0;
}
