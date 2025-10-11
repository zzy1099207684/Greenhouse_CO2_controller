# Greenhouse CO₂ Controller

This project is a greenhouse CO₂ controller that maintains air quality by regulating CO₂ levels. It reads CO₂, temperature, humidity, and pressure sensors, stores a user-set CO₂ target in memory, and controls a fan and CO₂ valve accordingly. Data is uploaded to the cloud (ThingSpeak) for monitoring and remote adjustments. The project is based on Raspberry Pi Pico W (RP2040) and FreeRTOS. 

## Features

**CO₂ Control**: 

Automatic CO₂ control based on state machine. With hysteresis deadband (by default 50 ppm) to prevent oscillation. The deadband and other control parameters are configurable (see Configuration section). 

**Emergency Ventilation:** 

When CO₂ concentration exceeds critical threshold (default 2000 ppm), the system enters emergency mode, which immediately closes the CO₂ valve, activates maximum fan speed, and displays warning on the screen. This safety feature protects from situations such as valve malfunction or CO₂ leakage.

**Environmental Monitoring**: 

In addition to CO₂ levels, it also monitors temperature, and relative humidity via Modbus RTU communication protocol. All sensor data is collected, displayed on the local OLED interface, and uploaded to the cloud for remote monitoring.

**Local User Interface**: 

OLED display (128×64 SSD1306) with rotary encoder and three function buttons, providing local control for viewing sensor readings, adjusting CO₂ setpoints and configuring WiFi settings.

**Configuration Storage**: 

Persistent storage (EEPROM) that saves CO₂ setpoint values and WiFi network settings, which are auto reloaded after system restarts.

**ThingSpeak Integration**: 

ThingSpeak IoT integration, allowing real-time data visualization, and remote setpoint setting. The system periodically uploads sensor readings (CO₂, temperature, humidity, fan speed) and retrieves CO₂ setpoint changes from the cloud.

**Active Ventilation Mode**: 

Optional active ventilation mode for more precise CO₂ control, such as growing plants that need lower CO₂ concentration or a quicker reduction is necessary. Disabled by default, see Configuration section to enable. 

**Other Safety Functions**: 

The fan health monitoring system detects fan failure using the rotation counter. An alarm message is shown on the OLED display if no rotation is detected while the fan is expected to run.

**Real-Time Operating System Architecture**: 

Built on FreeRTOS with multiple concurrent tasks handling sensor reading, control logic, user interface, and network communication. Tasks are coordinated through event groups, mutexes, and software timers, ensuring responsive operation and deterministic behavior.

## System Architecture

### Software Architecture

```
Greenhouse_CO2_controller
├── GreenhouseMonitor (Central Controller)
├── CO2Controller
│   ├── CO2Sensor
│   ├── CO2Valve
│   └── FanController
├── Environment_Sensors
│   ├── HumidityTempSensor
│   └── PressureSensor (currently not used)
├── UI (UI and User Control)
├── EEPROM (Configuration Storage)
├── Utils
│   └── Debug (Debug Print)
└── ThingSpeak Service
```

### CO₂ Control State Machine

**IDLE**
- Normal state, monitoring CO₂ level
- Transitions: 
  - When CO₂ < setpoint - deadband (by default 50 ppm) -> INJECTING
  - When CO₂ > CO2_CRITICAL (by default 2000 ppm) -> EMERGENCY
  - (Optional, requires `WITH_ACTIVE_VENTILATION` enabled) When CO₂ > setpoint + ACTIVE_VENTILATION_THRESHOLD (by default 200 ppm) -> VENTILATING

**INJECTING**
- Opening valve to inject CO₂. It uses timer to close the valve. Injection duration is calculated by formula: `open_time = K × (setpoint - CO₂_current)`, with K = 13, and clamped between 50ms and 2000ms.
- Transitions: After injection duration -> MIXING

**MIXING**

- Waiting time for letting CO₂ distribute evenly throughout the greenhouse. By default 60 seconds (configurable for larger greenhouses)
- Transitions: After mixing period -> IDLE

**VENTILATING** (Optional - requires `WITH_ACTIVE_VENTILATION` enabled)

- Fan actively running to reduce excess CO₂ when significantly above setpoint (defult 200 ppm)
- Transitions: When CO₂ drops to setpoint + deadband -> IDLE

