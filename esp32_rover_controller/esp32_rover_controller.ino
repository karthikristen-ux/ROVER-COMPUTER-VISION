#include <WiFi.h>
#include <WebServer.h>

// --- WiFi Settings ---
const char *ssid = "ROVER_WIFI";
const char *password = "12345678"; // Min 8 characters

WebServer server(80);

// --- Motor Pins (L298N) ---
// Left Motor
const int IN1 = 26;
const int IN2 = 27;
// Right Motor
const int IN3 = 14;
const int IN4 = 13;

// --- PWM Settings ---
// Using LEDC PWM to get full power out of the L298N with 3.3V logic
const int FREQ      = 1000; // 1 kHz PWM frequency
const int RESOLUTION = 8;   // 8-bit (0-255)
const int SPEED     = 255;  // Full speed

// --- Function Prototypes ---
void handleRoot();
void handleCommand();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();

void setup() {
  Serial.begin(115200);

  // Attach PWM to each motor pin
  ledcAttach(IN1, FREQ, RESOLUTION);
  ledcAttach(IN2, FREQ, RESOLUTION);
  ledcAttach(IN3, FREQ, RESOLUTION);
  ledcAttach(IN4, FREQ, RESOLUTION);

  stopMotors(); // Ensure motors are off at start
  Serial.println("Motor PWM initialized.");

  // Setup WiFi Access Point
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Setup Web Server Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);

  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Type F, B, L, R, or S to test motors.");
}

void loop() {
  server.handleClient();

  // Manual control via Serial Monitor for debugging
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    cmd = toupper(cmd);

    if (cmd == 'F') {
      Serial.println("Serial Command: F");
      moveForward();
    } else if (cmd == 'B') {
      Serial.println("Serial Command: B");
      moveBackward();
    } else if (cmd == 'L') {
      Serial.println("Serial Command: L");
      turnLeft();
    } else if (cmd == 'R') {
      Serial.println("Serial Command: R");
      turnRight();
    } else if (cmd == 'S') {
      Serial.println("Serial Command: S");
      stopMotors();
    }
  }
}

// --- Motor Control Functions (PWM for full power) ---
void moveForward() {
  Serial.println("Action: Moving FORWARD");
  ledcWrite(IN1, SPEED);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, SPEED);
  ledcWrite(IN4, 0);
}

void moveBackward() {
  Serial.println("Action: Moving BACKWARD");
  ledcWrite(IN1, 0);
  ledcWrite(IN2, SPEED);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, SPEED);
}

void turnLeft() {
  Serial.println("Action: Turning LEFT");
  ledcWrite(IN1, 0);
  ledcWrite(IN2, SPEED);
  ledcWrite(IN3, SPEED);
  ledcWrite(IN4, 0);
}

void turnRight() {
  Serial.println("Action: Turning RIGHT");
  ledcWrite(IN1, SPEED);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, SPEED);
}

void stopMotors() {
  Serial.println("Action: STOPPING Motors");
  ledcWrite(IN1, 0);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, 0);
}

void handleRoot() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Rover Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background: #1a1a2e;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      font-family: Arial, sans-serif;
      color: white;
    }
    h1 { margin-bottom: 10px; font-size: 24px; color: #00d4ff; }
    #status { margin-bottom: 15px; font-size: 14px; color: #aaa; }
    #cam {
      width: 320px;
      height: 240px;
      background-color: #000;
      border: 3px solid #00d4ff;
      border-radius: 8px;
      margin-bottom: 20px;
      box-shadow: 0 4px 10px rgba(0,212,255,0.2);
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(3, 90px);
      grid-template-rows: repeat(3, 90px);
      gap: 10px;
    }
    .btn {
      background: #16213e;
      border: 2px solid #00d4ff;
      border-radius: 12px;
      color: #00d4ff;
      font-size: 28px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: background 0.15s;
      user-select: none;
      -webkit-tap-highlight-color: transparent;
    }
    .btn:active { background: #00d4ff; color: #1a1a2e; }
    .stop {
      background: #3d0000;
      border-color: #ff4444;
      color: #ff4444;
      font-size: 16px;
      font-weight: bold;
    }
    .stop:active { background: #ff4444; color: white; }
    .empty { visibility: hidden; }
  </style>
</head>
<body>
  <h1>&#128665; Rover Control</h1>
  <div id="status">Connected to ROVER_WIFI</div>
  
  <img id="cam" src="http://192.168.4.10:81/stream" alt="Camera Stream Loading...">

  <div class="grid">
    <div class="empty"></div>
    <button class="btn" id="btnF">&#8593;</button>
    <div class="empty"></div>

    <button class="btn" id="btnL">&#8592;</button>
    <button class="btn stop" id="btnS">STOP</button>
    <button class="btn" id="btnR">&#8594;</button>

    <div class="empty"></div>
    <button class="btn" id="btnB">&#8595;</button>
    <div class="empty"></div>
  </div>

  <script>
    function send(dir) {
      fetch('/cmd?dir=' + dir)
        .then(r => r.text())
        .then(t => { document.getElementById('status').innerText = 'Command: ' + dir + ' -> ' + t; })
        .catch(() => { document.getElementById('status').innerText = 'Connection lost!'; });
    }

    function setupBtn(id, dir) {
      const btn = document.getElementById(id);
      
      const press = (e) => { e.preventDefault(); send(dir); };
      const release = (e) => { e.preventDefault(); send('S'); };

      // Touch events for mobile
      btn.addEventListener('touchstart', press, {passive: false});
      btn.addEventListener('touchend', release, {passive: false});
      btn.addEventListener('touchcancel', release, {passive: false});

      // Mouse events for laptop
      btn.addEventListener('mousedown', press);
      btn.addEventListener('mouseup', release);
      btn.addEventListener('mouseleave', release);
      
      // Prevent context menu holding on mobile
      btn.addEventListener('contextmenu', e => e.preventDefault());
    }

    setupBtn('btnF', 'F');
    setupBtn('btnB', 'B');
    setupBtn('btnL', 'L');
    setupBtn('btnR', 'R');
    
    // Stop button only sends S on press
    const stopBtn = document.getElementById('btnS');
    const stopAction = (e) => { e.preventDefault(); send('S'); };
    stopBtn.addEventListener('touchstart', stopAction, {passive: false});
    stopBtn.addEventListener('mousedown', stopAction);
  </script>
</body>
</html>
)rawhtml";
  server.send(200, "text/html", html);
}


void handleCommand() {
  if (server.hasArg("dir")) {
    String dir = server.arg("dir");
    Serial.println("Command received: " + dir);

    if (dir == "F") {
      moveForward();
    } else if (dir == "B") {
      moveBackward();
    } else if (dir == "L") {
      turnLeft();
    } else if (dir == "R") {
      turnRight();
    } else if (dir == "S") {
      stopMotors();
    } else {
      server.send(400, "text/plain", "Invalid command");
      return;
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'dir' parameter");
  }
}

