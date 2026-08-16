import cv2
import threading
import time
import requests
import os
from flask import Flask, Response, jsonify, send_file, request
from ultralytics import YOLO

app = Flask(__name__)

# --- Configuration ---
# IP of the ESP32-CAM (set to static 192.168.4.10 in firmware)
CAMERA_URL = "http://192.168.4.10:81/stream"

# IP of the ESP32 Rover Controller (default AP IP)
ROVER_CMD_URL = "http://192.168.4.1/cmd"

MODEL_PATH = "yolov8n.pt"

# --- Global Variables ---
running = True
latest_frame = None
model = None

# --- Initialize YOLO ---
print(f"Loading YOLO model from {MODEL_PATH}...")
try:
    model = YOLO(MODEL_PATH)
    print("YOLO model loaded successfully.")
except Exception as e:
    print(f"Failed to load YOLO model: {e}")

# --- Video Processing Thread ---
def video_capture_loop():
    global latest_frame, running
    print(f"Connecting to ESP32-CAM at {CAMERA_URL}...")
    
    # Use OpenCV to capture the MJPEG stream
    cap = cv2.VideoCapture(CAMERA_URL)
    
    # Retry mechanism if camera is not available immediately
    while not cap.isOpened() and running:
        print("Waiting for camera...")
        time.sleep(2)
        cap.open(CAMERA_URL)

    print("Camera connected!")

    while running:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame. Reconnecting...")
            cap.release()
            time.sleep(1)
            cap.open(CAMERA_URL)
            continue
        
        # Run YOLO inference
        if model:
            results = model(frame, verbose=False)
            # Plot the results on the frame
            annotated_frame = results[0].plot()
        else:
            annotated_frame = frame

        # Update global frame
        latest_frame = annotated_frame
        
        # Small sleep to yield CPU and limit FPS
        time.sleep(0.03)
        
    cap.release()
    print("Video capture stopped.")

# --- Flask Routes ---
@app.route("/")
def index():
    dash_path = os.path.join(os.path.dirname(__file__), "dashboard.html")
    if os.path.exists(dash_path):
        return send_file(dash_path)
    return "<h1>Dashboard missing</h1>"

def generate_video_stream():
    global latest_frame, running
    while running:
        if latest_frame is not None:
            # Encode frame to JPEG
            ret, buffer = cv2.imencode('.jpg', latest_frame, [cv2.IMWRITE_JPEG_QUALITY, 70])
            if ret:
                frame_bytes = buffer.tobytes()
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
        else:
            time.sleep(0.1)
        time.sleep(0.05) # ~20 FPS

@app.route("/video_feed")
def video_feed():
    return Response(generate_video_stream(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route("/api/rover_cmd")
def rover_cmd():
    # Forward the command to the ESP32
    dir = request.args.get("dir")
    if not dir:
        return jsonify({"status": "error", "message": "Missing dir parameter"}), 400
    
    try:
        esp32_url = f"{ROVER_CMD_URL}?dir={dir}"
        response = requests.get(esp32_url, timeout=2)
        if response.status_code == 200:
            return jsonify({"status": "ok", "message": f"Command {dir} sent"})
        else:
            return jsonify({"status": "error", "message": f"ESP32 returned {response.status_code}"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route("/api/flash_toggle")
def flash_toggle():
    """Proxy flash toggle request to ESP32-CAM on port 82."""
    try:
        response = requests.get("http://192.168.4.10:82/flash", timeout=2)
        return Response(response.content, content_type="application/json")
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route("/api/battery")
def battery():
    """Return battery level. Currently simulated — upgrade to real ADC later."""
    # TODO: Replace with real ADC reading from ESP32 when voltage divider is wired
    return jsonify({"level": 85, "voltage": "7.4V"})

# --- Startup ---
if __name__ == "__main__":
    # Start video processing in a background thread
    t = threading.Thread(target=video_capture_loop, daemon=True)
    t.start()
    
    # Run Flask app, listen on all interfaces so mobile phone can connect
    print("Starting server on 0.0.0.0:5000")
    print("Connect your phone to 'ROVER_WIFI' and go to http://<YOUR_PC_IP>:5000")
    try:
        app.run(host="0.0.0.0", port=5000, threaded=True)
    except KeyboardInterrupt:
        running = False
        print("Shutting down...")
