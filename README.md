
![logo](img/Small.png)

---
## NEW Update 2025-05-26
- **add Association Flood**
- **add Authentication Flood**
---
> The Association Flood attack only works on 2.4GHz
---
## Firmware Upload Guide

### BW16 Setup
1. Open **Arduino IDE**.
2. Click **File** > **Preferences**.
3. Add the following Additional Boards Manager URL:  
   `https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json`
4. Go to **Boards Manager** and search for **BW16**.
5. Download the **Realtek Amebaa Boards** (version 3.1.5) manager.
### ESP32 Setup
1. Open **Arduino IDE**.
2. Click **File** > **Preferences**.
3. Add the following Additional Boards Manager URL:  
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
4. Go to **Boards Manger** and search for **Esp32**.
5. Downlload the **esp32 by Espressif Sysytems** board manager.
### How to Upload Firmware
#### BW16
1. Open the BW16 folder in `BW16.ino` file in Arduino IDE.
2. Click on **Tools**.
3. Click **Board** and Select `Ai-Thinker BW16(RTL8720DN)`
4. Enable **Auto Upload Mode** and click **Enable**.
5. Enable **Standard Lib** and select **Arduino_STD_PRINTF**.
6. Click **Upload** to upload the firmware.
#### ESP32
1. Open the Esp32 folder in `.ino` file in Arduino IDE.
2. Click on **Tools**.
3. Click **Board** and Select `ESP32 Dev Module` (Your ESP32 Board)
4. Click **Partition Scheme** and Select **No OTA (2MB APP/2MB SPIFFS)**
5. Click **Upload** to upload the firmware.
> Note: The **Evil Twin** function is not compatible with the **ESP32-S3** board,  
> but using it may increase the effective range of **Sour Apple**.
---

## Requirements

- **SSD1306 Display**
- **4 Buttons**
- **BW16 Board**
- **ESP32 Board**
---

## Connections

### Buttons
- **Up Button**: `PA27`
- **Down Button**: `PA12`
- **Select Button**: `PA13`
- **Back Button**: `PB3`
> In this new version, you **must** connect the **Back Button**.

### SSD1306 Display
- **SDA**: `PA26`
- **SCL**: `PA25`
## ESP32
| I2C  | ESP32          | BW16      |
|------|----------------|-----------|
| SDA  | GPIO 21        | PA26      |
| SCL  | GPIO 22        | PA25      |
| GND  | GND            | GND       |
| 3V3  | 3V3            | 3V3       |
> If you don't need the **Evil Twin** function or the **Sour Apple** function,  
> simply leave them unconnected.
---
# Source
[Link](https://github.com/tesa-klebeband/RTL8720dn-Deauther)
## DISCLAIMER

This tool has been made for **educational** and **testing** purposes only. Any misuse or illegal activities conducted with the tool are strictly prohibited.  
I am **not** responsible for any consequences arising from the use of this tool, which is done at your own risk.

### Feedback
If you find any **issues** or have **suggestions**, please let me know on **Discord**: `warwick.320`
