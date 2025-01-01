#include <WiFiProvisioner.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Define a custom configuration
  WiFiProvisioner::Config
      customCfg("Custom Wi-Fi Provisioning",   // Access Point Name
                "Welcome to Custom Provision", // HTML Page Title
                "#15be79",                     // Theme Color
                R"rawliteral(
        <svg id="Icons" xmlns="http://www.w3.org/2000/svg" width="5rem" height="5rem" viewBox="0 0 512 512">
          <circle cx="256" cy="256" r="255.98" style="fill:#15be79"/>
          <path d="M508.81 296.61c-14.63 12.52-29.54 23.9-48.29 25.6-22 2-42.82-10.62-58.66-26s-28.54-33.88-45.22-48.36a129.2 129.2 0 0 0-181 11.46c-16.27 18.23-30.47 43-54.75 45.88-33 3.93-58.78-36.33-91.88-33.27-9.67.91-18.52 5.49-27.79 8.38C-12.24 139.7 90.78 14.78 231.4 1.2s265.81 89.48 279.4 230.2a256.53 256.53 0 0 1-1.99 65.21z" style="fill:#14a36d"/>
        </svg>
    )rawliteral",                              // SVG Logo
                "Custom Provisioner",          // Project Title
                "Custom Setup",                // Project Sub-title
                "Follow the steps to provision your device",    // Project
                                                                // Information
                "All rights reserved © Custom WiFiProvisioner", // Footer Text
                "The status LED will turn green, indicating a successful "
                "connection." // Connection Success Message
                "This process cannot be reversed.", // Reset Confirmation Text
                "API Key",                          // Input Field Text
                8,                                  // Input Field Length
                true,                               // Show Input Field
                true                                // Show Reset Field
      );

  // Create the WiFiProvisioner instance with the custom configuration
  WiFiProvisioner provisioner(customCfg);

  // Set up callbacks
  provisioner
      .onInputCheck([](const char *input) -> bool {
        Serial.printf("Checking if input code equals to 1234: %s\n", input);
        return strcmp(input, "1234") == 0; // Validate input
      })
      .onFactoryReset([]() {
        Serial.println("Factory reset triggered!");
        // Add any cleanup logic here if necessary
      })
      .onSuccess([](const char *ssid, const char *password, const char *input) {
        Serial.printf("Connected to SSID: %s\n", ssid);
        if (password) {
          Serial.printf("Password: %s\n", password);
        }
        if (input) {
          Serial.printf("Input: %s\n", input);
        }
        Serial.println("Provisioning completed successfully!");
      });

  // Start provisioning
  provisioner.startProvisioning();
}

void loop() {
  // Allow the provisioner to handle HTTP requests or other tasks
  delay(100);
}