**EMERGENCY**
- Critical high CO₂ alarm, maximum ventilation and valve closed (by default 2000 ppm)
- Warning displayed on OLED
- Transitions: Running until CO₂ drops to CO₂ setpoint

For detailed parameter definitions and customization, refer to `src/CO2_Controller/CO2Controller.h`.

### FreeRTOS Task Architecture

| Task | Priority | Responsibility |
|------|----------|---------------|
| `greenhouse_monitor_run` | IDLE+2 | Main coordinator, handles setpoint changes from UI/network and coordinate other modules |
| `CO2Controller::controlTask` | IDLE+3 | CO₂ control loop, by default at 1s interval |
| `network_setting_task` | IDLE+2 | WiFi configuration and connection management |
| `read_sensor_task` | IDLE+1 | Periodic sensor reading and data distribution |
| `UI_control::runner` | IDLE+2 | User input handling and display updates |
| `ThingSpeak tasks` | IDLE+1 and IDLE+2 | WiFi connection, data upload, remote setpoint retrieval |

Data synchronization using event groups, mutexes, and software timers.

## Hardware Requirements

### Components List

| Component | Notes |
|-----------|-------|
| Raspberry Pi Pico W (RP2040) | WiFi version required |
| CO₂ Sensor (Vaisala GMP252) | Modbus RTU compatible. Address: 240, outputs 32-bit float |
| Temperature/Humidity Sensor (Vaisala HMP60) | Modbus RTU compatible. Address: 241, 16-bit registers |
| Fan Controller (Produal MIO 12-V) | Address: 1, speed range: 0-1000 |
| 24C256 EEPROM | I2C. Address: 0x50 |
| OLED Display SSD1306 128x64 | I2C. Address: 0x3C |
| Solenoid valve with relay |  |
| Rotary Encoder, with push button |  |
| Buttons |  |

### Pin Connections

#### UART (Modbus RTU)
| Pico Pin | Connection |
|----------|----------|
| GPIO 4 | UART1 TX |
| GPIO 5 | UART1 RX |

9600 baud, 2 stop bits, no parity

#### I2C Buses
| Bus | Pico Pins | Speed | Devices |
|-----|-----------|-----------|---------|
| I2C0 | GPIO 16 (SDA), GPIO 17 (SCL) | 400 kHz | EEPROM |
| I2C1 | GPIO 14 (SDA), GPIO 15 (SCL) | 400 kHz | SSD1306 OLED |

#### GPIO
| Component         | GPIO Pin | Notes               |
| ----------------- | -------- | ------------------- |
| Button 1          | GPIO 9   | Pull-up, active low |
| Button 2          | GPIO 8   | Pull-up, active low |
| Button 3          | GPIO 7   | Pull-up, active low |
| Rotary Encoder A  | GPIO 10  |                     |
| Rotary Encoder B  | GPIO 11  |                     |
| Rotary Encoder SW | GPIO 12  | Pull-up, active low |
| CO₂ Valve Relay   | GPIO 27  | Active high         |

## Dependencies

### Required Tools and Libraries

1. **Pico SDK**
   - Set environment variable: `export PICO_SDK_PATH=/path/to/pico-sdk`
2. **FreeRTOS Kernel V10.6.2**
   - Included in `rp2040-freertos/FreeRTOS-KernelV10.6.2/`
3. **Build Tools**
   - CMake ≥ 3.12
   - ARM GCC Toolchain (`arm-none-eabi-gcc`)
   - Make build system
4. **Optional for Debugging**
   - OpenOCD
   - GDB (`arm-none-eabi-gdb`)

### Environment Setup

#### Pico SDK

Refer to https://github.com/raspberrypi/pico-sdk

#### Toolchain

