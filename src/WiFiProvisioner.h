#ifndef WIFIPROVISIONER_H
#define WIFIPROVISIONER_H

#include <IPAddress.h>
#include <functional>

class WebServer;
class DNSServer;

class WiFiProvisioner {
public:
  struct Config {
    const char *AP_NAME;                 // Access Point Name
    const char *HTML_TITLE;              // Page Title
    const char *THEME_COLOR;             // Theme color for the page
    const char *SVG_LOGO;                // SVG Logo for the web page
    const char *PROJECT_TITLE;           // Project Title
    const char *PROJECT_SUB_TITLE;       // Project sub-title
    const char *PROJECT_INFO;            // Information about the project
    const char *FOOTER_TEXT;             // Footer text
    const char *CONNECTION_SUCCESSFUL;   // Message for a successful connection
    const char *RESET_CONFIRMATION_TEXT; // Factory reset confirmation text
    const char *INPUT_TEXT;              // Text for additional input field
    int INPUT_LENGTH;                    // Length for additional input field
    bool SHOW_INPUT_FIELD;               // Whether to show an input field
    bool SHOW_RESET_FIELD;               // Whether to show a reset field

    Config()
        : AP_NAME("ESP32 Wi-Fi Provisioning"),
          HTML_TITLE("Welcome to Wi-Fi Provision"), THEME_COLOR("dodgerblue"),
          SVG_LOGO(R"rawliteral(
          <svg xmlns="http://www.w3.org/2000/svg" width="5rem" height="5rem" preserveAspectRatio="xMidYMid meet" viewBox="0 0 32 32">
            <path fill="var(--font-color)" d="M30 19h-4v-4h-2v9H8V8h9V6h-4V2h-2v4H8a2.002 2.002 0 0 0-2 2v3H2v2h4v6H2v2h4v3a2.002 2.002 0 0 0 2 2h3v4h2v-4h6v4h2v-4h3a2.003 2.003 0 0 0 2-2v-3h4Z"/>
            <path fill="var(--theme-color)" d="M21 21H11V11h10zm-8-2h6v-6h-6zm18-6h-2A10.012 10.012 0 0 0 19 3V1a12.013 12.013 0 0 1 12 12z"/>
            <path fill="var(--font-color)" d="M26 13h-2a5.006 5.006 0 0 0-5-5V6a7.008 7.008 0 0 1 7 7Z"/>
          </svg>
        )rawliteral"),
          PROJECT_TITLE("Wifi Provisioner"), PROJECT_SUB_TITLE("Device Setup"),
          PROJECT_INFO("Follow the steps to provision your device"),
          FOOTER_TEXT("All rights reserved © WiFiProvisioner"),
          CONNECTION_SUCCESSFUL(
              "Your device is now provisioned and ready to use."),
          RESET_CONFIRMATION_TEXT("This process cannot be undone."),
          INPUT_TEXT("Device Key"), INPUT_LENGTH(6), SHOW_INPUT_FIELD(true),
          SHOW_RESET_FIELD(true) {}
  };

  using InputCheckCallback = std::function<bool(const char *)>;
  using FactoryResetCallback = std::function<bool()>;
  using OnSuccessCallback =
      std::function<void(const char *, const char *, const char *)>;

  explicit WiFiProvisioner(const Config &customConfig = Config());
  ~WiFiProvisioner();

  inline Config &getConfig() { return _config; }

  bool startProvisioning();

  void setInputCheckCallback(InputCheckCallback callback);
  void setFactoryResetCallback(FactoryResetCallback callback);
  void setOnSuccessCallback(OnSuccessCallback callback);

private:
  void loop();
  bool connect(const char *ssid, const char *password);
  void releaseResources();
  void handleRootRequest();
  void handleResetRequest();
  void handleUpdateRequest();
  void handleConfigureRequest();
  void sendBadRequestResponse();
  void handleSuccesfulConnection(const char *ssid);
  void handleUnsuccessfulConnection(const char *ssid, const char *reason);

  InputCheckCallback inputCheckCallback;
  FactoryResetCallback factoryResetCallback;
  OnSuccessCallback onSuccessCallback;

  Config _config;
  WebServer *_server;
  DNSServer *_dnsServer;
  const IPAddress _apIP;
  const IPAddress netMsk;
  const uint16_t _dnsPort = 53;
  const unsigned int _serverPort = 80;
  const unsigned int _wifiDelay = 100;
  const unsigned int _wifiConnectionTimeout = 1000;
  bool _serverLoopFlag = false;
  // unsigned int _onSuccessDelay = 100;
};

#endif // WIFIPROVISIONER_H
