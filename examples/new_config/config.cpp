#include <WiFiProvisioner.h>

// Custom configuration
WiFiProvisioner::Config customConfig = {
    .AP_NAME = "MyDevice",
    .HTML_TITLE = "My Custom Provisioning",
    .THEME_COLOR = "coral",
    .PROJECT_TITLE = "Custom ESP32 Provisioning",
    .FOOTER_INFO = "My Custom Footer © 2024"};

WiFiProvisioner provisioner(customConfig);

void setup() {
  provisioner.enableSerialDebug(true);
  provisioner.setupAccessPointAndServer();
  auto config = provisioner.getConfig();
  config.AP_NAME = "UpdatedDevice";
  provisioner.setConfig(config);
}
