#ifndef WIFIPROVISIONER_H
#define WIFIPROVISIONER_H

#include <Arduino.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>

namespace WiFiProvisioner {

typedef bool (*InputCheckCallback)(const String &);
typedef void (*FactoryResetCallback)();
typedef void (*OnProvisionCallback)();

class WiFiProvisioner {
public:
  WiFiProvisioner();
  ~WiFiProvisioner();
  String AP_NAME = "ESP32 Wi-Fi Provisioning";
  String HTML_TITLE = "Welcome to Wifi Provision";
  String THEME_COLOR = "dodgerblue";
  String SVG_LOGO = R"rawliteral(
  <svg xmlns="http://www.w3.org/2000/svg" width="5rem" height="5rem"
                preserveAspectRatio="xMidYMid meet" viewBox="0 0 32 32"><path
                    fill="var(--font-color)" d="M30
                    19h-4v-4h-2v9H8V8h9V6h-4V2h-2v4H8a2.002 2.002 0 0 0-2
                    2v3H2v2h4v6H2v2h4v3a2.002 2.002 0 0 0 2
                    2h3v4h2v-4h6v4h2v-4h3a2.003 2.003 0 0 0 2-2v-3h4Z"/><path
                        fill="var(--theme-color)" d="M21
                        21H11V11h10zm-8-2h6v-6h-6zm18-6h-2A10.012 10.012 0 0 0
                        19 3V1a12.013 12.013 0 0 1 12 12z"/><path
                            fill="var(--font-color)" d="M26 13h-2a5.006 5.006 0
                            0 0-5-5V6a7.008 7.008 0 0 1 7 7Z"/></svg>)rawliteral";
  String PROJECT_TITLE = "ESP32 Wi-Fi Provisioning";
  String PROJECT_INFO = "Select a network to connect your device";
  String INPUT_TEXT = "Verification Code";
  String INPUT_PLACEHOLDER = "Enter verification code";
  String INPUT_LENGTH = "";
  String FOOTER_INFO = "Copyright © 2023";
  String INPUT_INVALID_LENGTH = "Invalid input length";
  String INPUT_NOT_VALID = "Invalid verification code";
  String CONNECTION_SUCCESSFUL = "The status LED will turn green, indicating "
                                 "a successful connection.";
  String RESET_CONFIRMATION_TEXT =
      "This will unlink the device from your account, and you will need to "
      "re-provision it as a new device.";

  void connectToWiFi();
  void setupAccessPointAndServer();
  void resetCredentials();
  void setInputCheckCallback(InputCheckCallback callback);
  void setFactoryResetCallback(FactoryResetCallback callback);
  void setOnProvisionCallback(OnProvisionCallback callback);
  void setConnectionTimeout(unsigned long timeout);
  void setShowInputField(bool value);
  void setRestartOnSuccess(bool value);
  void enableSerialDebug(bool enable);

private:
  void serverLoop();
  void releaseResources();
  void stopServerLoop(bool value);
  void serveRootPage();
  void handleRootRequest();
  void handleUpdateRequest();
  void handleConfigureRequest();
  void connectToWiFiNetwork(const String &ssid, const String &password);
  void handleSuccessfulConnection(bool codeok, const String &ssid,
                                  const String &password);
  void handleUnsuccessfulConnection(const String &ssid);
  void resetToFactorySettings();
  void sendBadRequestResponse();
  void saveNetworkConnectionDetails(const String &ssid, const String &password);
  bool connectToExistingWiFINetwork();
  void debugPrintln(const char *message);
  void debugPrintln(const String &message);
  int convertRRSItoLevel(int rssi);
  String getAvailableNetworks();
  InputCheckCallback inputCheckCallback;
  FactoryResetCallback factoryResetCallback;
  OnProvisionCallback onProvisionCallback;
  WebServer *m_server;
  DNSServer *m_dns_server;
  Preferences m_preferences;
  IPAddress apIP;
  IPAddress netMsk;
  bool showInputField = false;
  bool restartOnSuccess = false;
  bool stopLoopFlag = false;
  bool serialDebug = false;
  unsigned long connectionTimeout = 0;
  unsigned int wifiDelay = 100;
  unsigned int newWifiConnectionTimeout = 10000;
  const byte DNS_PORT = 53;
};

} // namespace WiFiProvisioner

#endif
