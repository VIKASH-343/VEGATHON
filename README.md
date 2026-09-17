# VEGAXIS — Underground Worker Safety & Communication System
 
> A low-cost, hardware-centric worker safety system based on the VEGA ARIES processor, integrating environmental monitoring, physiological monitoring, fall detection, local emergency alerts, wireless communication, and experimental Through-the-Earth (TTE) magnetic-induction communication.
 
---
 
## Overview
 
VEGAXIS is a worker safety and communication platform designed for environments where conventional communication infrastructure may be unreliable, including:
 
- Underground mines
- Confined spaces
- Industrial environments
- Tunnels
- Remote work sites
- Hazardous environments
The system continuously monitors worker and environmental conditions using a set of low-cost sensors connected to the **VEGA ARIES** development platform.
 
Safety-critical events are handled locally by the node through visual and audible alerts. Wireless communication is used for remote monitoring, while an experimental **Through-the-Earth (TTE) Magnetic Induction communication system** is being developed as an additional communication path for underground environments.
 
The project intentionally uses **local threshold-based decision logic rather than AI/ML**, keeping the safety response deterministic and suitable for an embedded proof-of-concept.
 
---
 
# System Architecture
 
```text
                         VEGAXIS SAFETY NODE
                                │
                                ▼
                    ┌───────────────────────┐
                    │    VEGA ARIES v3.0    │
                    │       Processor       │
                    └───────────┬───────────┘
                                │
        ┌───────────────────────┼────────────────────────┐
        │                       │                        │
        ▼                       ▼                        ▼
  ENVIRONMENTAL            PHYSIOLOGICAL             MOTION
     SENSING                  SENSING                SENSING
        │                       │                        │
   ┌────┴────┐             ┌────┴────┐              ┌────┴────┐
   │ DHT11   │             │MAX30100 │              │ ADXL345 │
   │ Temp/Hum │             │ HR/SpO2 │              │  Fall   │
   └─────────┘             └─────────┘              │Detection│
   ┌─────────┐                  │                   └─────────┘
   │ MQ135   │                  │
   │ Gas/AQ  │                  │
   └─────────┘                  │
        │                       │
        └───────────┬───────────┘
                    │
                    ▼
             THRESHOLD ENGINE
                    │
        ┌───────────┼────────────┐
        │           │            │
        ▼           ▼            ▼
     BUZZER      RGB/ALARM      OLED
        │           │            │
        └───────────┼────────────┘
                    │
                    ▼
             COMMUNICATION
                    │
          ┌─────────┴──────────┐
          │                    │
          ▼                    ▼
     E220 LoRa             TTE / MI
      Module             Communication
          │                    │
          ▼                    ▼
     Remote Node        Through Earth
     / Dashboard        Communication
```
 
---
 
# Key Features
 
* Environmental monitoring
* Temperature and humidity measurement
* Hazardous-air/gas indication
* Heart-rate monitoring
* SpO₂ monitoring
* Fall detection
* Emergency SOS button
* Local audible alarm
* Visual alarm indication
* OLED status display
* Wireless communication using LoRa
* Experimental Through-the-Earth communication
* Local threshold-based safety decisions
* Modular hardware architecture
* Low-cost proof-of-concept implementation
* VEGA ARIES processor-based design
---
 
# Hardware
 
## Main Controller
 
### VEGA ARIES v3.0
 
The VEGA ARIES board acts as the central processing and control platform.
 
It interfaces with:
 
* Digital sensors
* Analog sensors
* I²C peripherals
* UART communication modules
* Buttons
* Alarm outputs
* Display
* Communication hardware
The board provides the required GPIO, I²C, UART, analog and power interfaces for the prototype.
 
---
 
# Sensors
 
## 1. DHT11 — Temperature & Humidity
 
Measures:
 
* Ambient temperature
* Relative humidity
### Connection
 
| DHT11 | VEGA ARIES |
| ----- | ---------- |
| DATA  | GPIO4      |
| VCC   | 3.3V       |
| GND   | GND        |
 
---
 
## 2. MQ135 — Gas / Air Quality Sensor
 
The MQ135 is used as an experimental air-quality / hazardous-gas indication sensor.
 
### Connection
 
