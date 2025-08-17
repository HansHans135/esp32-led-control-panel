#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#define FILE_READ "r"
#define FILE_WRITE "w"
#ifdef __AVR__
 #include <avr/power.h>
#endif

#define LED_PIN    4
#define LED_COUNT 16*16
#define LED_WIDTH 16
#define LED_HEIGHT 16

ESP8266WebServer server(80);
WiFiManager wifiManager;

uint32_t ledColors[LED_COUNT];
bool needUpdate = false;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 0; padding: 20px; }
    #colorPicker { width: 100px; height: 50px; }
    #ledGrid { 
      display: grid;
      grid-template-columns: repeat(16, 1fr);
      gap: 1px;
      margin: 20px auto;
      touch-action: none;
      width: 100%;
      max-width: 500px;
    }
    .led {
      aspect-ratio: 1/1;
      background-color: #000;
      border: 1px solid #ccc;
      cursor: pointer;
    }
    .controls {
      margin: 20px auto;
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
    }
    button {
      padding: 10px;
      cursor: pointer;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
    }
    .preset {
      width: 30px;
      height: 30px;
      border: 1px solid #ccc;
      display: inline-block;
      cursor: pointer;
      margin: 2px;
    }
    .saveContainer {
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <h1>ESP8266 LED Control Panel</h1>
  
  <div class="controls">
    <input type="color" id="colorPicker" value="#FF0000">
    <button onclick="fillAll()">Fill All</button>
    <button onclick="clearAll()">Clear All</button>
  </div>
  
  <div class="presetColors">
    <div class="preset" style="background-color: #FF0000" onclick="setColor('#FF0000')"></div>
    <div class="preset" style="background-color: #00FF00" onclick="setColor('#00FF00')"></div>
    <div class="preset" style="background-color: #0000FF" onclick="setColor('#0000FF')"></div>
    <div class="preset" style="background-color: #FFFF00" onclick="setColor('#FFFF00')"></div>
    <div class="preset" style="background-color: #FF00FF" onclick="setColor('#FF00FF')"></div>
    <div class="preset" style="background-color: #00FFFF" onclick="setColor('#00FFFF')"></div>
    <div class="preset" style="background-color: #FFFFFF" onclick="setColor('#FFFFFF')"></div>
    <div class="preset" style="background-color: #000000" onclick="setColor('#000000')"></div>
  </div>
  
  <div id="ledGrid"></div>

  <script>
    const gridSize = 16;
    const grid = document.getElementById('ledGrid');
    const colorPicker = document.getElementById('colorPicker');
    let isDrawing = false;
    let currentColor = colorPicker.value;
    let ledStates = Array(gridSize * gridSize).fill('#000000');
    
    function initGrid() {
      grid.innerHTML = '';
      
      for (let y = 0; y < gridSize; y++) {
        for (let x = 0; x < gridSize; x++) {
          const i = y * gridSize + x;
          
          const led = document.createElement('div');
          led.className = 'led';
          led.dataset.index = i;
          led.dataset.x = x;
          led.dataset.y = y;
          led.style.backgroundColor = ledStates[i];
          
          led.addEventListener('mousedown', startDrawing);
          led.addEventListener('mouseover', draw);
          led.addEventListener('touchstart', handleTouch);
          led.addEventListener('touchmove', handleTouchMove);
          
          grid.appendChild(led);
        }
      }
      
      document.addEventListener('mouseup', stopDrawing);
      document.addEventListener('touchend', stopDrawing);
    }
    
    function startDrawing(e) {
      isDrawing = true;
      const index = e.target.dataset.index;
      setLedColor(index, currentColor);
    }
    
    function draw(e) {
      if (!isDrawing) return;
      const index = e.target.dataset.index;
      setLedColor(index, currentColor);
    }
    
    function stopDrawing() {
      if (isDrawing) {
        isDrawing = false;
        updateLeds();
      }
    }
    
    function handleTouch(e) {
      e.preventDefault();
      const touch = e.touches[0];
      const led = document.elementFromPoint(touch.clientX, touch.clientY);
      if (led && led.classList.contains('led')) {
        isDrawing = true;
        setLedColor(led.dataset.index, currentColor);
      }
    }
    
    function handleTouchMove(e) {
      e.preventDefault();
      if (!isDrawing) return;
      
      const touch = e.touches[0];
      const led = document.elementFromPoint(touch.clientX, touch.clientY);
      if (led && led.classList.contains('led')) {
        setLedColor(led.dataset.index, currentColor);
      }
    }
    
    function setLedColor(index, color) {
      ledStates[index] = color;
      const led = document.querySelector(`.led[data-index="${index}"]`);
      if (led) {
        led.style.backgroundColor = color;
      }
    }
    
    function setColor(color) {
      colorPicker.value = color;
      currentColor = color;
    }
    
    function fillAll() {
      for (let i = 0; i < ledStates.length; i++) {
        setLedColor(i, currentColor);
      }
      updateLeds();
    }
    
    function clearAll() {
      for (let i = 0; i < ledStates.length; i++) {
        setLedColor(i, '#000000');
      }
      updateLeds();
    }
    
    function updateLeds() {
      const colors = ledStates.map(color => {
        return parseInt(color.substring(1), 16);
      });
      fetch('/update', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ colors })
      })
      .then(response => response.json())
      .then(data => {
        console.log('Update successful:', data);
        fetch('/autosave', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ colors: ledStates })
        })
        .catch(error => console.error('Autosave error:', error));
      })
      .catch(error => console.error('Update error:', error));
    }
    
    colorPicker.addEventListener('change', (e) => {
      currentColor = e.target.value;
    });
    
    initGrid();
    
    fetch('/lastpattern')
      .then(response => response.json())
      .then(data => {
        if (data.colors && data.colors.length > 0) {
          ledStates = data.colors;
          for (let i = 0; i < ledStates.length; i++) {
            const led = document.querySelector(`.led[data-index="${i}"]`);
            if (led) {
              led.style.backgroundColor = ledStates[i];
            }
          }
        }
      })
      .catch(error => console.error('Load error:', error));
  </script>
