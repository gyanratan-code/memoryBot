# ToFroBot

MemoryBot is an ESP8266-based robot control system that uses a web interface and WebSocket communication to send movement commands. The robot records movement commands and can even perform a reverse (path retrace) operation based on its recorded path.

## Overview

MemoryBot sets up a WiFi Access Point (AP) using the ESP8266 module and serves a control webpage that lets you command the robot. The control page displays directional buttons (forward, backward, left, right, stop) and a reverse button that triggers a retrace of the robot’s path. Communication between the webpage and the robot is handled through a WebSocket connection, ensuring real-time responsiveness.

## Features

- **WiFi Soft AP Mode:** The ESP8266 creates its own WiFi network (SSID: `FDP_Group:15`, Password: `xxxxxxx`).
- **Asynchronous Web Server:** Hosts a control webpage for sending commands.
- **WebSocket Communication:** Provides low-latency bi-directional communication.
- **Robot Movement Control:** Supports commands for moving forward, backward, left, right, and stopping.
- **Path Retrace:** Implements a “reverse” functionality to retrace the robot's path.

## Prerequisites

- **Hardware:**
  - ESP8266 module (e.g., NodeMCU, Wemos D1 Mini)
  - Motor driver compatible with ESP8266 (check wiring and pin configurations)
  - Motors and power supply as required for your robot

- **Software/Libraries:**
  - [ESP8266WiFi](https://github.com/esp8266/Arduino) library
  - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) library
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) library
  - [Vector Library](#) (ensure that this dependency is added if not part of your standard libraries)

## Installation

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/gyanratan-code/memoryBot.git
   cd memoryBot
   ```

2. **Set Up the Arduino IDE (or PlatformIO):**
   - Install the ESP8266 board support.
   - Install the required libraries using the Library Manager or manually add them to your Arduino libraries folder.

3. **Upload the Code:**
   - Open `code.ino` in your Arduino IDE.
   - Connect your ESP8266 board.
   - Compile and upload the sketch to your board.

## Usage

1. **Power On & Connect:**
   - Once the ESP8266 boots, it creates a WiFi network named `FDP_Group:15`.
   - Connect to the network using the password `xxxxxxxx`.

2. **Access the Control Interface:**
   - Open a web browser and navigate to the IP address `http://192.168.4.22/` (this is set in the sketch).

3. **Control the Robot:**
   - Use the on-screen buttons to send directional commands:
     - **Forward, Backward, Left, Right, Stop**
   - Press the **Reverse** button to trigger the path retrace function (note: controls will be disabled until the reverse operation is complete).

4. **WebSocket Feedback:**
   - The robot sends back status messages (such as “Path retrace done”) which appear on the control page.

## Code Overview

- **ESP8266 Setup:**  
  Configures the ESP8266 as a WiFi access point with a fixed IP (`192.168.4.22`).

- **Web Server & WebSocket:**  
  Uses the ESPAsyncWebServer library to serve a control HTML page and set up a WebSocket at `/ws` for real-time communication.

- **Robot Control:**  
  The sketch defines various movement functions corresponding to different commands (forward, backward, etc.) and maps these to WebSocket messages.

- **HTML Interface:**  
  The embedded HTML (served from program memory) includes an image map for directional control and JavaScript to handle WebSocket events.

## Customization

- **Pin Configuration:**  
  The motor and control pins are defined at the beginning of the code. Adjust these as per your hardware setup.

- **WiFi Credentials:**  
  Modify the SSID, password, and network settings in the code if you need a different network configuration.

- **Movement Parameters:**  
  Variables like `rotation_time` can be adjusted to fine-tune the robot’s movements.

## Contributing

If you have suggestions or improvements, feel free to fork the repository and submit a pull request. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is open source. You can modify and redistribute it according to the terms of your chosen license (e.g., MIT License).