| MQ135 | VEGA ARIES      |
| ----- | --------------- |
| AOUT  | Analog Input A0 |
| VCC   | 5V              |
| GND   | GND             |
 
> **Note:** MQ-series sensors are not selective gas analyzers. The prototype therefore treats the MQ135 output as an air-quality / hazard indicator rather than as a precise concentration measurement.
 
---
 
## 3. ADXL345 — Fall Detection
 
The ADXL345 accelerometer is used to detect abnormal motion patterns associated with a possible worker fall.
 
The sensor communicates through the I²C interface.
 
### I²C Address
 
```text
0x53
```
 
### Signals
 
| ADXL345 | VEGA ARIES |
| ------- | ---------- |
| VCC     | 3.3V       |
| GND     | GND        |
| SDA     | I2C0_SDA   |
| SCL     | I2C0_SCL   |
 
---
 
## 4. MAX30100 — Heart Rate & SpO₂
 
The MAX30100 provides optical sensing for:
 
* Heart rate
* Blood oxygen saturation (SpO₂)
### I²C Address
 
```text
0x57
```
 
### Signals
 
| MAX30100 | VEGA ARIES |
| -------- | ---------- |
| VCC      | 3.3V       |
| GND      | GND        |
| SDA      | I2C0_SDA   |
| SCL      | I2C0_SCL   |
 
---
 
## 5. SH1106 OLED Display
 
A small OLED display provides local status information to the worker.
 
Possible display pages include:
 
```text
PAGE 1
Temperature
Humidity
Gas Status
 
PAGE 2
Heart Rate
SpO₂
 
PAGE 3
Motion / Fall Status
 
PAGE 4
Communication Status
```
 
The OLED shares the I²C bus with the other I²C sensors.
 
---
 
# Shared I²C Bus
 
The ADXL345, MAX30100 and SH1106 OLED operate on the shared I²C0 interface.
 
```text
                 VEGA ARIES
                     │
              ┌──────┴──────┐
              │    I2C0     │
              │             │
          SDA │─────────────┼─────┐
          SCL │─────────────┼──┐  │
              └─────────────┘  │  │
                               │  │
                    ┌──────────┘  └──────────┐
                    ▼                       ▼
                ADXL345                  MAX30100
                 0x53                      0x57
                    │                       │
                    └──────────┬────────────┘
                               │
                               ▼
                          SH1106 OLED
```
 
### Pull-up resistors
 
The I²C bus uses pull-up resistors:
 
```text
SDA ─── 4.7kΩ ─── 3.3V
SCL ─── 4.7kΩ ─── 3.3V
```
 
---
 
# Emergency & Safety Interface
 
## SOS Button
 
The dedicated SOS button provides a manual emergency trigger.
 
```text
Button → GPIO6
Other terminal → GND
```
 
The input is configured using:
 
```text
INPUT_PULLUP
```
 
When pressed, the controller detects the active-low signal and initiates the emergency response.
 
---
 
## Buzzer
 
The buzzer provides a local audible warning.
 
```text
Buzzer + → GPIO5
Buzzer - → GND
```
 
The buzzer can be activated during:
 
* Fall detection
* Hazardous environmental condition
* SOS event
* Critical system condition
---
 
## OLED Page Button
 
A separate button is provided for cycling through display pages.
 
```text
Button → GPIO9
Other terminal → GND
```
 
Configured as:
 
```text
INPUT_PULLUP
```
 
---
 
## Fall Alarm Output
 
A dedicated digital output is provided for the fall alarm.
 
```text
Alarm Signal → GPIO13
```
 
This output can drive an external indicator or alarm driver.
 
---
 
# LoRa Communication
 
## E220-900T22D
 
The E220-900T22D LoRa module provides long-range wireless communication between the worker node and a remote monitoring node.
 
### UART Interface
 
```text
E220 TXD → ARIES UART1 RX1
E220 RXD → ARIES UART1 TX1
```
 
### Control Pins
 
```text
M0 → GND
M1 → GND
```
 
### Power
 
```text
VCC → Appropriate module supply
GND → GND
```
 
The LoRa link can be used to transmit:
 
