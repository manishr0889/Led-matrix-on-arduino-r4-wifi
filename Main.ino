#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// Your WiFi Network Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WiFiServer server(80);
ArduinoLEDMatrix matrix;

int currentDiceNumber = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize LED matrix
  matrix.begin();
  
  // Show a dash indicating it's booting/waiting
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.text("-", 4, 1);
  matrix.endDraw();

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start Server
  server.begin();
  
  // Seed random number generator using an open analog pin
  randomSeed(analogRead(0));
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected.");
    String currentLine = "";
    boolean rollDice = false;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (c == '\n') {
          // If the line is blank, it means the end of the client HTTP request
          if (currentLine.length() == 0) {
            
            // Send the HTTP response headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            // Generate Web Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #FF5733; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; border-radius: 10px;}");
            client.println(".dice-display { font-size: 50px; margin: 20px; font-weight: bold; color: #333; }</style></head>");
            
            client.println("<body><h1>Arduino R4 Digital Dice</h1>");
            
            // If button was clicked, handle logic
            if (rollDice) {
              currentDiceNumber = random(1, 7); // Generates 1 to 6
              displayNumberOnMatrix(currentDiceNumber);
            }

            if(currentDiceNumber > 0) {
              client.print("<div class=\"dice-display\">Rolled: ");
              client.print(currentDiceNumber);
              client.println("</div>");
            } else {
              client.println("<div class=\"dice-display\">Click below to roll!</div>");
            }
            
            client.println("<p><a href=\"/roll\"><button class=\"button\">START</button></a></p>");
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        // Check if user clicked the "START" button (which routes to /roll)
        if (currentLine.endsWith("GET /roll")) {
          rollDice = true;
        }
      }
    }
    
    // Give the web browser time to receive data, then close connection
    delay(10);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to handle printing the dynamic numbers onto the 12x8 LED grid
void displayNumberOnMatrix(int number) {
  String numStr = String(number);
  
  matrix.beginDraw();
  matrix.clear();
  matrix.textFont(Font_5x7); // Load standard built-in text font
  matrix.stroke(0xFFFFFFFF);
  
  // Center the single-digit text on the 12x8 matrix display
  matrix.text(numStr.c_str(), 4, 1); 
  matrix.endDraw();
}