</body>
</html>
)rawliteral";


void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleUpdateLeds() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }
  
  JsonArray colors = doc["colors"];
  
  if (colors.size() != LED_COUNT) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Color count mismatch\"}");
    return;
  }
  
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t color = colors[i];
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    ledColors[i] = strip.Color(r, g, b);
  }
  
  needUpdate = true;
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"LEDs updated\"}");
}

void updateLedStrip() {
  for (int y = 0; y < LED_HEIGHT; y++) {
    for (int x = 0; x < LED_WIDTH; x++) {
      int ledIndex = y * LED_WIDTH + x;
      int mirroredX = LED_WIDTH - 1 - x;
      int physicalLedIndex = getSnakeIndex(mirroredX, y);
      strip.setPixelColor(physicalLedIndex, ledColors[ledIndex]);
    }
  }
  strip.show();
}

void handleSavePattern() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }
  
  String name = doc["name"].as<String>();
  String filename = "/pattern_" + name + ".json";
  
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Unable to create file\"}");
    return;
  }
  
  if (serializeJson(doc, file) == 0) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Save failed\"}");
  } else {
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Pattern saved\"}");
  }
  
  file.close();
}

void handleListPatterns() {
  DynamicJsonDocument doc(1024);
  JsonArray patterns = doc.to<JsonArray>();
  
  Dir dir = SPIFFS.openDir("/");
  
  while (dir.next()) {
    String filename = dir.fileName();
    if (filename.startsWith("/pattern_") && filename.endsWith(".json")) {
      String name = filename.substring(9, filename.length() - 5);
      JsonObject pattern = patterns.createNestedObject();
      pattern["name"] = name;
    }
  }
  
  String response;
  serializeJson(patterns, response);
  server.send(200, "application/json", response);
}

void handleGetPattern() {
  String name = server.pathArg(0);
  String filename = "/pattern_" + name + ".json";
  
  if (!SPIFFS.exists(filename)) {
    server.send(404, "application/json", "{\"status\":\"error\",\"message\":\"Pattern not found\"}");
    return;
  }
  
  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Unable to read file\"}");
    return;
  }
  
  String content = file.readString();
  file.close();
  
  server.send(200, "application/json", content);
  
  DynamicJsonDocument doc(16384);
  deserializeJson(doc, content);
  
  JsonArray colors = doc["colors"];
  for (int i = 0; i < LED_COUNT && i < colors.size(); i++) {
    String colorStr = colors[i].as<String>();
    long color = strtol(colorStr.substring(1).c_str(), NULL, 16);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    ledColors[i] = strip.Color(r, g, b);
  }
  
  needUpdate = true;
}

