import cv2
import requests
import time
from ultralytics import YOLO

# Network Settings
ROVER_IP = "http://192.168.4.1"
CAM_URL = "http://192.168.4.10:81/stream"

def send_command(command):
    """Sends a steering command to the Rover ESP32."""
    try:
        # Send an HTTP GET request just like the web dashboard does
        requests.get(f"{ROVER_IP}/cmd?dir={command}", timeout=1.0)
        print(f"--> Sent command: {command}")
    except requests.exceptions.RequestException as e:
        print(f"--> Failed to send command: {e}")

def main():
    print("Loading YOLOv8 AI model...")
    # Load the incredibly fast, lightweight YOLOv8 nano model
    model = YOLO('yolov8n.pt') 

    print(f"Connecting to live video stream at {CAM_URL}...")
    cap = cv2.VideoCapture(CAM_URL)
    
    if not cap.isOpened():
        print("\nERROR: Could not connect to the camera stream.")
        print("Please ensure your laptop is connected to 'ROVER_WIFI'.")
        return

    print("\n[ AI Auto-Pilot Active ] - Press 'q' to quit.")

    last_stop_time = 0
    COOLDOWN = 1.5  # Wait 1.5 seconds before spamming the STOP command again

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame. Reconnecting...")
            time.sleep(1)
            cap = cv2.VideoCapture(CAM_URL)
            continue

        # Run YOLO object detection on the current frame
        results = model(frame, stream=True, verbose=False)
        
        obstacle_detected = False

        for result in results:
            boxes = result.boxes
            for box in boxes:
                # Get the bounding box coordinates
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = box.conf[0]
                cls = int(box.cls[0])
                class_name = model.names[cls]

                # If the AI is more than 30% sure it's an object, consider it an obstacle
                # Lowering this makes it much more sensitive!
                if conf > 0.30:
                    obstacle_detected = True
                    
                    # If it's a person, draw a green box. Otherwise, red.
                    color = (0, 255, 0) if class_name == 'person' else (0, 0, 255)
                    
                    # Draw a targeting box around the object
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 3)
                    
                    # Write the name of the object above the box
                    label = f"{class_name} {conf:.0%}"
                    cv2.putText(frame, label, (x1, max(y1 - 10, 0)), cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)

        # If an obstacle is on screen, tell the rover to STOP!
        if obstacle_detected:
            current_time = time.time()
            if current_time - last_stop_time > COOLDOWN:
                print(f"\n[!] OBSTACLE DETECTED in path! Hitting the brakes.")
                send_command('S')
                last_stop_time = current_time

        # Display the live high-tech video feed
        cv2.imshow("Rover AI Vision", frame)

        # Press 'q' on the keyboard to exit the program safely
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Cleanup when closing
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
