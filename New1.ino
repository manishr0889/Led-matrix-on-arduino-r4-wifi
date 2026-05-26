#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// Access Point Credentials (Your Arduino will create this network)
const char* ap_ssid = "Arduino_R4_Dice";
const char* ap_pass = "12345678"; // Must be at least 8 characters

WiFiServer server(80);
ArduinoLEDMatrix matrix;

int diceNumber = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Initialize the built-in 12x8 LED Matrix
  matrix.begin();
  
  // Display an initial placeholder on the matrix
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.text("-", 4, 1);
  matrix.endDraw();

  Serial.println("\nConfiguring Access Point...");

  // Start the Access Point on the Arduino R4 WiFi
  int status = WiFi.beginAP(ap_ssid, ap_pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating Access Point failed!");
    while (true);
  }

  // Print the details to Serial Monitor
  Serial.println("Access Point Started successfully!");
  Serial.print("Connect to Wi-Fi Network: ");
  Serial.println(ap_ssid);
  
  // FIXED: On the R4 WiFi, WiFi.localIP() returns the AP address when in AP Mode
  IPAddress myAddress = WiFi.localIP();
  Serial.print("Open this IP address in your web browser: ");
  Serial.println(myAddress);

  // Start the web server
  server.begin();
  
  // Seed random generator using an open analog pin
  randomSeed(analogRead(0));
}

void loop() {
  // Check for incoming clients (like your phone or laptop browser)
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected.");
    String currentLine = "";
    boolean rollDice = false;
    boolean scanWifi = false;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send standard HTTP response headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            // Build the HTML Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<style>html { font-family: Arial; text-align: center; background-color: #f4f4f9;}");
            client.println(".btn { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; font-size: 16px; margin: 10px; cursor: pointer; border-radius: 5px; text-decoration: none; display: inline-block;}");
            client.println(".btn-scan { background-color: #008CBA; }");
            client.println(".box { border: 1px solid #ccc; padding: 20px; display: inline-block; background: white; border-radius: 10px; box-shadow: 2px 2px 12px #aaa; margin-top: 20px; min-width: 280px;}");
            client.println("ul { list-style-type: none; padding: 0; text-align: left; display: inline-block;}");
            client.println("li { background: #eee; margin: 5px 0; padding: 10px; border-radius: 4px; }</style></head>");
            
            client.println("<body><h1>Arduino R4 Control Panel</h1>");
            
            // Handle Dice Rolling Request
            if (rollDice) {
              diceNumber = random(1, 7);
              displayNumberOnMatrix(diceNumber);
            }

            // --- PAGE COMPONENT: DICE GAME ---
            client.println("<div class='box'><h2>🎲 Dice Game</h2>");
            if (diceNumber > 0) {
              client.print("<h3>Rolled Number: <span style='color:#4CAF50; font-size: 28px;'>");
              client.print(diceNumber);
              client.println("</span></h3>");
            } else {
              client.println("<h3>Click Start to Roll</h3>");
            }
            client.println("<a href='/roll' class='btn'>START ROLL</a>");
            client.println("</div><br>");

            // --- PAGE COMPONENT: WI-FI SCANNER ---
            client.println("<div class='box'><h2>📡 Network Scanner Tool</h2>");
            client.println("<a href='/scan' class='btn btn-scan'>SCAN NETWORKS</a>");
            
            if (scanWifi) {
              client.println("<br><h3>Nearby Networks:</h3><ul>");
              int numNetworks = WiFi.scanNetworks();
              if (numNetworks == -1) {
                client.println("<li>No networks found or error scanning.</li>");
              } else {
                for (int i = 0; i < numNetworks; i++) {
                  client.print("<li><strong>");
                  client.print(WiFi.SSID(i));
                  client.print("</strong> (Signal: ");
                  client.print(WiFi.RSSI(i));
                  client.print(" dBm)</li>");
                }
              }
              client.println("</ul>");
            }
            client.println("</div>");
            
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        // Check user actions
        if (currentLine.endsWith("GET /roll")) {
          rollDice = true;
        }
        if (currentLine.endsWith("GET /scan")) {
          scanWifi = true;
        }
      }
    }
    delay(10);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to handle printing the dynamic numbers onto the 12x8 LED matrix
void displayNumberOnMatrix(int number) {
  String numStr = String(number);
  matrix.beginDraw();
  matrix.clear();
  matrix.textFont(Font_5x7);
  matrix.stroke(0xFFFFFFFF);
  matrix.text(numStr.c_str(), 4, 1); // Center the text perfectly
  matrix.endDraw();
}
