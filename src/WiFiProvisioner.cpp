// WiFiProvisioner.cpp
#include "WiFiProvisioner.h"
#include "html/provision_html.h"
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

namespace WiFiProvisioner {

// Constructor: Initialize with provided config or use defaults
WiFiProvisioner::WiFiProvisioner(const Config &config)
    : _config(config), _server(nullptr), _dnsServer(nullptr),
      _apIP(192, 168, 4, 1), netMsk(255, 255, 255, 0) {}

WiFiProvisioner::~WiFiProvisioner() { releaseResources(); }

void WiFiProvisioner::setConnectionTimeout(unsigned long timeout) {
  connectionTimeout = timeout;
}
void WiFiProvisioner::releaseResources() {
  if (_server != nullptr) {
    delete _server;
    _server = nullptr;
  }
  if (_dnsServer != nullptr) {
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

  // Initialize the server object
  _server = new WebServer(_serverPort);
  _dnsServer = new DNSServer();

  // Configure Wi-Fi to AP+STA mode
  WiFi.mode(WIFI_AP_STA);
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
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO, "AP IP address: %s",
                             WiFi.softAPIP().toString());

  // Start DNS server for captive portal
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  _dnsServer->start(_dnsPort, "*", _apIP);

  // Define HTTP server routes
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
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO, "HTTP server started");
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

int WiFiProvisioner::convertRRSItoLevel(int rssi) {
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

String WiFiProvisioner::getAvailableNetworks() {
  //  Get Availible networks and return as json
  StaticJsonDocument<4096> jsonDoc;
  JsonArray networks = jsonDoc.to<JsonArray>();

  JsonObject inputObj = networks.createNestedObject();
  inputObj["show_code"] = (_config.SHOW_INPUT_FIELD) ? "true" : "false";
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Starting Network Scan...");
  int n = WiFi.scanNetworks(false, false);
  if (n) {
    for (int i = 0; i < n; ++i) {
      JsonObject networkObj = networks.createNestedObject();
      networkObj["rssi"] = convertRRSItoLevel(WiFi.RSSI(i));
      networkObj["ssid"] = WiFi.SSID(i);
      networkObj["authmode"] =
          (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 0 : 1;
    }
  }

  String jsonString;
  serializeJson(jsonDoc, jsonString);
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO, "Found Networks : %s",
                             jsonString);
  return jsonString;
}

void WiFiProvisioner::handleRootRequest() { serveRootPage(); }

void WiFiProvisioner::serveRootPage() {
  //  Build the Root HTML Page
  _server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  _server->send(200, "text/html", "");

  _server->sendContent_P(index_html1, strlen_P(index_html1));
  if (_config.HTML_TITLE != "") {
    _server->sendContent(_config.HTML_TITLE);
  }
  _server->sendContent_P(index_html2, strlen_P(index_html2));
  _server->sendContent(_config.THEME_COLOR);
  _server->sendContent_P(index_html3, strlen_P(index_html3));
  if (_config.SVG_LOGO != "") {
    _server->sendContent(_config.SVG_LOGO);
  }
  _server->sendContent_P(index_html4, strlen_P(index_html4));
  if (_config.PROJECT_TITLE != "") {
    _server->sendContent(_config.PROJECT_TITLE);
  }
  _server->sendContent_P(index_html5, strlen_P(index_html5));
  if (_config.PROJECT_INFO != "") {
    _server->sendContent(_config.PROJECT_INFO);
  }
  _server->sendContent_P(index_html6, strlen_P(index_html6));
  if (_config.INPUT_TEXT != "") {
    _server->sendContent(_config.INPUT_TEXT);
  }
  _server->sendContent_P(index_html7, strlen_P(index_html7));
  if (_config.INPUT_PLACEHOLDER != "") {
    _server->sendContent(_config.INPUT_PLACEHOLDER);
  }
  _server->sendContent_P(index_html8, strlen_P(index_html8));
  if (_config.INPUT_LENGTH != "") {
    _server->sendContent(_config.INPUT_LENGTH);
  } else {
    _server->sendContent("1000");
  }
  _server->sendContent_P(index_html9, strlen_P(index_html9));
  if (_config.FOOTER_INFO != "") {
    _server->sendContent(_config.FOOTER_INFO);
  }
  _server->sendContent_P(index_html10, strlen_P(index_html10));

  String javascriptVariables;

  javascriptVariables += "var invalid_code_lenght = \"";
  javascriptVariables += _config.INPUT_INVALID_LENGTH;
  javascriptVariables += "\";\n";

  javascriptVariables += "var invalid_code = \"";
  javascriptVariables += _config.INPUT_NOT_VALID;
  javascriptVariables += "\";\n";

  javascriptVariables += "var connection_successful_text = \"";
  javascriptVariables += _config.CONNECTION_SUCCESSFUL;
  javascriptVariables += "\";\n";

  javascriptVariables += "var reset_confirmation_text = \"";
  javascriptVariables += _config.RESET_CONFIRMATION_TEXT;
  javascriptVariables += "\";\n";
  _server->sendContent(javascriptVariables);
  _server->sendContent_P(index_html11, strlen_P(index_html11));
  if (_config.INPUT_LENGTH != "") {
    _server->sendContent("&& code_listener.value.length !=");
    _server->sendContent(_config.INPUT_LENGTH);
  } else {
    _server->sendContent("&& code_listener.value.length ==");
    _server->sendContent("-1");
  }
  _server->sendContent_P(index_html12, strlen_P(index_html12));
  _server->sendContent(""); // Mark the end of the response
  // _server->send(200, "text/html", index_html);
  _server->client().stop();
}

void WiFiProvisioner::handleUpdateRequest() {
  _server->send(200, "application/json", getAvailableNetworks());
  _server->client().stop();
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
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "Failed to connect to WiFi: %s with password %s",
                               ssid_connect, pass_connect ? pass_connect : "");
    handleUnsuccessfulConnection(ssid_connect);
    return;
  }

