# primetime
are you living on a prime numbered day of your life?

## What It Does

- Tracks up to 4 people's birthdates
- Calculates the number of days each person has been alive
- Lights up their dedicated LED when that number is prime
- Each LED stays on for the entire prime day
- Includes a status LED for system health monitoring
- Automatically syncs time via NTP (Network Time Protocol)
- Reconnects to WiFi automatically if connection drops

<img width="328" height="247" alt="Screenshot 2026-02-13 at 12 59 45 PM" src="https://github.com/user-attachments/assets/7f590335-ce62-4b0e-b5d1-1619a856dc82" />

## Hardware Required

### Core Components
- **ESP8266 microcontroller** (choose one):
  - NodeMCU v1.0
  - Wemos D1 Mini
  - Any ESP8266-based board with at least 5 available GPIO pins

### LEDs & Resistors
- **5x LEDs** (any color you prefer):
  - 4 for family members
  - 1 for status indicator
- **5x 220-330Ω resistors** (one for each LED)
- **Breadboard** (or perfboard for permanent installation)
- **Jumper wires**

### Power Supply
- **Micro USB cable** (for programming and power)
- **USB power adapter** (5V, 1A minimum) for permanent installation

## 🔌 Wiring Diagram

```
ESP8266 (NodeMCU)          Component
─────────────────          ─────────
D1 (GPIO5)  ──[220Ω]────── LED1 (Person 1) ──── GND
D2 (GPIO4)  ──[220Ω]────── LED2 (Person 2) ──── GND
D5 (GPIO14) ──[220Ω]────── LED3 (Person 3) ──── GND
D6 (GPIO12) ──[220Ω]────── LED4 (Person 4) ──── GND
D7 (GPIO13) ──[220Ω]────── LED5 (Status)   ──── GND
```

### LED Connection Steps
1. Connect the **long leg (anode)** of each LED to a 220Ω resistor
2. Connect the resistor to the corresponding GPIO pin
3. Connect the **short leg (cathode)** of each LED to GND
4. Double-check polarity before powering on!

## 💻 Software Setup

### Prerequisites

1. **Arduino IDE** (1.8.x or 2.x)
   - Download from: https://www.arduino.cc/en/software

2. **ESP8266 Board Support**
   - Open Arduino IDE
   - Go to `File` → `Preferences`
   - Add to "Additional Board Manager URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to `Tools` → `Board` → `Board Manager`
   - Search for "esp8266" and install "esp8266 by ESP8266 Community"

3. **Required Libraries** (install via Library Manager):
   - `NTPClient` by Fabrice Weinberg
   - `TimeLib` by Michael Margolis
   - `ESP8266WiFi` (included with ESP8266 board package)

### Installing Libraries
1. Open Arduino IDE
2. Go to `Sketch` → `Include Library` → `Manage Libraries`
3. Search for and install:
   - "NTPClient"
   - "Time" or "TimeLib"

## ⚙️ Configuration

