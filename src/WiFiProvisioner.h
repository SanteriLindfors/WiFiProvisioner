#ifndef WIFIPROVISIONER_H
#define WIFIPROVISIONER_H

// #include <Arduino.h>
#include <Preferences.h>
#include <functional>

class WebServer;
class DNSServer;

namespace WiFiProvisioner {

using InputCheckCallback = std::function<bool(const char *)>;
using FactoryResetCallback = std::function<void()>;
using OnProvisionCallback = std::function<void()>;
using OnSuccessCallback =
    std::function<void(const char *, const char *, const char *)>;

struct Config {
  const char *AP_NAME = "ESP32 Wi-Fi Provisioning";      // Access Point Name
  const char *HTML_TITLE = "Welcome to Wi-Fi Provision"; // Page Title
  const char *THEME_COLOR = "dodgerblue"; // Theme color for the page
  const char *SVG_LOGO = R"rawliteral(
        <svg xmlns="http://www.w3.org/2000/svg" width="5rem" height="5rem"
        preserveAspectRatio="xMidYMid meet" viewBox="0 0 32 32">
        <path fill="var(--font-color)" d="M30
        19h-4v-4h-2v9H8V8h9V6h-4V2h-2v4H8a2.002 2.002 0 0 0-2
        2v3H2v2h4v6H2v2h4v3a2.002 2.002 0 0 0 2
        2h3v4h2v-4h6v4h2v-4h3a2.003 2.003 0 0 0 2-2v-3h4Z"/> <path
        fill="var(--theme-color)" d="M21
        21H11V11h10zm-8-2h6v-6h-6zm18-6h-2A10.012 10.012 0 0 0 19
        3V1a12.013 12.013 0 0 1 12 12z"/> <path fill="var(--font-color)"
        d="M26 13h-2a5.006 5.006 0 0 0-5-5V6a7.008 7.008 0 0 1 7
        7Z"/></svg>)rawliteral";          // SVG Logo
  const char *PROJECT_TITLE = "ESP32 Wi-Fi Provisioning"; // Project Title
  const char *PROJECT_INFO =
      "Select a network to connect your device"; // Project Information
  const char *INPUT_TEXT = "Verification Code";  // Text for input fields
  const char *INPUT_PLACEHOLDER =
      "Enter verification code"; // Placeholder text for inputs
  const char *INPUT_LENGTH = ""; // Length validation (default empty)
  const char *FOOTER_INFO = "Copyright © 2023"; // Footer Information
  const char *INPUT_INVALID_LENGTH =
      "Invalid input length"; // Invalid input length error
  const char *INPUT_NOT_VALID =
      "Invalid verification code"; // Invalid input error
  const char *CONNECTION_SUCCESSFUL =
      "The status LED will turn green, indicating a successful "
      "connection."; // Connection success message
  const char *RESET_CONFIRMATION_TEXT =
      "This will unlink the device from your account, and you will need to "
      "re-provision it as a new device."; // Factory reset confirmation
  bool SHOW_INPUT_FIELD =
      false; // Show additional input field with wifi credientials
};

class WiFiProvisioner {
public:
  explicit WiFiProvisioner(const Config &config = Config());
  ~WiFiProvisioner();

  inline Config &getConfig() { return _config; }

  bool startProvisioning();

  void connectToWiFi();
  void resetCredentials();
  void setInputCheckCallback(InputCheckCallback callback);
  void setFactoryResetCallback(FactoryResetCallback callback);
  void setOnProvisionCallback(OnProvisionCallback callback);
  void setOnSuccessCallback(OnSuccessCallback callback);
  void setConnectionTimeout(unsigned long timeout);

private:
  void loop();
  bool connect(const char *ssid, const char *password);
  void releaseResources();
  void serveRootPage();
  void handleRootRequest();
  void handleUpdateRequest();
  void handleConfigureRequest();
  void sendBadRequestResponse();
  void handleSuccesfulConnection(const char *ssid);
  void handleUnsuccessfulConnection(const char *ssid, const char *reason);

  void resetToFactorySettings();
  void saveNetworkConnectionDetails(const String &ssid, const String &password);
  bool connectToExistingWiFINetwork();

  InputCheckCallback inputCheckCallback;
  FactoryResetCallback factoryResetCallback;
  OnProvisionCallback onProvisionCallback;
  OnSuccessCallback onSuccessCallback;

  Config _config;
  WebServer *_server;
  DNSServer *_dnsServer;
  Preferences m_preferences;
  IPAddress _apIP;
  IPAddress netMsk;
  const byte _dnsPort = 53;
  const unsigned int _serverPort = 80;
  bool _serverLoopFlag = false;

  unsigned int _wifiDelay = 100;
  unsigned int _onSuccessDelay = 100;
  unsigned int _wifiConnectionTimeout = 1000;

  unsigned long connectionTimeout = 0;
};

} // namespace WiFiProvisioner

#endif // WIFIPROVISIONER_H
