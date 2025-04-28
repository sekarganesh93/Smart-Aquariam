# Smart-Aquariam
Smart Aquarium Controller
 Wiring Details

Device/Module	ESP32 GPIO Pin	Notes
Water Temp Sensor (DS18B20)	GPIO21	Use 4.7kΩ pull-up resistor between Data and VCC
Fan (via relay/MOSFET)	GPIO4	Controls fan automatically/manual
Light (via relay/MOSFET)	GPIO5	Controls lighting based on time/manual
Filter (via relay/MOSFET)	GPIO13	Controls filter based on time/manual
Push Button 1 (on-board)	GPIO0	Used for test mode/manual activation
TFT Display	Built-in	SPI wired internally
Power	5V via USB	Or external regulated 5V supply
⚡ Important: Use relay modules or MOSFET driver boards when controlling motors, fans, lights (do not drive high current directly from ESP32).

⚙️ Hardware Notes
DS18B20 Temp Sensor:

Data -> GPIO21

Pull-up resistor (4.7kΩ) needed between Data and 3.3V

GND -> GND

VCC -> 3.3V

Relays / MOSFETs:

Connect control pins of relays/MOSFETs to respective GPIOs (4, 5, 13).

Drive external 5V/12V load safely.

📜 Project Overview
Device: LilyGO T-Display-S3 (ESP32-S3 + 1.9" TFT display)

Functionality:

Monitor water temperature using DS18B20.

Automatically control:

Fan (based on temperature).

Light (based on time: 7 AM to 10 PM ON).

Filter (based on time: 7 AM to 10 PM ON).

Display real-time information:

Current time (12-hour format with AM/PM).

Date.

Water Temperature (°C).

Fan, Light, Filter Status (ON/OFF, ON-Test).

Manual test mode using onboard button:

Press button → all outputs ON for 10 seconds.

Display shows "ON-Test" during manual activation.

🖥️ TFT Display Layout

Display Area	Information
Top Left	Current Time (12-hr format)
Below Time	Current Date (DD-MM-YYYY)
Middle	Water Temperature (°C)
Below	Cooling Fan Status (ON / OFF / ON-Test)
Lower	Light Status (ON / OFF / ON-Test)
Bottom	Filter Status (ON / OFF / ON-Test)
🛠️ Software Libraries Used
WiFi.h — Connect ESP32 to Wi-Fi.

OneWire.h — Communicate with DS18B20 sensors.

DallasTemperature.h — Handle DS18B20 sensor reading.

TFT_eSPI.h — Manage TFT display drawing.

time.h — Fetch NTP time for RTC.

⏰ Time Configuration
Timezone: GMT+5:30 (India Standard Time)

NTP Server: pool.ntp.org

Auto-sync time on boot via internet.

🔥 Key Software Behavior
Fan turns ON if water temp > 26°C.

Fan turns OFF if water temp < 24°C.

Light and Filter turn ON between 7:00 AM and 10:00 PM, otherwise OFF.

Manual Test Mode:

Activated by pressing onboard Button 1 (GPIO0).

Turns ON Fan, Light, and Filter for 10 seconds.

Updates TFT to show ON-Test instead of regular ON.

⚡ Important Changes Implemented
Updated to correct GPIOs available on LilyGO T-Display-S3 (GPIO4, 5, 13 used instead of unavailable 22, 23, 19).

Water Temp label updated (from Aquarium Temp).

Colon (:) UI bug fixed after auto-return.

Test mode timing increased to 10 seconds.

Proper dynamic status update without extra colons.

🛡️ Future Enhancements (Optional Ideas)
Control the system remotely via Web Server or Android App.

Add alarm if water temperature goes too high/low.

Enable manual ON/OFF from mobile phone.

Display Wi-Fi signal strength.

Save settings in EEPROM/Flash.
