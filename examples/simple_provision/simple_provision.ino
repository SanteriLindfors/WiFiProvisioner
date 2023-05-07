#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

void setup() {
  Serial.begin(115200);
  // Only when trialing, trigger provision everytime
  provisioner.resetCredentials();
  
  provisioner.enableSerialDebug(true);
  provisioner.connectToWiFi();
}

void loop() {
  // Your application logic here
}
