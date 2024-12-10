// WiFiProvisioner.cpp
#include "WiFiProvisioner.h"
// #include "internal/json.h"
#include "internal/provision_html.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#define WIFI_PROVISIONER_LOG_DEBUG 0
#define WIFI_PROVISIONER_LOG_INFO 1
#define WIFI_PROVISIONER_LOG_WARN 2
#define WIFI_PROVISIONER_LOG_ERROR 3

// Enable or disable WiFiProvisioner debug logs
#define WIFI_PROVISIONER_DEBUG

#ifdef WIFI_PROVISIONER_DEBUG
#define WIFI_PROVISIONER_DEBUG_LOG(level, format, ...)                         \
  do {                                                                         \
    if (level >= WIFI_PROVISIONER_LOG_INFO) {                                  \
      Serial.printf("[%s] " format "\n",                                       \
                    (level == WIFI_PROVISIONER_LOG_DEBUG)  ? "DEBUG"           \
                    : (level == WIFI_PROVISIONER_LOG_INFO) ? "INFO"            \
                    : (level == WIFI_PROVISIONER_LOG_WARN) ? "WARN"            \
                                                           : "ERROR",          \
                    ##__VA_ARGS__);                                            \
    }                                                                          \
  } while (0)
#else
#define DEBUG_LOG(level, format, ...)                                          \
  do {                                                                         \
  } while (0) // Empty macro
#endif

namespace {

int convertRRSItoLevel(int rssi) {
  //  Convert RSSI to 0 - 4 Step level
  int numlevels = 4;
  int MIN_RSSI = -100;
  int MAX_RSSI = -55;

  if (rssi < MIN_RSSI) {
    return 0;
  } else if (rssi >= MAX_RSSI) {
    return numlevels;
  } else {
    int inputRange = MAX_RSSI - MIN_RSSI;
    int res = std::ceil((rssi - MIN_RSSI) * numlevels / inputRange);
    if (res == 0) {
      return 1;
    } else {
      return res;
    }
  }
}

void networkScan(JsonDocument &doc) {

  JsonArray networks = doc.createNestedArray("network");

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Starting Network Scan...");
  int n = WiFi.scanNetworks(false, false);
  if (n) {
    for (int i = 0; i < n; ++i) {
      JsonObject network = networks.createNestedObject();
      network["rssi"] = convertRRSItoLevel(WiFi.RSSI(i));
      network["ssid"] = WiFi.SSID(i);
      network["authmode"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 0 : 1;
    }
  }
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Network scan complete");
}

void sendHeader(WiFiClient &client, int statusCode, const char *contentType,
                size_t contentLength) {
  client.print("HTTP/1.0 ");
  client.print(statusCode);
  client.println(" OK");

  client.print("Content-Type: ");
  client.println(contentType);

  client.print("Content-Length: ");
  client.println(contentLength);

  client.println("Connection: close");

  client.println();
}

} // namespace

WiFiProvisioner::WiFiProvisioner(const Config &config)
    : _config(config), _server(nullptr), _dnsServer(nullptr),
      _apIP(192, 168, 4, 1), netMsk(255, 255, 255, 0) {}

WiFiProvisioner::~WiFiProvisioner() { releaseResources(); }

void WiFiProvisioner::releaseResources() {
  // Webserver
  if (_server != nullptr) {
    if (_server->client() && _server->client().connected()) {
      WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                                 "Closing active client connection");
      _server->client().flush();
      _server->client().stop();
    }
    _server->stop();
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO, "Deleting server");
    delete _server;
    _server = nullptr;
  }

  // DNS
  if (_dnsServer != nullptr) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                               "Stopping DNS server");
    _dnsServer->stop();
    delete _dnsServer;
    _dnsServer = nullptr;
  }
}

void WiFiProvisioner::resetCredentials() {
  m_preferences.begin("network", false);
  m_preferences.clear();
  m_preferences.end();
}

