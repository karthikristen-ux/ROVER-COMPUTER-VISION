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

## 3. Running the System

1. Power on the Rover. 
2. The ESP32 will create a WiFi network called **`ROVER_WIFI`** (Password: `12345678`).
3. Connect your **Laptop/PC** to `ROVER_WIFI`.
4. Open a terminal on your PC, navigate to `pc_backend/`, and install requirements if you haven't:
   ```bash
   pip install flask opencv-python requests ultralytics
   ```
5. Run the backend:
   ```bash
   python app.py
   ```
6. Take your **Mobile Phone** and connect to **`ROVER_WIFI`**.
7. Open the browser on your phone and go to: `http://<YOUR_PC_IP>:5000` (The PC terminal will print out the correct IP when you run it).
8. You will see the YOLO video feed and a touch joystick. Enjoy!