```text
Worker ID
Temperature
Humidity
Gas status
Heart rate
SpO₂
Fall status
SOS status
Battery/system status
Communication status
```
 
---
 
# Through-the-Earth Communication
 
One of the major experimental components of VEGAXIS is the development of a **Through-the-Earth Magnetic Induction (TTE-MI) communication link**.
 
Unlike conventional RF communication, the prototype attempts to couple a low-frequency magnetic field through the surrounding earth.
 
```text
              TRANSMITTER
                   │
             9.99 kHz Signal
                   │
              Coil Driver
                   │
              TX Resonant
                 Tank
                   │
                 TX Coil
                   │
                   ▼
═══════════════════════════════
        EARTH / SOIL
     Magnetic Coupling Path
═══════════════════════════════
                   │
                   ▼
                 RX Coil
                   │
              RX Resonant
                 Network
                   │
                LM386
              Amplifier
                   │
                   ▼
             Received Signal
```
 
The TTE subsystem is currently treated as a **proof-of-concept communication link**, separate from the main LoRa communication path.
 
---
 
# TTE Transmitter
 
The transmitter generates a low-frequency carrier of approximately:
 
```text
f ≈ 9.99 kHz
```
 
The prototype uses a 555 timer configured as an astable oscillator.
 
### TX Hardware
 
```text
555 Timer
    │
    ▼
Frequency Control
(Potentiometer)
    │
    ▼
Coil Driver
    │
    ▼
Resonant Tank
    │
    ▼
TX Coil
```
 
### TX Coil
 
Approximate prototype parameters:
 
```text
Outer radius ≈ 9.5 cm
Inner radius ≈ 1.5 cm
Inductance ≈ 62 µH
Resistance ≈ 0.1 Ω
```
 
### Tank Capacitor
 
```text
C ≈ 4.7 µF
```
 
The resonant network is used to increase the magnetic excitation around the operating frequency.
 
---
 
# TTE Receiver
 
The receiver uses a separate pickup coil to detect the magnetic field generated by the transmitter.
 
```text
RX Coil
   │
   ▼
Filter / Resonant Network
   │
   ▼
LM386 Amplifier
   │
   ▼
Volume Control
   │
   ▼
Audio / Monitoring Output
```
 
### RX Coil
 
Approximate prototype parameters:
 
```text
Outer radius ≈ 4.5 cm
Inner radius ≈ 1.5 cm
Inductance ≈ 94 µH
Resistance ≈ 3.19 Ω
```
 
### Filter Capacitor
 
```text
C ≈ 2.7 µF
```
 
---
 
# TTE Prototype Parameters
 
| Parameter           | Prototype Value |
| ------------------- | --------------: |
| Operating frequency |       ~9.99 kHz |
| TX coil inductance  |          ~62 µH |
| RX coil inductance  |          ~94 µH |
| TX coil resistance  |          ~0.1 Ω |
| RX coil resistance  |         ~3.19 Ω |
| TX tank capacitor   |         ~4.7 µF |
| RX filter capacitor |         ~2.7 µF |
 
---
 
# Simulated Range Estimation
 
An initial simulation was performed for a 10 kHz magnetic-induction link with an assumed transmitter current of approximately 1 A.
 
The estimated maximum ranges were:
 
| Soil Condition         | Estimated Range |
| ---------------------- | --------------: |
| Dry soil               |         22.61 m |
| Typical soil           |         20.09 m |
| Wet soil               |         16.31 m |
| Highly conductive soil |         11.04 m |
 
These values are **simulation estimates**, not measured field performance.
 
Actual communication range depends on:
 
* Soil conductivity
* Coil geometry
* Coil orientation
* Transmitter current
* Receiver sensitivity
* Resonant tuning
* Environmental electromagnetic noise
* Ground conditions
* Receiver signal-processing method
---
 
# Safety Decision Logic
 
VEGAXIS uses deterministic threshold-based logic.
 
```text
Sensor Data
    │
    ▼
Signal Processing
    │
    ▼
Threshold Evaluation
    │
    ├──── NORMAL ───────► Continue Monitoring
    │
    ├──── WARNING ──────► Local Warning
    │
    └──── CRITICAL ─────► Alarm + Communication
```
 
A critical event can trigger:
 