void WiFiProvisioner::connectToWiFi() {
  // If connected, return immediately
  if (connectToExistingWiFINetwork()) {
    WIFI_PROVISIONER_DEBUG_LOG(
        WIFI_PROVISIONER_LOG_INFO,
        "Success Wifi connection with stored credentials, returning");
    return;
  }
  // Start AP
  startProvisioning();
}
bool WiFiProvisioner::connectToExistingWiFINetwork() {
  // Check if existing network configuration is found
  m_preferences.begin("network", true);
  String storedSSID = m_preferences.getString("ssid", "");
  String storedPassword = m_preferences.getString("password", "");
  m_preferences.end();

  if (storedSSID != "") {
    WiFi.mode(WIFI_STA); // Set Wi-Fi mode to STA
    delay(_wifiDelay);
    WIFI_PROVISIONER_DEBUG_LOG(
        WIFI_PROVISIONER_LOG_INFO,
        "Found existing wifi credientials, trying to connect with timeout %s",
        String(connectionTimeout));

    // Try to Connect to the WiFi with stored credentials
    if (storedPassword.isEmpty()) {
      WiFi.begin(storedSSID.c_str());
    } else {
      WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
    }
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(_wifiDelay);

      // Check if the connection timeout is reached
      if (connectionTimeout != 0 &&
          (millis() - startTime) >= connectionTimeout) {
        WiFi.disconnect();
        delay(_wifiDelay);
        WIFI_PROVISIONER_DEBUG_LOG(
            WIFI_PROVISIONER_LOG_INFO,
            "Connection timeout reached, continuing to start the provision");
        return false;
      }
    }
    return true;
  }
  return false;
}

bool WiFiProvisioner::startProvisioning() {
  // Invoke the provisioning callback if set
  if (onProvisionCallback) {
    onProvisionCallback();
  }

  // Reset loop flag and release existing resources
  _serverLoopFlag = false;
  releaseResources();

  // Disconnect
  WiFi.disconnect(false, true);
  delay(_wifiDelay);

  // Initialize the server object
  _server = new WebServer(_serverPort);
  _dnsServer = new DNSServer();

  // Configure Wi-Fi to AP+STA mode
  if (!WiFi.mode(WIFI_AP_STA)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to switch to AP+STA mode");
    return false;
  }

  delay(_wifiDelay);

  // Configure the access point
  if (!WiFi.softAPConfig(_apIP, _apIP, netMsk)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to configure AP IP settings");
    return false;
  }
  if (!WiFi.softAP(_config.AP_NAME)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to start Access Point");
    return false;
  }

  delay(_wifiDelay);

  // Start DNS server for captive portal
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  if (!_dnsServer->start(_dnsPort, "*", _apIP)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to start DNS server");
    return false;
  }

  // HTTP server routes
  _server->on("/", [this]() { this->handleRootRequest(); });
  _server->on("/configure", HTTP_POST,
              [this]() { this->handleConfigureRequest(); });
  _server->on("/update", [this]() { this->handleUpdateRequest(); });
  _server->on("/generate_204", [this]() { this->handleRootRequest(); });
  _server->on("/fwlink", [this]() { this->handleRootRequest(); });
  _server->onNotFound([this]() { this->handleRootRequest(); });
  _server->on("/factoryreset", HTTP_POST,
              [this]() { this->resetToFactorySettings(); });

  // Start the web server
  _server->begin();
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "HTTP server started at %s",
                             WiFi.softAPIP().toString());
  loop();
  return true;
}

void WiFiProvisioner::loop() {
  while (!_serverLoopFlag) {
    // DNS
    _dnsServer->processNextRequest();
    // HTTP
    _server->handleClient();
  }
  releaseResources();
  WiFi.mode(WIFI_STA); // Set Wi-Fi mode back to STA
  delay(_wifiDelay);
}

void WiFiProvisioner::setInputCheckCallback(InputCheckCallback callback) {
  inputCheckCallback = std::move(callback);
}

