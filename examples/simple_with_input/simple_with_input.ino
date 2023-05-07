#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

bool inputValidationCallback(const String &input) {
  // Print the entered code and some additional info text
  Serial.print("Entered code: ");
  Serial.println(input);
  Serial.println("Checking if the code is valid...");

  // Check if the input is valid, e.g., checking if it's the string "1234"
  return input == "1234";
}

void setup() {
  Serial.begin(115200);
  // Only when trialing, trigger provision everytime
  provisioner.resetCredentials();

  // Set the input validation callback
  provisioner.setInputCheckCallback(inputValidationCallback);

  // Enable the input field
  provisioner.setShowInputField(true);

  // Customize the error message for invalid input
  provisioner.INPUT_NOT_VALID = "Please enter the correct access code.";

  // Change the input text and placeholder text
  provisioner.INPUT_TEXT = "Enter access code:";
  provisioner.INPUT_PLACEHOLDER = "Access code";

  // Code needs to be 4 characters
  provisioner.INPUT_LENGTH = "4";

  provisioner.PROJECT_INFO = "Access code can be found on your profile page";

  // Connect to WiFi or start the provisioning process
  provisioner.connectToWiFi();
}

void loop() {
  // Your application logic here
}
