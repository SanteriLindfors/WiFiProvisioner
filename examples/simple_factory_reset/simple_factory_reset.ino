#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

bool checkInputCode(const String &input) {
  // Replace this with your custom code validation logic
  return input == "1234";
}

void factoryReset() {
  // Implement your custom factory reset logic here
  Serial.println("Resetting device to factory settings...");
  provisioner.setShowInputField(true);
  // Note that resetCredentials() is automatically called,
  // you dont need to call it
  provisioner.INPUT_TEXT = "New Input";
  provisioner.INPUT_PLACEHOLDER = "Changed Input after reset!";
}

void setup() {
  Serial.begin(115200);
  // Only when trialing, trigger provision everytime
  provisioner.resetCredentials();

  // Try to press "Factory reset" in provision HTML page and the input will show
  provisioner.enableSerialDebug(true);
  provisioner.setInputCheckCallback(checkInputCode);
  provisioner.setFactoryResetCallback(factoryReset);
  provisioner.connectToWiFi();
}

void loop() {
  // Your application logic here
}