void WiFiProvisioner::setFactoryResetCallback(FactoryResetCallback callback) {
  factoryResetCallback = std::move(callback);
}

void WiFiProvisioner::setOnProvisionCallback(OnProvisionCallback callback) {
  onProvisionCallback = std::move(callback);
}

void WiFiProvisioner::setOnSuccessCallback(OnSuccessCallback callback) {
  onSuccessCallback = std::move(callback);
}

void WiFiProvisioner::handleRootRequest() { serveRootPage(); }

void WiFiProvisioner::serveRootPage() {

  // Calculate and send the content length
  size_t contentLength =
      strlen_P(index_html1) + strlen_P(index_html2) + strlen_P(index_html3) +
      strlen_P(index_html4) + strlen_P(index_html5) + strlen_P(index_html6) +
      strlen_P(index_html7) + strlen_P(index_html8) + strlen_P(index_html9) +
      strlen_P(index_html10) + strlen_P(index_html11) + strlen_P(index_html12) +
      strlen(_config.HTML_TITLE) + strlen(_config.THEME_COLOR) +
      strlen(_config.SVG_LOGO) + strlen(_config.PROJECT_TITLE) +
      strlen(_config.PROJECT_INFO) + strlen(_config.INPUT_TEXT) +
      strlen(_config.INPUT_PLACEHOLDER) +
      (strlen(_config.INPUT_LENGTH) ? strlen(_config.INPUT_LENGTH)
                                    : 4) + // Default to 4 ("1000")
      strlen(_config.FOOTER_INFO);

  WiFiClient client = _server->client();

  sendHeader(client, 200, "text/html", contentLength);

  // Send the body directly in chunks
  client.write_P(index_html1, strlen_P(index_html1));
  client.print(_config.HTML_TITLE);
  client.write_P(index_html2, strlen_P(index_html2));
  client.print(_config.THEME_COLOR);
  client.write_P(index_html3, strlen_P(index_html3));
  client.print(_config.SVG_LOGO);
  client.write_P(index_html4, strlen_P(index_html4));
  client.print(_config.PROJECT_TITLE);
  client.write_P(index_html5, strlen_P(index_html5));
  client.print(_config.PROJECT_INFO);
  client.write_P(index_html6, strlen_P(index_html6));
  client.print(_config.INPUT_TEXT);
  client.write_P(index_html7, strlen_P(index_html7));
  client.print(_config.INPUT_PLACEHOLDER);
  client.write_P(index_html8, strlen_P(index_html8));
  client.print(strlen(_config.INPUT_LENGTH) ? _config.INPUT_LENGTH : "1000");
  client.write_P(index_html9, strlen_P(index_html9));
  client.print(_config.FOOTER_INFO);
  client.write_P(index_html10, strlen_P(index_html10));
  client.write_P(index_html11, strlen_P(index_html11));
  client.write_P(index_html12, strlen_P(index_html12));

  // Close the client connection
  client.flush();
  client.stop();
}

void WiFiProvisioner::handleUpdateRequest() {
  JsonDocument doc;

  doc["show_code"] = _config.SHOW_INPUT_FIELD ? "true" : "false";

  networkScan(doc);

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN, "SENDING");

  WiFiClient client = _server->client();

  sendHeader(client, 200, "application/json", measureJson(doc));

  // Send the response body
  serializeJson(doc, client);

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN, "end");
  //   _server->send(200, "application/json", R"rawliteral({
  //   "show_code": true,
  //   "network": [
  //     { "ssid": "Network1", "rssi": -45, "authmode": 1 },
  //     { "ssid": "Network2", "rssi": -70, "authmode": 0 }
  //   ]
  // })rawliteral");

  client.flush();
  client.stop();
}