```text
LOCAL BUZZER
     +
VISUAL INDICATOR
     +
OLED ALERT
     +
REMOTE LoRa MESSAGE
```
 
This architecture allows the safety response to remain functional even when the wireless communication link is unavailable.
 
---
 
# Example Safety Events
 
## Fall Detected
 
```text
ADXL345
   ↓
Motion Analysis
   ↓
Fall Detected
   ↓
Local Alarm
   ↓
OLED Alert
   ↓
LoRa Emergency Message
```
 
## Environmental Hazard
 
```text
DHT11 / MQ135
      ↓
Threshold Evaluation
      ↓
Hazard Detected
      ↓
Buzzer + Visual Alert
      ↓
OLED Warning
      ↓
Remote Notification
```
 
## SOS
 
```text
SOS Button
     ↓
GPIO6
     ↓
Emergency Event
     ↓
Immediate Local Alarm
     ↓
LoRa Emergency Packet
```
 
---
 
# Communication Architecture
 
VEGAXIS uses multiple communication layers.
 
```text
                  VEGAXIS
                     │
        ┌────────────┼────────────┐
        │            │            │
        ▼            ▼            ▼
      I²C           UART         TTE-MI
        │            │            │
        ▼            ▼            ▼
 Sensors/OLED       LoRa       Magnetic Link
        │            │            │
        ▼            ▼            ▼
 Local Node     Remote Node   Underground
                              Communication
```
 
The communication methods are complementary rather than mutually exclusive.
 
---
 
# Hardware Block Diagram
 
```text
                         ┌──────────────────┐
                         │   VEGA ARIES     │
                         │     v3.0         │
                         └────────┬─────────┘
                                  │
          ┌───────────────────────┼──────────────────────┐
          │                       │                      │
          ▼                       ▼                      ▼
   ┌─────────────┐         ┌─────────────┐       ┌─────────────┐
   │ Environmental│         │ Physiological│       │   Motion    │
   │   Sensors   │         │   Sensors   │       │   Sensor    │
   │             │         │             │       │             │
   │ DHT11       │         │ MAX30100    │       │ ADXL345     │
   │ MQ135       │         │ HR / SpO2   │       │ Fall        │
   └─────────────┘         └─────────────┘       └─────────────┘
          │                       │                      │
          └───────────────────────┼──────────────────────┘
                                  │
                                  ▼
                         ┌─────────────────┐
                         │ Safety Decision │
                         │     Engine      │
                         └────────┬────────┘
                                  │
                 ┌────────────────┼────────────────┐
                 │                │                │
                 ▼                ▼                ▼
              Buzzer             OLED          Alarm Output
                 │                │                │
                 └────────────────┼────────────────┘
                                  │
                                  ▼
                         ┌─────────────────┐
                         │ Communication   │
                         └────────┬────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    ▼                           ▼
               E220 LoRa                    TTE-MI
                    │                           │
                    ▼                           ▼
             Remote Monitoring          Through-Earth Link
```
 
---
 
# Prototype Development
 
The project is being developed incrementally.
 
### Phase 1 — Sensor Integration
 
* [x] VEGA ARIES setup
* [x] Temperature/humidity sensing
* [x] Gas/air-quality sensing
* [x] Accelerometer interface
* [x] Heart-rate/SpO₂ interface
* [x] OLED interface
### Phase 2 — Local Safety System
 
* [x] SOS button
* [x] Buzzer
* [x] Alarm output
* [x] Display paging
* [x] Threshold-based alerts
### Phase 3 — Wireless Communication
 
* [x] UART interface
* [x] E220 LoRa integration
* [ ] Remote monitoring node
* [ ] Complete telemetry packet format
* [ ] End-to-end communication testing
### Phase 4 — TTE Communication
 
* [x] Magnetic-induction concept study
* [x] Coil parameter estimation
* [x] TTE range simulation
* [x] TX breadboard prototype
* [x] RX breadboard prototype
* [x] 9.99 kHz operating point
* [ ] Digital data modulation
* [ ] Digital demodulation
* [ ] Error detection
* [ ] End-to-end TTE data link
* [ ] Field testing
---
 
# Repository Structure
 