### 1. WiFi Settings
```cpp
const char* ssid = "YOUR_WIFI_SSID";        // Your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### 2. Timezone Configuration
```cpp
const long TIMEZONE_OFFSET = -7 * 3600; // GMT-7 (Pacific Time)
```
Find your timezone offset:
- **PST (Pacific)**: `-8 * 3600` or `-7 * 3600` (DST)
- **MST (Mountain)**: `-7 * 3600` or `-6 * 3600` (DST)
- **CST (Central)**: `-6 * 3600` or `-5 * 3600` (DST)
- **EST (Eastern)**: `-5 * 3600` or `-4 * 3600` (DST)
- **GMT/UTC**: `0`
- **CET (Central Europe)**: `1 * 3600` or `2 * 3600` (DST)

[Full list of UTC offsets](https://en.wikipedia.org/wiki/List_of_UTC_offsets)

### 3. Family Member Configuration
```cpp
FamilyMember family[4] = {
  {"Aba",   1990, 1, 15, D1, false, 0},  // Name, Year, Month, Day, Pin
  {"Bobo",     1992, 6, 22, D2, false, 0},
  {"Coco", 2015, 9, 8,  D5, false, 0},
  {"DeeDee",   2018, 12, 3, D6, false, 0}
};
```
### 4. LED Brightness (Optional)
```cpp
const int LED_BRIGHTNESS = 40;  // 0-1023 (1023 = full brightness)
```
- **40** = ~4% brightness (wasy on eyes)
- **100** = ~10% brightness
- **512** = ~50% brightness
- **1023** = 100% brightness

### 5. Test Mode
```cpp
const bool TEST_MODE = false;  // Set to true to test all LEDs
```
When `TEST_MODE = true`, the system cycles through each LED (including status LED) every second.

## Uploading the Code

1. Connect your ESP8266 to your computer via USB
2. Open `primetime.ino` in Arduino IDE
3. Select your board:
   - `Tools` → `Board` → `ESP8266 Boards` → `NodeMCU 1.0 (ESP-12E Module)` (or your board)
4. Select the correct port:
   - `Tools` → `Port` → (select your USB port)
5. Click the **Upload** button (→)
6. Wait for "Done uploading" message

## Testing & Debugging

### Serial Monitor
1. Open `Tools` → `Serial Monitor`
2. Set baud rate to **115200**
3. You should see:
   - WiFi connection status
   - Network scan results
   - Time sync confirmation
   - Current prime day status for each person
   - System status updates

### Test Mode
1. Set `TEST_MODE = true` in the code
2. Upload the code
3. Watch each LED light up in sequence (1 second each)
4. Verify all connections are working
5. Set `TEST_MODE = false` and re-upload for normal operation

### Status LED Indicators
- **Solid ON**: System operational, WiFi connected
- **Blinking**: WiFi disconnected, attempting to reconnect

## Understanding Prime Days

A prime number is a number greater than 1 that can only be divided by 1 and itself. For example:
- 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37...

The system calculates days since birth starting from day 0 (birth date). So:
- Day 0 = Birth date (not prime)
- Day 1 = 1st day after birth (not prime)
- Day 2 = 2nd day after birth (PRIME! ✨)
- Day 3 = 3rd day after birth (PRIME! ✨)
- Day 4 = 4th day after birth (not prime)
- Day 5 = 5th day after birth (PRIME! ✨)

## Expected Prime Day Frequency

As you age, prime days become rarer:
- **First year**: ~61 prime days
- **Age 10**: ~1 prime day every 5-6 days
- **Age 30**: ~1 prime day every 10-11 days
- **Age 60**: ~1 prime day every 14-15 days
- **Age 90**: ~1 prime day every 17-18 days

## Troubleshooting

### WiFi Won't Connect
- Verify SSID and password are correct (case-sensitive!)
- Check that your router supports 2.4GHz (ESP8266 doesn't support 5GHz)
- Ensure router is within range
- Check serial monitor for connection error codes

### LEDs Don't Light Up
- Verify wiring and LED polarity (long leg = positive)
- Check resistor values (220-330Ω)
- Test with `TEST_MODE = true`
- Measure voltage with multimeter (should be ~3.3V on GPIO pins)

### Wrong Time/Date
- Verify timezone offset is correct
- Check that ESP8266 can access internet (required for NTP)
- Wait a few minutes for initial time sync

### Compilation Errors
- Verify all libraries are installed
- Check that ESP8266 board package is installed
- Try updating to latest library versions

## Advanced Modifications

### Change NTP Server
```cpp
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", TIMEZONE_OFFSET, NTP_UPDATE_INTERVAL);
```
Use regional NTP servers for faster sync:
- North America: `us.pool.ntp.org`
- Europe: `europe.pool.ntp.org`
- Asia: `asia.pool.ntp.org`

### Add More Family Members
Currently limited to 4 members due to available GPIO pins. To expand:
1. Use GPIO expander (MCP23017)
2. Use ESP32 instead (more GPIO pins)
3. Use multiplexing

### Change Update Frequency
```cpp
delay(60000); // Check every minute (default)
delay(10000); // Check every 10 seconds (more responsive but higher power usage)
```

## Acknowledgements
- NTP library by Fabrice Weinberg
- TimeLib by Michael Margolis
- [Don Zagier](https://people.mpim-bonn.mpg.de/zagier/files/doi/10.1007/BF03039306/fulltext.pdf)
- [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)

---

