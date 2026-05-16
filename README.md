# ArduinoCore-AT32F4

# Arduino Core Support for WeAct Studio BlackPill (AT32F403ACGU7)

This repository provides an enhanced, updated, and community-driven fork of the official WeActStudio Arduino Core for the AT32F4 series. 

Despite numerous community requests for essential libraries and peripheral support, the original upstream repository has lacked updates for a long time. To bridge this gap, this distribution integrates much-needed libraries, core platform updates, and comprehensive hardware support to make the **AT32F403ACGU7 BlackPill** fully functional and stable within the Arduino IDE ecosystem.

---

## 🚀 Key Enhancements in This Distribution

* **Added Missing Libraries:** Integrated core hardware libraries that were previously omitted or incomplete in the official release.
* **Peripheral Extensions:** Improved support for onboard timers, SPI/I2C buses, and optimized interface routing.
* **Bug Fixes & Stability:** Resolved various compilation errors and structure mismatches encountered in the legacy core.
* **Community-Driven:** Built to keep the AT32F4 series alive and highly accessible for developers using the traditional Arduino workflow.

---

## 🛠️ Hardware Specifications

The target hardware for this core is the **WeAct Studio BlackPill AT32F403ACGU7** development board. Key specs include:

* **MCU:** AT32F403ACGU7 @ 240MHz (ARM Cortex-M4)
* **Memory:** 96+128 KB RAM / 256+768 KB ROM
* **Peripherals:** 4x SPI, 3x I2C, 7x USART/UART, 14x Timers, USBFS, 2x CAN, and SDIO.

