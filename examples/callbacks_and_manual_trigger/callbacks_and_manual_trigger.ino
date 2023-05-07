#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

const int buttonPin = 9; // GPIO pin number for the built-in BOOT button

bool inputValidationCallback(const String& input) {
  Serial.println("Checking input validity...");
  // Check if the input is valid, e.g., checking if it's "1234"
  return input == "1234";
}

void factoryReset() {
  // Perform your factory reset logic here
  Serial.println("Factory reset triggered.");

  // Show the input field after factory reset
  provisioner.setShowInputField(true);
}

void onProvision() {
  Serial.println("Provisioning started...");
  // Use a simple if statement to conditionally show the input field
  // For example, if a certain condition is met, show the input field
  bool example = false;
  if (example) {
    provisioner.setShowInputField(true);
  } else {
    provisioner.setShowInputField(false);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started...");

  // Only when trialing, trigger provision everytime
  provisioner.resetCredentials();

  //Enable Debug
  provisioner.enableSerialDebug(true);
  
  // Set the input validation callback
  provisioner.setInputCheckCallback(inputValidationCallback);

  // Set the factory reset callback
  provisioner.setFactoryResetCallback(factoryReset);

  // Set the onProvision callback
  provisioner.setOnProvisionCallback(onProvision);

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
    provisioner.setupAccessPointAndServer();
  }
}
