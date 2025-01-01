#include <WiFiProvisioner.h>

WiFiProvisioner provisioner;
const int buttonPin = 9; // GPIO pin number for the built-in BOOT button

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started...");

  // Show all fields
  provisioner.getConfig().SHOW_INPUT_FIELD = true;
  provisioner.getConfig().SHOW_RESET_FIELD = true;
  provisioner
      .onInputCheck([](const char *input) -> bool {
        Serial.printf("Checking if input code equals to 1234: %s\n", input);
        return strcmp(input, "1234") == 0; // Validate input
      })
      .onFactoryReset([]() { Serial.println("Factory reset triggered!"); })
      .onSuccess([](const char *ssid, const char *password, const char *input) {
        Serial.printf("Connected to SSID: %s\n", ssid);
        if (password)
          Serial.printf("Password: %s\n", password);
        if (input)
          Serial.printf("Input: %s\n", input);
      });

  provisioner.startProvisioning();
}

void loop() {
  // Read the state of the button
  int buttonState = digitalRead(buttonPin);

  // If the button is pressed (LOW because of the pull-up resistor), manually
  // start the provisioning by calling the setupAccessPointAndServer() function
  if (buttonState == LOW) {
    Serial.println("Button pressed. Starting provisioning...");
    provisioner.startProvisioning();
  }
}