  bool status = true;
  if (input_connect && inputCheckCallback) {
    bool status = inputCheckCallback(input_connect);
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                               "Input check callback result: %s",
                               status ? "true" : "false");
  }

  handleConnectionResult(status, ssid_connect);

  // Make sure that response is send & displayed to client properly
  if (status) {
    if (onSuccessCallback) {
      onSuccessCallback(ssid_connect, pass_connect, input_connect);
    }
    // Keep server alive for while for user to see result.
    delay(5000);
    // Signal to break from loop
    _serverLoopFlag = true;
  } else {
    WiFi.disconnect(false, true);
  }
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

    if (millis() - startTime >= newWifiConnectionTimeout) {
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

void WiFiProvisioner::handleConnectionResult(bool success, const char *ssid) {
  JsonDocument doc;
  doc["success"] = success;
  doc["ssid"] = ssid;
  if (!success) {
    doc["reason"] = "code";
  }
  // String finalout;
  // serializeJson(doc, finalout);
  // _server->send(200, "application/json", finalout);
  // _server->client().stop();

  _server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  _server->send(200, "application/json");
  WiFiClient client = _server->client();
  serializeJson(doc, client);
  _server->sendContent(""); // Mark the end of the response
  _server->client().stop();
}

void WiFiProvisioner::handleUnsuccessfulConnection(const String &ssid) {
  JsonDocument doc;
  doc["success"] = false;
  doc["ssid"] = ssid;
  doc["reason"] = "ssid";

  // String output;
  // serializeJson(res_false, output);
  // _server->send(200, "application/json", output);
  // _server->client().stop();

  // Write response headers
  char contentLengthHeader[50];
  snprintf(contentLengthHeader, sizeof(contentLengthHeader),
           "Content-Length: %zu", measureJsonPretty(doc));

  WiFiClient client = _server->client();
  _server->sendContent("HTTP/1.0 200 OK");
  _server->sendContent("Content-Type: application/json");
  _server->sendContent("Connection: close");
  _server->sendContent(contentLengthHeader);
  _server->sendContent("");

  // Write JSON document
  serializeJsonPretty(doc, client);
  _server->client().stop();

  // _server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  // _server->send(200, "application/json");
  // WiFiClient client = _server->client();
  // serializeJson(doc, client);
  // _server->sendContent(""); // Mark the end of the response
  // _server->client().stop();

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

} // namespace WiFiProvisioner
