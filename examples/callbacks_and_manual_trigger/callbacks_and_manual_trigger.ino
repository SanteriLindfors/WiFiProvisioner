#include <WiFiProvisioner.h>

 WiFiProvisioner provisioner;
const int buttonPin = 9; // GPIO pin number for the built-in BOOT button



void setup() {
  Serial.begin(115200);
  Serial.println("Setup started...");

  provisioner
      .onInputCheck([](const char *input) -> bool {
          return strcmp(input, "1234") == 0; // Validate input
      })
      .onFactoryReset([]() {
          Serial.println("Factory reset triggered!");
      })
      .onSuccess([](const char *ssid, const char *password, const char *input) {
          Serial.printf("Connected to SSID: %s\n", ssid);
          if (password) Serial.printf("Password: %s\n", password);
          if (input) Serial.printf("Input: %s\n", input);
      });

  provisioner.startProvisioning();

  }

void loop() {
    // Handle Wi-Fi provisioning loop if necessary
}


  // Set the initial input text and placeholder text
  provisioner.INPUT_TEXT = "Enter custom value:";
  provisioner.INPUT_PLACEHOLDER = "Custom value";

  // Set the input length to 4
  provisioner.INPUT_LENGTH = "4"; // Limit the input length to 4 characters

  // Set up the button pin as input with pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Read the state of the button
  int buttonState = digitalRead(buttonPin);

  // If the button is pressed (LOW because of the pull-up resistor), manually start the provisioning by calling the setupAccessPointAndServer() function
  if (buttonState == LOW) {
    Serial.println("Button pressed. Starting provisioning...");
    provisioner.startProvisioning();
  }
}