```text
VEGAXIS/
│
├── README.md
│
├── hardware/
│   ├── schematics/
│   │   ├── safety_node/
│   │   └── tte_mi/
│   │
│   ├── pcb/
│   │
│   └── wiring/
│
├── firmware/
│   ├── sensor_tests/
│   ├── safety_node/
│   ├── lora/
│   └── tte/
│
├── communication/
│   ├── lora/
│   └── tte_mi/
│
├── simulation/
│   └── tte/
│
├── documentation/
│   ├── architecture/
│   ├── test_results/
│   └── images/
│
└── media/
    ├── block_diagram/
    ├── hardware/
    └── prototype/
```
 
---
 
# Pin Assignment
 
| Function         | VEGA ARIES Pin |
| ---------------- | -------------- |
| DHT11 Data       | GPIO4          |
| Buzzer           | GPIO5          |
| SOS Button       | GPIO6          |
| OLED Page Button | GPIO9          |
| Fall Alarm       | GPIO13         |
| MQ135 Analog     | A0             |
| ADXL345 SDA      | I2C0 SDA       |
| ADXL345 SCL      | I2C0 SCL       |
| MAX30100 SDA     | I2C0 SDA       |
| MAX30100 SCL     | I2C0 SCL       |
| OLED SDA         | I2C0 SDA       |
| OLED SCL         | I2C0 SCL       |
| LoRa TXD         | UART1 RX1      |
| LoRa RXD         | UART1 TX1      |
 
---
 
# Power Architecture
 
The prototype can be powered using a portable power source such as a power bank.
 
```text
                 POWER BANK
                     │
                     ▼
              VEGA ARIES
                     │
        ┌────────────┼─────────────┐
        │            │             │
        ▼            ▼             ▼
       5V          3.3V           GND
        │            │             │
        ▼            ▼             ▼
     MQ135       I²C Sensors    All Modules
                  OLED
```
 
Power requirements should be validated individually for each module before integrating the complete system.
 
---
 
# Design Philosophy
 
VEGAXIS follows several design principles:
 
### 1. Local Safety First
 
A safety event should not depend entirely on a remote server or wireless network.
 
### 2. Deterministic Decision Making
 
The prototype uses threshold-based logic instead of AI/ML for critical safety decisions.
 
### 3. Modular Architecture
 
Sensors, communication modules and safety outputs can be independently replaced or upgraded.
 
### 4. Multiple Communication Paths
 
LoRa provides wireless communication while TTE-MI is investigated for underground communication where conventional wireless links may have limitations.
 
### 5. Low-Cost Prototyping
 
The initial system is constructed using commercially available development boards and sensor modules to enable rapid prototyping.
 
---
 
# Current Prototype
 
The current prototype consists of:
 
```text
VEGA ARIES v3.0
       │
       ├── DHT11
       ├── MQ135
       ├── ADXL345
       ├── MAX30100
       ├── SH1106 OLED
       ├── SOS Button
       ├── Page Button
       ├── Buzzer
       ├── Fall Alarm Output
       │
       └── E220-900T22D LoRa
       
       +
 
   TTE-MI Prototype
       │
       ├── 555 TX oscillator
       ├── TX coil
       ├── Resonant tank
       ├── RX coil
       ├── RX filter
       └── LM386 receiver
```
 
---
 
# Demonstration Flow
 
During a demonstration, the system can be operated as follows:
 
```text
1. Power ON
      ↓
2. Initialize VEGA ARIES
      ↓
3. Initialize sensors
      ↓
4. Display normal status
      ↓
5. Continuously monitor worker/environment
      ↓
6. Detect abnormal condition
      ↓
7. Activate local warning
      ↓
8. Generate emergency event
      ↓
9. Transmit event through LoRa
      ↓
10. TTE link can be demonstrated
    as an independent underground
    communication proof-of-concept
```
 
---
 
# Experimental TTE Hardware
 
The current TTE prototype is implemented on breadboards.
 
### Transmitter
 
```text
555 Timer
    ↓
Frequency Control
    ↓
Coil Driver
    ↓
Resonant Tank
    ↓
TX Coil
```
 
### Receiver
 
```text
RX Coil
    ↓
Filter
    ↓
LM386
    ↓
Volume Control
    ↓
Output
```
 
