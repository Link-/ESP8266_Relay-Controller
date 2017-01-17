/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "....";
const char *password = "....";

/* Pins */
int LED_PIN = 14;
int RELAY_PIN = 4;
int LED_STATE = 0;    // 0 == LOW | 1 == HIGH
boolean DEBUG = false;

ESP8266WebServer server(80);

void setup() {
  delay(1000);
  if (DEBUG) Serial.begin(74880);

  /**
   * Setup GPIO operation
   */
  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  if (DEBUG) Serial.println();
  if (DEBUG) Serial.print("Connecting to ");
  if (DEBUG) Serial.println(ssid);

  // Connect
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Blink LED
    toggleWiFiStatusLED();
    if (DEBUG) Serial.print(".");
  }

  if (DEBUG) Serial.println("");
  if (DEBUG) Serial.println("WiFi connected");  
  if (DEBUG) Serial.println("IP address: ");
  if (DEBUG) Serial.println(WiFi.localIP());
  powerOnStatusLED();
  
  server.on("/", handleHome);
  server.on("/gpio", handleGPIO);
  server.begin();
  if (DEBUG) Serial.println("HTTP server started");
}

/**
 * When connection is lost -- Execute a reboot
 */
void wifiHeartBeat() {
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG) Serial.println("Connection lost. Rebooting in 5 seconds.");
    powerOffStatusLED();
    delay(5000);
    ESP.reset();
  }
}

/**
 * Request Handlers
 */
// Handler of: http://ip-address/
void handleHome() {
  String message = "<p>You are connected to <strong>Jarvis Node#1</strong></p>";
  message += "<p>Available Commands:</p>";
  message += "<p>--> /gpio?device=on</p>";
  message += "<p>--> /gpio?device=off</p>";
  server.send(200, "text/html", message);
}

// Handler of: http://ip-address/gpio
void handleGPIO() {
  String message = "<p>You are connected to <strong>Jarvis Node#1</strong></p> <p>Args received:</p><br /><p>";
  for (int i = 0; i < server.args(); i++) {
    // Output
    message += "Commands: " + (String)i + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "<br />";
  }

  // Handling special params
  if (server.arg("device") == "on") {
    digitalWrite(RELAY_PIN, HIGH);
    message += "-- Device (Pin#4) is now <strong>ON</strong>";
  } else if (server.arg("device") == "off") {
    digitalWrite(RELAY_PIN, LOW);
    message += "-- Device (Pin#4) is now <strong>OFF</strong>";
  }
  
  message += "</p>";
  server.send(200, "text/html", message);
}

/**
 * Toggle Status LED
 */
void toggleWiFiStatusLED() {
  if (LED_STATE == 0) {
    digitalWrite(LED_PIN, HIGH);
    LED_STATE = 1;
  } else if (LED_STATE == 1) {
    digitalWrite(LED_PIN, LOW);
    LED_STATE = 0;
  }
}

/**
 * Power On the Status LED
 */
void powerOnStatusLED() {
  digitalWrite(LED_PIN, HIGH);
  LED_STATE = 1;
}

/**
 * Power Off the Status LED
 */
void powerOffStatusLED() {
  digitalWrite(LED_PIN, LOW);
  LED_STATE = 0;
}

void loop() {
  server.handleClient();
  wifiHeartBeat();
}