Download from [ARM Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Choose the operating system and architecture of your computer, such as Windows, macOS and Linux. 

This project needs `AArch32 bare-metal target (arm-none-eabi)`.

## Build

### Method 1: Building UF2 File

This method generates a `.uf2` file that can be drag-and-dropped onto the Pico in bootloader mode.

```bash
# Clone the repository
git clone --recursive https://github.com/zzy1099207684/Greenhouse_CO2_controller.git
cd Greenhouse_CO2_controller

# Create build directory
mkdir build
cd build

# Configure CMake
cmake ..

# Build the project
make -j4
```

Ater building, it will output file `Greenhouse_CO2.uf2`. To install:

1. Hold the BOOTSEL button on Pico W while connecting USB
2. Pico appears as a mass storage device
3. Drag `Greenhouse_CO2.uf2` to the RPI-RP2 drive
4. Pico automatically reboots and runs the program

### Method 2: Using OpenOCD

This method uses a debugger probe (e.g. Raspberry Pi Debug Probe).

**Install OpenOCD:**

```bash
# Ubuntu/Debian
sudo apt install openocd

# macOS
brew install openocd
```

Or use the prebuilt binary from  Raspberry Pi's downstream fork: https://github.com/raspberrypi/openocd

**Flash via OpenOCD:**

```bash
# From build directory
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program Greenhouse_CO2.elf verify reset exit"
```

## Configuration

### ThingSpeak Setup

1. **Create a ThingSpeak Channel** at [thingspeak.com](https://thingspeak.com/)

2. **Configure Channel Fields:**
   - Field 1: CO₂ Level (ppm)
   - Field 2: Temperature (°C)
   - Field 3: Relative Humidity (%)
   - Field 4: Fan Speed (0-1000)
   - Field 5: CO₂ Setpoint (ppm) - for remote control

3. **Update API Keys** in `src/network/entry/thing_speak.h`:
   ```cpp
   #define WRITE_API_KEY "YOUR_WRITE_API_KEY"
   #define READ_API_KEY  "YOUR_READ_API_KEY"
   ```

4. **Rebuild** the project after modifying API keys.

The default API key in the project is a dummy one, please apply your own key to use this function. 

### WiFi Configuration

1. Power on the system
2. Navigate to Settings -> Network Settings on OLED display
3. Select WiFi SSID from scanned networks or enter manually
4. Enter password using rotary encoder
5. Credentials are automatically saved to EEPROM

### CO₂ Control Parameters

**Adjusting CO₂ Setpoint:**

- **Via UI**: Settings -> Set CO₂ Target
- **Via ThingSpeak**: Update Field 5 in your channel
- Changes are persisted to EEPROM automatically

Other default parameters are defined in `src/CO2_Controller/CO2Controller.h`.

### Enable Active Ventilation Mode (Optional)

By default, the system uses passive control: it stops CO₂ injection when the level reaches the setpoint. This is sufficient for most applications. However, if you need more precise CO₂ regulation, such as plants that require lower CO₂ concentrations, or when you need to reduce CO₂ levels more quickly. 

**To enable:**

1. Edit `CMakeLists.txt`, uncomment this line (around line 123):
   ```cmake
   WITH_ACTIVE_VENTILATION
   ```

2. Rebuild the project:
   ```bash
   cd build
   make clean
   cmake ..
   make
   ```

When enabled, this adds a `VENTILATING` state to the CO₂ control state machine. In this state, fans actively run to remove excess CO₂ when levels are significantly above the setpoint. 

### Debug Output

Debug printing is enabled by default. To view debug messages, connect to debugger via USB, at baud rate 115200. To disable debug output, remove `ENABLE_DEBUG_PRINT` in `CMakeLists.txt` (around line 122).

## License

[GPL-3.0 license](https://github.com/zzy1099207684/Greenhouse_CO2_controller/tree/main#GPL-3.0-1-ov-file)

## Contributors

- [**shengt25**](https://github.com/shengt25) - CO₂ Controller, Modbus integration
- [**binchiz**](https://github.com/binchiz) - Greenhouse Monitor, EEPROM, Environmental sensors
- [**Riinaaal**](https://github.com/Riinaaal) - User Interface (OLED + Input handling)
- [**zzy1099207684**](https://github.com/zzy1099207684) - Network integration, ThingSpeak service

## Acknowledgments

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [FreeRTOS](https://www.freertos.org/)
- [ThingSpeak](https://thingspeak.com/)
- [nanomodbus](https://github.com/debevv/nanomodbus) - Modbus RTU implementation
- [Hardware libraries](https://gitlab.metropolia.fi/lansk/rp2040-freertos) - i2c, uart, modbus, ipstack and ssd1306 libraries
