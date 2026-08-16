# AI Rover Project (YOLO + ESP32)

This project turns an ESP32 and ESP32-CAM into a remote-controlled rover with computer vision powered by YOLOv8.

## Project Structure
- `esp32_rover_controller/`: Arduino code for the ESP32 that controls the motors and hosts the `ROVER_WIFI` access point.
- `esp32_cam_streamer/`: Arduino code for the ESP32-CAM that connects to the WiFi and streams video.
- `pc_backend/`: Python Flask backend that connects to the video stream, runs YOLOv8, and hosts the mobile dashboard.

---

## 1. Hardware Setup & Wiring

### Components Needed:
1. ESP32 (Controller)
2. ESP32-CAM (Vision)
3. HW-095 Motor Driver (L298N based)
4. 2x BO Motors + Chassis
5. Battery (e.g. 2x 18650) to power the HW-095 and ESP32s.

### HW-095 Motor & Power Wiring (Screw Terminals)
- **OUT1 & OUT2** -> Left Motor (Connect the two motor wires here. Swap them if the motor spins backward)
- **OUT3 & OUT4** -> Right Motor (Connect the two motor wires here. Swap them if the motor spins backward)
- **12V (VCC)** -> Battery Positive (+) (e.g., 7.4V - 12V)
- **GND** -> Battery Negative (-) AND ESP32 GND
- **5V** -> ESP32 5V/VIN pin (This powers the ESP32 from the motor driver's regulator)

### HW-095 Control Wiring (Header Pins to ESP32)
- **IN1** -> ESP32 GPIO **26**
- **IN2** -> ESP32 GPIO **27**
- **IN3** -> ESP32 GPIO **14**
- **IN4** -> ESP32 GPIO **12**

### ESP32-CAM Wiring
- Just provide 5V and GND. Mount it facing forward on the rover.

---

## 2. Firmware Installation

1. **ESP32 Rover Controller**: 
   - Open `esp32_rover_controller/esp32_rover_controller.ino` in the Arduino IDE.
   - Select your ESP32 board and flash.
2. **ESP32-CAM Streamer**:
   - Open `esp32_cam_streamer/esp32_cam_streamer.ino` in the Arduino IDE.
   - Select "AI Thinker ESP32-CAM" and flash.

---

## 3. Running the System on a PC / Laptop

If you are using a new laptop or want to run the AI processing on a different computer, follow these steps:

1. **Clone the code to your PC**:
   Bring this project folder onto the new PC (via GitHub, USB drive, or email).
2. **Power on the Rover**: 
   Turn on the main ESP32 (wait 3 seconds) then the ESP32-CAM. 
   The ESP32 will create a WiFi network called **`ROVER_WIFI`** (Password: `12345678`).
3. **Connect your PC to the Rover**:
   Connect your PC's WiFi to `ROVER_WIFI`.
4. **Install Python & Dependencies**:
   Open a terminal/command prompt in the `pc_backend` folder on the PC:
   ```bash
   pip install flask opencv-python requests ultralytics
   ```
5. **Run the AI Backend**:
   ```bash
   python app.py
   ```
   *Note: This terminal must stay open. The PC acts as the brain processing the video!*
6. **Access the Dashboard on the PC**:
   Open a browser on that PC and go to: `http://localhost:5000`

---

## 4. Controlling from your Mobile Phone

The PC runs the heavy AI (YOLO). Your phone acts as a lightweight remote control screen.

1. **Find your PC's IP Address**:
   On your PC (while connected to `ROVER_WIFI`), open a new terminal and type:
   - **Windows**: `ipconfig`
   - **Mac/Linux**: `ifconfig`
   Look for the IPv4 Address (it will likely be `192.168.4.x`, for example `192.168.4.4`).
2. **Connect Phone to WiFi**:
   Connect your phone to the same **`ROVER_WIFI`** network.
3. **Open the Dashboard**:
   Open Chrome/Safari on your phone and type the PC's IP with port 5000. 
   Example: `http://192.168.4.4:5000`

Enjoy your professional Military-Grade Rover Cockpit!

### Troubleshooting
- **Error 128 / Video Feed Offline**: Unplug the ESP32-CAM, restart the `app.py` script on the PC, plug the ESP32-CAM back in, and press the RESET button on the ESP32-CAM board. Make sure the main ESP32 is powered *before* the CAM.