> 📝 For full hardware details, schematics, and official datasheets, please refer to the [WeActStudio BlackPill Hardware Repository](https://github.com/WeActStudio/WeActStudio.BlackPill).

---

## 🤝 How to Contribute & Collaborate

This project is fully open-source and relies on community collaboration to grow. Since this is an independent distribution born out of necessity, your feedback, bug reports, and code contributions are highly valued!

Here is how you can help optimize and expand this core:

### 1. 🐛 Report Bugs & Request Features
If you encounter compilation failures, broken library functions, or unexpected hardware behavior:
* Open an **Issue** with details about your setup.
* Provide a minimal reproducible code snippet (MRE) if applicable.
* Mention your operating system and Arduino IDE version.

### 2. 🧪 Testing & Validation
Simply downloading this core, running your existing Arduino projects on the BlackPill AT32, and sharing your success stories or performance benchmarks in the **Discussions** tab is a massive help!

---

## 🔗 References & Credits

* **Original Core Base:** [WeActStudio/ArduinoCore-AT32F4](https://github.com/WeActStudio/ArduinoCore-AT32F4)
* **Hardware Design & Docs:** [WeActStudio.BlackPill](https://github.com/WeActStudio/WeActStudio.BlackPill)
* **MCU Manufacturer:** [ArteryTek Official Website](https://arterytek.com)


## Using this core with the Arduino IDE

## 🛠 Installation Guide

1. Open your **Arduino IDE** (v2.0 or higher recommended).
2. Navigate to **File > Preferences**.
3. Locate the **Additional Boards Manager URLs** input box and inject the following raw manifest pointer URL:
To compile for this core with the Arduino IDE, add the following URL to the boards manager.

`https://raw.githubusercontent.com/rizacelik/ArduinoCore-AT32F4/refs/heads/main/package_at32F403_index.json`

![image](at32.png)

4. Navigate to **Tools > Board > Boards Manager...**
5. Input `ArteryTek` or `AT32F403A` in the search index panel.
6. Select the target framework iteration and execute **Install**.

---

## 🔧 Hardware Requirements & Driver Installation

To program the AT32F403A board via USB and use the hardware serial monitoring features, you must install the appropriate PC drivers from ArteryTek's official support channels.

### Required Drivers:
1. **Virtual Serial Port (VCP) Driver:** Enables your PC to recognize the microcontroller's hardware UART over a standard USB connection for serial debugging.
2. **USB DFU Driver:** Required for flashing code directly onto the board using the native USB Bootloader interface. 
   *(Note: The necessary Windows DFU driver binaries are already bundled inside the `Artery_ISP_Console` tool installed automatically by this core package).*

### Official Download Instructions:
If you need to install or update the drivers manually, visit the official [ArteryTek Tools Support Page](https://arterychip.com/en/support/tools.jsp?index=4) and download the packages under these specific tabs:

- **Program and Debug:** Contains the primary debugging drivers and utility software.
- **USB Tool and Driver:** Contains the standalone USB DFU device drivers and configuration utilities.

---

# ArduinoCore-AT32F4

An Arduino Board Support Package (BSP) tailored for ArteryTek AT32F4 microcontrollers, specifically optimized for high-performance applications like flight controllers, motor control systems, and multi-sensor navigation units.

---

## 📌 Features & Architectural Capabilities
- **Hardware UART:** Native hardware serial communication. *Note: SoftwareSerial is not supported due to high-performance timing restrictions.*
- **Advanced Motor Control:** Full implementation of standard Arduino Servo control alongside ultra-fast, low-latency **DShot300/DShot600** protocols for electronic speed controllers (ESCs).
- **Multi-Bus SPI Control:** Native hardware routing for multiple concurrent SPI peripherals.

---

## Serial print support

You can print the code you've written to the Arduino's Serial display. Typical usage is as follows.

```cpp
void setup() {
  Serial.begin(115200); 
  Serial.println("AT32F403a Serial Initialized.");
}

void loop() {
    Serial.println("AT32F403A Board");
  // Your code here
  
}
```


## 🔌 Hardware UART Configuration

The core isolates communication strictly onto hardware peripherals to maintain deterministic timing. The physical pin mappings are designated as follows:


| Peripheral | Port Pin | Function | Mode |
| :--- | :--- | :--- | :--- |
| **USART1** Serial1 | `PA9` / `PA10` | TX / RX | Default Configuration |
| **USART2** Serial2 | `PA2` / `PA3` | TX / RX | Default Configuration |
| **USART3** Serial3 | `PB10` / `PB11` | TX / RX | Default Configuration |
| **UART4** Serial4 | `PA0` / `PA1` | TX / RX | Remapped Configuration |

### Code Implementation Example
```cpp
void setup() {
  // Initializes USART1 by default at 115200 baud
  Serial1.begin(115200);
  Serial.begin(115200); 
  Serial.println("AT32F4 Hardware Serial Initialized.");
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    Serial.println(c);
  // Your code here
  }
}
```

---

## 🚀 Motor Control: High-Speed DShot Implementation

For drone flight dynamics and responsive robotics, this core supports native hardware-timed **DShot300** and **DShot600** configurations directly on GPIO outputs.

### DShot Telemetry & Control Example
```cpp
#include <Arduino.h>
#include "DShot.h"

// Define ESC instances with desired operating frequency
DShot esc1(DShot::DSHOT600_HZ);   // Options: DSHOT300_HZ / DSHOT600_HZ
DShot esc2(DShot::DSHOT600_HZ);
DShot esc3(DShot::DSHOT600_HZ);
DShot esc4(DShot::DSHOT600_HZ);

void setup() {
  Serial.begin(115200);
  
  // Assign physical output pins
  esc1.begin(PA0);
  esc2.begin(PA1);
  esc3.begin(PA2);
  esc4.begin(PA3);

  // Arming sequence loop (Sends idle throttle pulse)
  Serial.println("Arming ESCs...");
  for(int i = 0; i < 1000; i++){
      esc1.write(1000);
      esc2.write(1000);
      esc3.write(1000);
      esc4.write(1000);
  }
  delay(10);
  Serial.println("System Armed.");
}

void loop() {
  // Apply operational throttle values (Range: 1047 - 2000)
  esc1.write(1100);
  esc2.write(1100);
  esc3.write(1100);
  esc4.write(1100);    
}
```

---

### Servo Control Example

```cpp
#include <Servo.h>
Servo servo1; 
Servo servo2;
Servo servo3;
Servo servo4;

int i = 0;

void setup() {
  servo1.attach(PB0);
  servo2.attach(PB1); 
  servo3.attach(PB4); 
  servo4.attach(PB5); 
}

void loop() {
  for (i = 0; i < 180; i++) { 
    servo1.write(i);              
    servo2.write(i);     
    servo3.write(i);
    servo4.write(i);         
    delay(10);                      
  }

  for (i = 180; i > 0; i--) { 
    servo1.write(i);                
    servo2.write(i);     
    servo3.write(i);
    servo4.write(i);          
    delay(10);                      
  }
}

```

You can also use the `writeMicroseconds()` function instead of the `write()` function. It's especially good for controlling brushless or DC motors.

```cpp
servo1.writeMicroseconds(1500);
servo2.writeMicroseconds(1500);
servo3.writeMicroseconds(1500);
servo4.writeMicroseconds(1500);
```

### Native Hardware SPI Pin Mapping

The core routes the three independent hardware SPI peripherals to the following physical pins:

| SPI Peripheral | Signal Function | Port Pin | Pin Description |
| :--- | :--- | :--- | :--- |
| **SPI / SPI1** | `SPI1_CS` | `PA4` | Chip Select (Default IMU / Peripherals) |
| | `SPI1_SCK` | `PA5` | Serial Clock |
| | `SPI1_MISO` | `PA6` | Master In Slave Out |
| | `SPI1_MOSI` | `PA7` | Master Out Slave In |
| **SPI_2 / SPI2**| `SPI2_CS` | `PB12` | Chip Select |
| | `SPI2_SCK` | `PB13` | Serial Clock |
| | `SPI2_MISO` | `PB14` | Master In Slave Out |
| | `SPI2_MOSI` | `PB15` | Master Out Slave In |
| **SPI_3 / SPI3**| `SPI3_CS` | `PA15` | Chip Select |
| | `SPI3_SCK` | `PB3` | Serial Clock |
| | `SPI3_MISO` | `PB4` | Master In Slave Out |
| | `SPI3_MOSI` | `PB5` | Master Out Slave In |

## 🧭 Hardware SPI Integration (MPU9250 IMU Example)

The core exposes up to three explicit SPI buses. The class interfaces are structured as:
- `SPI1` maps to **#define SPI_SELECT SPI1**
- `SPI2` maps to **#define SPI_SELECT SPI2**
- `SPI3` maps to **#define SPI_SELECT SPI3**

Below is an enterprise-grade register interface routine reading data from an **MPU9250 Inertial Measurement Unit** via the native **SPI** (`SPI1`) driver:

```cpp
#include "SPI.h"
#define SPI_SELECT SPI1

// MPU9250 Register Definitions
#define MPU9250_ADDRESS            0x68
#define MPU9250_WHO_AM_I           0x75
#define MPU9250_PWR_MGMT_1         0x6B
#define MPU9250_CONFIG             0x1A
#define MPU9250_GYRO_CONFIG        0x1B
#define MPU9250_ACCEL_CONFIG       0x1C
#define MPU9250_ACCEL_CONFIG2      0x1D
#define MPU9250_INT_PIN_CFG        0x37
#define MPU9250_ACCEL_XOUT_H       0x3B
#define MPU9250_GYRO_XOUT_H        0x43

const uint8_t MPU9250_CS_PIN = PA4;

float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

uint8_t spiTransfer(uint8_t reg, uint8_t data, bool read = false) {
    uint8_t result;
    uint8_t txByte = read ? (reg | 0x80) : reg;
    
    digitalWrite(MPU9250_CS_PIN, LOW);
    SPI.transfer(txByte);
    result = SPI.transfer(data);
    digitalWrite(MPU9250_CS_PIN, HIGH);
    
    delayMicroseconds(10);
    return result;
}

void writeRegister(uint8_t reg, uint8_t value) {
    spiTransfer(reg, value, false);
    delay(10);
}

uint8_t readRegister(uint8_t reg) {
    return spiTransfer(reg, 0x00, true);
}

void readRegisters(uint8_t reg, uint8_t* data, uint8_t length) {
    digitalWrite(MPU9250_CS_PIN, LOW);
    SPI.transfer(reg | 0x80);
    for (uint8_t i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00);
    }
    digitalWrite(MPU9250_CS_PIN, HIGH);
    delayMicroseconds(10);
}

int16_t read16BitRegister(uint8_t regHigh) {
    uint8_t buffer[2];
    readRegisters(regHigh, buffer, 2);
    return (int16_t)((buffer[0] << 8) | buffer[1]);
}

bool initializeMPU9250() {
    Serial.println("Initializing MPU9250 via SPI...");
    uint8_t whoami = readRegister(MPU9250_WHO_AM_I);
    Serial.print("WHO_AM_I Registry: 0x");
    Serial.println(whoami, HEX);
    
    if (whoami != 0x71 && whoami != 0x73) {
        Serial.println("CRITICAL ERROR: MPU9250 sensor transaction failed.");
        return false;
    }
    
    writeRegister(MPU9250_PWR_MGMT_1, 0x80);  // Logic Device Reset
    delay(100);
    writeRegister(MPU9250_PWR_MGMT_1, 0x01);  // Auto-clock configuration
    delay(100);
    writeRegister(MPU9250_CONFIG, 0x03);       // Gyro/Accel Low Pass Filtering
    writeRegister(MPU9250_GYRO_CONFIG, 0x08);  // Scale allocation: ±500 dps
    writeRegister(MPU9250_ACCEL_CONFIG, 0x08); // Scale allocation: ±4g
    writeRegister(MPU9250_ACCEL_CONFIG2, 0x03);
    writeRegister(MPU9250_INT_PIN_CFG, 0x02);  // Enable register bypassing
    delay(100);
    return true;
}

void calibrateMPU9250(int samples = 500) {
    Serial.println("\nCalibration Active... Keep the sensor stationary!");
    delay(3000);
    long axSum = 0, aySum = 0, azSum = 0;
    long gxSum = 0, gySum = 0, gzSum = 0;
    
    for (int i = 0; i < samples; i++) {
        axSum += read16BitRegister(MPU9250_ACCEL_XOUT_H);
        aySum += read16BitRegister(MPU9250_ACCEL_XOUT_H + 2);
        azSum += read16BitRegister(MPU9250_ACCEL_XOUT_H + 4);
        gxSum += read16BitRegister(MPU9250_GYRO_XOUT_H);
        gySum += read16BitRegister(MPU9250_GYRO_XOUT_H + 2);
        gzSum += read16BitRegister(MPU9250_GYRO_XOUT_H + 4);
        delay(5);
    }
    
    accelOffsetZ = (azSum / samples) - 8192; // 1g offset reference at ±4g scale
    accelOffsetX = axSum / samples;
    accelOffsetY = aySum / samples;
    gyroOffsetX = gxSum / samples;
    gyroOffsetY = gySum / samples;
    gyroOffsetZ = gzSum / samples;
    Serial.println("Calibration completed.");
}

void readSensorData() {
    float ax = read16BitRegister(MPU9250_ACCEL_XOUT_H) - accelOffsetX;
    float ay = read16BitRegister(MPU9250_ACCEL_XOUT_H + 2) - accelOffsetY;
    float az = read16BitRegister(MPU9250_ACCEL_XOUT_H + 4) - accelOffsetZ;

    float accelX = ax / 8192.0;
    float accelY = ay / 8192.0;
    float accelZ = az / 8192.0;
    
    float pitchAcc = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / M_PI;
    float rollAcc = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180.0 / M_PI;

    Serial.print("Pitch: "); Serial.print(pitchAcc);
    Serial.print(" | Roll: "); Serial.println(rollAcc);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nMPU9250 Hardware SPI Interface Program");
    
    pinMode(MPU9250_CS_PIN, OUTPUT);
    digitalWrite(MPU9250_CS_PIN, HIGH);
    
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0)); // Initialize at 8 MHz
    
    if (!initializeMPU9250()) {
        while (1); // Halt system on missing hardware signature
    }
    
    calibrateMPU9250();
}

void loop() {
    readSensorData();
    delay(20);
}
```

---

## I2C

**I2C2** `PB10` / `PB11` pins are the same as USART3 pins. Therefore, you should not use USART3 when using I2C2. The reverse is also true; you cannot use I2C2 when using USART3. A conflict will occur.

The default I2C support is I2C1. If you want to use a different I2C pin, you must define the pins in your Arduino program as follows. For example enable I2C2:

```cpp

#define WIRE_SCL_PIN PB10
#define WIRE_SDA_PIN PB11

```

| Peripheral | Port Pin | Function | Mode |
| :--- | :--- | :--- | :--- |
| **I2C1**  | `PB6` / `PB7` | I2C1_SCL / I2C1_SDA | Default Configuration |
| **I2C2**  | `PB10` / `PB11` | I2C2_SCL / I2C2_SDA | Default Configuration |
| **I2C3**  | `PA8` / `PC9` | I2C3_SCL / I2C3_SDA | Default Configuration |


## ⚖️ License
This project is contributed to the open-source electronics community under the MIT License.