void WiFiProvisioner::handleConfigureRequest() {
  _server->client().setTimeout(30);

  if (!_server->hasArg("plain")) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "No 'plain' argument found in request");
    sendBadRequestResponse();
    return;
  }

  JsonDocument doc;
  auto error = deserializeJson(doc, _server->arg("plain"));
  if (error) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "JSON parsing failed: %s", error.c_str());
    sendBadRequestResponse();
    return;
  }

  const char *ssid_connect = doc["ssid"];
  const char *pass_connect = doc["password"];
  const char *input_connect = doc["code"];

  WIFI_PROVISIONER_DEBUG_LOG(
      WIFI_PROVISIONER_LOG_INFO, "SSID: %s, PASSWORD: %s, INPUT: %s",
      ssid_connect ? ssid_connect : "", pass_connect ? pass_connect : "",
      input_connect ? input_connect : "");

  if (!ssid_connect) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "SSID missing from request");
    sendBadRequestResponse();
    return;
  }

  if (!WiFi.disconnect(false, true)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "WiFi.disconnect() failed");
    sendBadRequestResponse();
    return;
  }

  delay(_wifiDelay);

  if (!connect(ssid_connect, pass_connect)) {

    // Keep server alive for while for user to see result.
    delay(_onSuccessDelay);

    // Signal to break from loop
    _serverLoopFlag = true;

    // WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
    //                            "Failed to connect to WiFi: %s with password
    //                            %s", ssid_connect, pass_connect ? pass_connect
    //                            : "");
    // handleUnsuccessfulConnection(ssid_connect, "ssid");
    return;
  }

  if (input_connect && inputCheckCallback &&
      !inputCheckCallback(input_connect)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                               "Input check callback failed.");
    handleUnsuccessfulConnection(ssid_connect, "code");
    return;
  }

  handleSuccesfulConnection(ssid_connect);

  if (onSuccessCallback) {
    onSuccessCallback(ssid_connect, pass_connect, input_connect);
  }

  // Keep server alive for while for user to see result.
  delay(_onSuccessDelay);

  // Signal to break from loop
  _serverLoopFlag = true;
}

bool WiFiProvisioner::connect(const char *ssid, const char *password) {
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Attempting to connect to SSID: %s", ssid);

  if (!ssid || strlen(ssid) == 0) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Invalid SSID provided");
    return false;
  }

  if (password && strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssid);
  }

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(_wifiDelay);

    if (millis() - startTime >= _wifiConnectionTimeout) {
      WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                                 "WiFi connection timeout reached for SSID: %s",
                                 ssid);
      return false;
    }
  }

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Successfully connected to SSID: %s", ssid);
  return true;
}

void WiFiProvisioner::handleSuccesfulConnection(const char *ssid) {
  JsonDocument doc;
  doc["success"] = true;
  doc["ssid"] = ssid;

  _server->setContentLength(measureJson(doc));
  _server->sendHeader("Content-Type", "application/json");
  _server->send(200);

  WiFiClient client = _server->client();
  serializeJson(doc, client);

  _server->client().stop();
}

void WiFiProvisioner::handleUnsuccessfulConnection(const char *ssid,
                                                   const char *reason) {
  JsonDocument doc;
  doc["success"] = false;
  doc["ssid"] = ssid;
  doc["reason"] = reason;

  _server->setContentLength(measureJson(doc));
  _server->sendHeader("Content-Type", "application/json");
  _server->send(200);

  WiFiClient client = _server->client();
  serializeJson(doc, client);

  _server->client().stop();

  WiFi.disconnect(false, true);
}

void WiFiProvisioner::saveNetworkConnectionDetails(const String &ssid,
                                                   const String &password) {
  m_preferences.begin("network", false);
  m_preferences.putString("ssid", ssid);
  m_preferences.putString("password", password);
  m_preferences.end();
}

void WiFiProvisioner::sendBadRequestResponse() {
  _server->send(400, "application/json", "{}");
  _server->client().stop();
}
void WiFiProvisioner::resetToFactorySettings() {
  // Clear the credientials and call factoryreset callback if needed and
  // continue to server root html page
  resetCredentials();
  if (factoryResetCallback) {
    factoryResetCallback();
  }
  serveRootPage();
}