void handleDeletePattern() {
  String name = server.pathArg(0);
  String filename = "/pattern_" + name + ".json";
  
  if (!SPIFFS.exists(filename)) {
    server.send(404, "application/json", "{\"status\":\"error\",\"message\":\"Pattern not found\"}");
    return;
  }
  
  if (SPIFFS.remove(filename)) {
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Pattern deleted\"}");
  } else {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Delete failed\"}");
  }
}

void handleAutoSave() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }
  
  File file = SPIFFS.open("/last_pattern.json", FILE_WRITE);
  if (!file) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Unable to create file\"}");
    return;
  }
  
  if (serializeJson(doc, file) == 0) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Save failed\"}");
  } else {
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Pattern saved\"}");
  }
  
  file.close();
}

void handleGetLastPattern() {
  if (!SPIFFS.exists("/last_pattern.json")) {
    server.send(200, "application/json", "{\"colors\":[]}");
    return;
  }
  
  File file = SPIFFS.open("/last_pattern.json", FILE_READ);
  if (!file) {
    server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Unable to read file\"}");
    return;
  }
  
  String content = file.readString();
  file.close();
  
  server.send(200, "application/json", content);
  
  DynamicJsonDocument doc(16384);
  deserializeJson(doc, content);
  
  JsonArray colors = doc["colors"];
  for (int i = 0; i < LED_COUNT && i < colors.size(); i++) {
    String colorStr = colors[i].as<String>();
    long color = strtol(colorStr.substring(1).c_str(), NULL, 16);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    ledColors[i] = strip.Color(r, g, b);
  }
  
  needUpdate = true;
}

void loadDefaultPattern() {
  if (SPIFFS.exists("/last_pattern.json")) {
    File file = SPIFFS.open("/last_pattern.json", FILE_READ);
    if (file) {
      String content = file.readString();
      file.close();
      
      DynamicJsonDocument doc(16384);
      DeserializationError error = deserializeJson(doc, content);
      
      if (!error) {
        JsonArray colors = doc["colors"];
        for (int i = 0; i < LED_COUNT && i < colors.size(); i++) {
          String colorStr = colors[i].as<String>();
          long color = strtol(colorStr.substring(1).c_str(), NULL, 16);
          uint8_t r = (color >> 16) & 0xFF;
          uint8_t g = (color >> 8) & 0xFF;
          uint8_t b = color & 0xFF;
          ledColors[i] = strip.Color(r, g, b);
        }
        
        needUpdate = true;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS mount failed!");
  } else {
    Serial.println("SPIFFS mount successful");
  }
  
  strip.begin();
  strip.show();
  strip.setBrightness(50);
  
  for (int i = 0; i < LED_COUNT; i++) {
    ledColors[i] = strip.Color(0, 0, 0);
  }
  
  wifiManager.setConfigPortalTimeout(180);
  
  if (!wifiManager.autoConnect("ESP8266_LED_Controller")) {
    Serial.println("Connection failed, restarting...");
    ESP.restart();
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdateLeds);
  server.on("/autosave", HTTP_POST, handleAutoSave);
  server.on("/lastpattern", HTTP_GET, handleGetLastPattern);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Using mirrored display to fix LED panel orientation");
  
  loadDefaultPattern();
}

void loop() {
  server.handleClient();
  
  if (needUpdate) {
    updateLedStrip();
    needUpdate = false;
  }
}


int getSnakeIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * LED_WIDTH + x;
  } else {
    return y * LED_WIDTH + (LED_WIDTH - 1 - x);
  }
}

int getXFromIndex(int index) {
  int y = index / LED_WIDTH;
  int x = index % LED_WIDTH;
  
  if (y % 2 == 1) {
    x = LED_WIDTH - 1 - x;
  }
  
  return x;
}

int getYFromIndex(int index) {
  return index / LED_WIDTH;
}
