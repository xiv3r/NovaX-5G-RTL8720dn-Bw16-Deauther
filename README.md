![logo](img/Small.png)

## ⭐ If you like this project, please give it a star!

---

## 🚀 NEW Update – 2025-05-26

- ✅ **Added Association Flood**
- ✅ **Added Authentication Flood**
- ✅ **Fixed reversed Up/Down buttons**
- ✅ **Added project logo**

> ⚠️ *Note:* Association Flood attack works only on **2.4GHz** networks.  
> 🔜 *Next update will include a **Web UI***.

---

## 🛠 Firmware Upload Guide

### 🔹 Uploading Firmware (Using Upload Tool)

1. First, [download the tool](https://github.com/warwick320/NovaX-5G-RTL8720dn-Bw16-Deauther/releases/tag/ImageTool_v2).
2. **Connect** your **BW16** board via USB.
3. Select the correct **COM port** for your BW16.
4. Click the **Start Firmware Upload** button.
5. ✅ Done!

---

### 🔹 BW16 Arduino Setup

1. Open **Arduino IDE**.
2. Go to **File > Preferences**.
3. Add the following URL to the **Additional Boards Manager URLs**: `https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json`
4. Open **Boards Manager**, search for **BW16**.
5. Install **Realtek Ameba Boards** (version **3.1.5**).

---

### 🔹 ESP32 Arduino Setup

1. Open **Arduino IDE**.
2. Go to **File > Preferences**.
3. Add the following URL to the **Additional Boards Manager URLs**: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   4. Open **Boards Manager**, search for **ESP32**.
5. Install **ESP32 by Espressif Systems**.

---

### 🔹 Uploading Firmware (Using Arduino IDE)

#### 📦 For BW16

1. Open the `BW16.ino` file in the **BW16** folder using Arduino IDE.
2. Go to **Tools > Board** and select `Ai-Thinker BW16 (RTL8720DN)`.
3. Enable **Auto Upload Mode**, then click **Enable**.
4. Enable **Standard Lib** and select `Arduino_STD_PRINTF`.
5. Click **Upload** to flash the firmware.

#### 📦 For ESP32

1. Open the `.ino` file in the **ESP32** folder using Arduino IDE.
2. Go to **Tools > Board** and select your board (e.g., `ESP32 Dev Module`).
3. Set **Partition Scheme** to `No OTA (2MB APP/2MB SPIFFS)`.
4. Click **Upload** to flash the firmware.

> ⚠️ **Note:** The **Evil Twin** function is **not supported** on the **ESP32-S3**,  
> but it may help extend the range of **Sour Apple** attacks.

---

## 📋 Requirements

- ✅ **SSD1306 OLED Display**  
- ✅ **4 Tactile Buttons**  
- ✅ **BW16 Board**  
- ✅ **ESP32 Board** *(optional)*

---

## 🔌 Hardware Connections

### 🎮 Buttons (BW16)

| Function | Pin  |
|----------|------|
| Up       | PA27 |
| Down     | PA12 |
| Select   | PA13 |
| Back     | PB3  |

> ⚠️ **Back Button is now mandatory in the latest version!**

---

### 📺 SSD1306 Display (I2C)

| Signal | BW16 Pin |
|--------|----------|
| SDA    | PA26     |
| SCL    | PA25     |

---

## 🔁 ESP32 ↔ BW16 I2C Wiring

| I2C Line | ESP32 Pin | BW16 Pin |
|----------|------------|-----------|
| SDA      | GPIO 21    | PA26      |
| SCL      | GPIO 22    | PA25      |
| GND      | GND        | GND       |
| 3V3      | 3V3        | 3V3       |

> ⚠️ If you are not using **Evil Twin** or **Sour Apple**, you can leave ESP32 disconnected.

---

## 📂 Source Code

[GitHub Repository](https://github.com/tesa-klebeband/RTL8720dn-Deauther)

---

## ⚠️ Disclaimer

This tool is intended for **educational** and **testing** purposes only.  
Any **misuse** of this tool for illegal activity is **strictly prohibited**.  
The author holds **no responsibility** for any consequences resulting from the use of this tool.

---

## 💬 Feedback & Support

If you find any **issues** or have **suggestions**, feel free to reach out via **Discord**:  
`warwick.320`


