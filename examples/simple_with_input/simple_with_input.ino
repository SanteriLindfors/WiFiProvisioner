#include <WiFiProvisioner.h>

void setup() {
  Serial.begin(115200);

  WiFiProvisioner provisioner;
  provisioner.getConfig().SHOW_INPUT_FIELD = true;
  provisioner.getConfig().SHOW_RESET_FIELD = false;

  provisioner.
      .onInputCheck([](const char *input) -> bool {
        Serial.printf("Checking if input code equals to 1234: %s\n", input);
        return strcmp(input, "1234") == 0; // Validate input
      })
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
  // Your application logic here
}
