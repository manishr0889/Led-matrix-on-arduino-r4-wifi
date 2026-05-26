// NOTE: This code requires an ESP8266 board. It will not compile on Arduino R4.
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
int diceNumber = 0;

void handleRoot() {
  // --- PAGE 1: THE DICE GAME ---
  String html = "<html><head><title>Smart Board</title></head><body style='text-align:center; font-family:Arial;'>";
  html += "<h1>🎲 Arduino Digital Dice 🎲</h1>";
  html += "<p><a href='/roll'><button style='padding:15px; font-size:20px; background:#4CAF50; color:white;'>START ROLL</button></a></p>";
  
  if(diceNumber > 0) {
    html += "<h2>You Rolled: " + String(diceNumber) + "</h2>";
  }
  
  html += "<br><hr style='width:50%'><br>";
  html += "<h3>Network Tools</h3>";
  html += "<p><a href='/scan'><button style='padding:10px; background:#008CBA; color:white;'>Go to Wi-Fi Deauther Page</button></a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleRoll() {
  diceNumber = random(1, 7); // Pick number 1 to 6
  handleRoot();              // Refresh page to show the number
}

void handleScan() {
  // --- PAGE 2: EXTRA WEB PAGE (THE DEAUTHER) ---
  int n = WiFi.scanNetworks(); // Scan for nearby networks
  
  String html = "<html><body style='font-family:Arial; padding:20px;'>";
  html += "<h1>📡 Wi-Fi Deauther Panel</h1>";
  html += "<p><a href='/'><button><- Back to Dice Game</button></a></p>";
  html += "<h3>Select a network to test deauthentication:</h3><ul>";
  
  for (int i = 0; i < n; ++i) {
    html += "<li><strong>" + WiFi.SSID(i) + "</strong> (" + String(WiFi.RSSI(i)) + " dBm) ";
    // Button to trigger the deauth attack function on this specific network index
    html += "<a href='/attack?target=" + String(i) + "'><button style='color:red;'>Deauth</button></a></li><br>";
  }
  
  html += "</ul></body></html>";
  server.send(200, "text/html", html);
}

void handleAttack() {
  String targetID = server.arg("target");
  
  // REAL DEAUTH LOGIC GOES HERE
  // (Using low-level ESP8266 functions like wifi_send_pkt_freedom to broadcast deauth frames)
  
  String html = "<html><body style='text-align:center; font-family:Arial; color:red;'>";
  html += "<h1>⚠️ Deauther Active ⚠️</h1>";
  html += "<p>Sending deauthentication frames to target network index: " + targetID + "</p>";
  html += "<br><a href='/scan'><button style='padding:10px;'>Stop & Go Back</button></a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  // Setup the board as a Hotspot (Access Point) so you can connect your phone directly to it
  WiFi.softAP("Dice_Deauther_Net", "password123");
  Serial.println("Hotspot Started! Connect to 'Dice_Deauther_Net'");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define Web Routes
  server.on("/", handleRoot);
  server.on("/roll", handleRoll);
  server.on("/scan", handleScan);
  server.on("/attack", handleAttack);
  
  server.begin();
}

void loop() {
  server.handleClient();
}