The present prototype primarily demonstrates **magnetic coupling and signal reception**. Digital communication functionality is part of the continuing development.
 
---
 
# Limitations
 
This project is currently a research and engineering prototype.
 
Important limitations include:
 
* Sensor modules are not industrial-certified safety instruments.
* MQ135 does not provide selective identification of individual hazardous gases.
* TTE communication range is dependent on ground conditions and coil configuration.
* Simulated TTE range values have not been established as guaranteed field performance.
* Breadboard implementations are not suitable for deployment in hazardous industrial environments.
* The current TTE prototype is primarily an analog signal-link demonstration.
* A production system would require appropriate electrical, mechanical, environmental and functional-safety validation.
---
 
# Future Work
 
## Safety Node
 
* [ ] Improve sensor calibration
* [ ] Add battery monitoring
* [ ] Improve enclosure
* [ ] Helmet/wearable integration
* [ ] Optimize power consumption
* [ ] Improve fall-detection algorithm
* [ ] Develop remote monitoring dashboard
* [ ] Implement structured emergency packets
## LoRa
 
* [ ] Telemetry packet protocol
* [ ] CRC/error checking
* [ ] Emergency packet priority
* [ ] Gateway/receiver node
* [ ] Remote dashboard integration
## TTE-MI
 
* [ ] Digital modulation
* [ ] Digital demodulation
* [ ] Carrier detection
* [ ] Packet framing
* [ ] CRC
* [ ] BER measurement
* [ ] Improved resonant matching
* [ ] Higher-efficiency coil driver
* [ ] Optimized TX/RX coil geometry
* [ ] Controlled soil experiments
* [ ] Field testing
---
 
# Technologies
 
### Hardware
 
* VEGA ARIES v3.0
* DHT11
* MQ135
* ADXL345
* MAX30100
* SH1106 OLED
* E220-900T22D LoRa
* 555 Timer
* LM386
* Custom TX/RX magnetic-induction coils
### Communication
 
* I²C
* UART
* LoRa
* Low-frequency Magnetic Induction
* Through-the-Earth Communication
### Development
 
* Embedded C/C++
* Microcontroller firmware
* Circuit prototyping
* Analog electronics
* Sensor interfacing
* Magnetic-induction simulation
---
 
# Project Status
 
**Overall Status: Prototype / Proof of Concept**
 
| Subsystem                 | Status                  |
| ------------------------- | ----------------------- |
| VEGA ARIES platform       | 🟢 Prototype            |
| Environmental sensing     | 🟢 Prototype            |
| Physiological sensing     | 🟢 Prototype            |
| Fall detection            | 🟢 Prototype            |
| OLED interface            | 🟢 Prototype            |
| Local alarm system        | 🟢 Prototype            |
| LoRa communication        | 🟡 Integration          |
| TTE TX                    | 🟢 Breadboard prototype |
| TTE RX                    | 🟢 Breadboard prototype |
| TTE digital communication | 🟡 Development          |
| Full system integration   | 🟡 Development          |
| Field validation          | ⚪ Planned               |
 
---
 
# Disclaimer
 
VEGAXIS is an academic/research prototype intended to demonstrate embedded sensing, safety monitoring and communication concepts.
 
It is **not a certified safety system** and should not be used as the sole means of protecting workers in mines, confined spaces, or other hazardous environments.
 
---
 
# License
 
This project is intended for academic and research purposes.
 
License information will be added as the project is finalized.
 
---
 
# Authors
 
**VEGAXIS Project Team**
 
Developed as a hardware innovation project based on the **VEGA Processor / ARIES platform**.
 
---
 
## Project Summary
 
```text
          SENSE
            │
            ▼
       ┌─────────┐
       │ VEGAXIS │
       │  NODE   │
       └────┬────┘
            │
       ┌────┴─────┐
       │          │
       ▼          ▼
    PROTECT    COMMUNICATE
       │          │
       ▼          ├──────────► LoRa
   Local Alarm    │
   + Display      └──────────► TTE-MI
                             
              ↓
       Underground Worker
            Safety
```
 
**VEGAXIS — Sense. Protect. Communicate.**
 
