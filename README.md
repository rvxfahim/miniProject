# Embedded HVAC Controller

A real-time embedded HVAC (Heating, Ventilation, and Air Conditioning) controller system that simulates room temperature control using Arduino with FreeRTOS and a Python-based graphical user interface.

## Demo

https://github.com/rvxfahim/miniProject/raw/main/demo.mp4

[Download demo video](./demo.mp4)

## Overview

This project implements a temperature control system with the following components:

1. **Arduino Controller**: Runs FreeRTOS tasks to simulate room thermal dynamics, manage compressor operations, and handle fan speed control
2. **Python GUI**: Provides a user-friendly interface built with PySide6/Qt for monitoring and controlling the HVAC system
3. **Serial Communication**: JSON-based protocol for bidirectional communication between the controller and GUI

## Architecture

### Arduino Controller (main.cpp)

The embedded controller uses FreeRTOS to manage multiple concurrent tasks:

- **Simulation Task**: Models room thermal dynamics including heat transfer, cooling power, and temperature changes based on physical parameters (thermal resistance, room volume, heat capacity)
- **Print Task**: Handles serial output of system status data in JSON format
- **Receive Task**: Processes incoming commands from the GUI (set temperature, start/stop, outside temperature, switch time)
- **Compressor Task**: Controls dual compressor operation with initialization and steady-state modes to maintain desired temperature
- **Load Balance Task**: Alternates between compressors at configurable intervals to distribute wear
- **Fan Speed Task**: Reads analog input and adjusts fan PWM output accordingly

The system simulates a room with configurable parameters:
- Wall thermal resistance: 0.005 K/W
- Room volume: 20 m³
- Room heat capacity: 1000 J/K
- Compressor cooling power: 2500-5000 W (dual mode)

### Python GUI (main.py)

The graphical interface provides:

- **Serial Port Configuration**: Automatic detection and connection to available serial ports
- **Temperature Monitoring**: Real-time display of inside and outside temperatures with graphical charts
- **Control Panel**: Set desired temperature, start/stop controller, adjust outside temperature and compressor switch time
- **Compressor Status**: Visual indicators for compressor A and B operation states

The GUI uses multi-threading to handle:
- Serial data reception (SerialThread)
- Serial data transmission with acknowledgment handling (SerialsendThread)

### Communication Protocol

JSON messages are exchanged over serial connection (Arduino configured for 9600 baud, GUI allows configurable baud rate):

**From GUI to Arduino**:
- `{"setT": <float>}` - Set desired temperature
- `{"start": <0|1>}` - Start or stop controller
- `{"outT": <int>}` - Set outside temperature
- `{"switchT": <int>}` - Set compressor switch time in seconds

**From Arduino to GUI**:
- `{"outT": <float>, "inT": <float>, "watt": <float>, "tm": <float>}` - Temperature and power data
- `{"comA": <0|1>, "comB": <0|1>}` - Compressor states
- `{"ack": 1}` - Command acknowledgment

## Hardware Requirements

- Arduino board (Uno or Mega 2560)
- Temperature sensors (simulated in this version)
- Fan with PWM control (connected to analog pin A0 input, PWM output on pin 4)
- Two compressor control outputs (pins 51 and 53 for Mega)
- USB serial connection to host computer

## Software Requirements

### Arduino Development
- PlatformIO
- FreeRTOS library (feilipu/FreeRTOS)
- ArduinoJson library (version 6.21.2 or higher)

### GUI Application
- Python 3.x
- PySide6
- pyserial
- numba

Install Python dependencies:
```bash
pip install PySide6 pyserial numba
```

## Building and Running

### Arduino Firmware

1. Open project in PlatformIO
2. Select environment (uno or megaatmega2560)
3. Build and upload:
```bash
pio run -e megaatmega2560 --target upload
```

### GUI Application

1. Navigate to src directory
2. Run the Python application:
```bash
cd src
python main.py
```

## Usage

1. Upload the Arduino firmware to your board
2. Launch the Python GUI application
3. Select the correct serial port from the Configuration tab
4. Set the baud rate to 9600 to match the Arduino configuration
5. Click Connect to establish communication
5. Switch to the Control tab
6. Set desired temperature and outside temperature
7. Click Start to begin HVAC control
8. Monitor real-time temperature changes and compressor status

## Project Structure

```
miniProject/
├── src/
│   ├── main.cpp              # Arduino FreeRTOS controller
│   ├── main.py               # Python GUI application
│   ├── main.qml              # QML UI main window
│   ├── Page2.qml             # QML control page
│   ├── DialControl.qml       # QML custom dial widget
│   └── *.png, *.jpg          # UI assets
├── platformio.ini            # PlatformIO configuration
├── demo.mp4                  # Project demonstration video
└── README.md                 # This file
```

## Features

- Real-time temperature simulation with physical thermal model
- Dual compressor system with automatic load balancing
- FreeRTOS-based task scheduling for reliable real-time operation
- JSON-based communication protocol with acknowledgment mechanism
- Cross-platform GUI with real-time charts and visual feedback
- Configurable system parameters (outside temperature, switch time)
- Safety features (compressor minimum off-time, initialization sequence)

## License

This project is provided as-is for educational and demonstration purposes.
