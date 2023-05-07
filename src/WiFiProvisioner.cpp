// WiFiProvisioner.cpp
#include "WiFiProvisioner.h"
#include "internal/provision_html.h"
#include <ArduinoJson.h>
#include <WiFi.h>

namespace WiFiProvisioner {

WiFiProvisioner::WiFiProvisioner()
    : m_server(nullptr), m_dns_server(nullptr), apIP(192, 168, 4, 1),
      netMsk(255, 255, 255, 0) {}
WiFiProvisioner::~WiFiProvisioner() { releaseResources(); }
void WiFiProvisioner::setShowInputField(bool value) { showInputField = value; }
void WiFiProvisioner::setRestartOnSuccess(bool value) {
  restartOnSuccess = value;
}
void WiFiProvisioner::stopServerLoop(bool value) { stopLoopFlag = value; }
void WiFiProvisioner::setConnectionTimeout(unsigned long timeout) {
  connectionTimeout = timeout;
}
void WiFiProvisioner::releaseResources() {
  if (m_server != nullptr) {
    delete m_server;
    m_server = nullptr;
  }
  if (m_dns_server != nullptr) {
    delete m_dns_server;
    m_dns_server = nullptr;
  }
}
void WiFiProvisioner::enableSerialDebug(bool enable) { serialDebug = enable; }
void WiFiProvisioner::debugPrintln(const char *message) {
  if (serialDebug) {
    Serial.println(message);
  }
}

void WiFiProvisioner::debugPrintln(const String &message) {
  if (serialDebug) {
    Serial.println(message);
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
    debugPrintln("Success Wifi connection with stored credentials, returning");
    return;
  }
  // Start AP
  setupAccessPointAndServer();
}
bool WiFiProvisioner::connectToExistingWiFINetwork() {
  // Check if existing network configuration is found
  m_preferences.begin("network", true);
  String storedSSID = m_preferences.getString("ssid", "");
  String storedPassword = m_preferences.getString("password", "");
  m_preferences.end();

  if (storedSSID != "") {
    WiFi.mode(WIFI_STA); // Set Wi-Fi mode to STA
    delay(wifiDelay);
    debugPrintln(
        "Found existing wifi credientials, trying to connect with timeout" +
        String(connectionTimeout));

    // Try to Connect to the WiFi with stored credentials
    if (storedPassword.isEmpty()) {
      WiFi.begin(storedSSID.c_str());
    } else {
      WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
    }
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(wifiDelay);

      // Check if the connection timeout is reached
      if (connectionTimeout != 0 &&
          (millis() - startTime) >= connectionTimeout) {
        WiFi.disconnect();
        delay(wifiDelay);
        debugPrintln(
            "Connection timeout reached, continuing to start the provision");
        return false;
      }
    }
    return true;
  }
  return false;
}
void WiFiProvisioner::setupAccessPointAndServer() {
  // Call onProvision Callback if set..
  if (onProvisionCallback) {
    onProvisionCallback();
  }

  //  Reset the loop flag
  stopServerLoop(false);

  // Initialize the server object
  m_server = new WebServer(80);
  m_dns_server = new DNSServer();

  // Set up the WiFi mode
  WiFi.mode(WIFI_AP_STA);
  delay(wifiDelay);

  // Configure the access point
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(AP_NAME.c_str());
  delay(wifiDelay);
  debugPrintln("AP IP address: " + String(WiFi.softAPIP()));

  // Set up the DNS server
  m_dns_server->setErrorReplyCode(DNSReplyCode::NoError);
  m_dns_server->start(DNS_PORT, "*", apIP);

  // Set up the web server routes
  m_server->on("/", [this]() { this->handleRootRequest(); });
  m_server->on("/configure", HTTP_POST,
               [this]() { this->handleConfigureRequest(); });
  m_server->on("/update", [this]() { this->handleUpdateRequest(); });
  m_server->on("/generate_204", [this]() { this->handleRootRequest(); });
  m_server->on("/fwlink", [this]() { this->handleRootRequest(); });
  m_server->onNotFound([this]() { this->handleRootRequest(); });
  m_server->on("/factoryreset", HTTP_POST,
               [this]() { this->resetToFactorySettings(); });

  // Start the web server
  m_server->begin();
  debugPrintln("HTTP server started");
  serverLoop();
}

void WiFiProvisioner::serverLoop() {
  while (!stopLoopFlag) {
    // DNS
    m_dns_server->processNextRequest();
    // HTTP
    m_server->handleClient();
  }
  releaseResources();
  WiFi.mode(WIFI_STA); // Set Wi-Fi mode back to STA
  delay(wifiDelay);
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
  inputCheckCallback = callback;
}

void WiFiProvisioner::setFactoryResetCallback(FactoryResetCallback callback) {
  factoryResetCallback = callback;
}
void WiFiProvisioner::setOnProvisionCallback(OnProvisionCallback callback) {
  onProvisionCallback = callback;
}

String WiFiProvisioner::getAvailableNetworks() {
  //  Get Availible networks and return as json
  StaticJsonDocument<4096> jsonDoc;
  JsonArray networks = jsonDoc.to<JsonArray>();

  JsonObject inputObj = networks.createNestedObject();
  inputObj["show_code"] = (showInputField) ? "true" : "false";
  debugPrintln("Starting Network Scan...");
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
  debugPrintln("Found Networks : " + jsonString);
  return jsonString;
}

void WiFiProvisioner::handleRootRequest() { serveRootPage(); }

void WiFiProvisioner::serveRootPage() {
  //  Build the Root HTML Page
  m_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  m_server->send(200, "text/html", "");

  m_server->sendContent_P(index_html1, strlen_P(index_html1));
  if (HTML_TITLE != "") {
    m_server->sendContent(HTML_TITLE);
  }
  m_server->sendContent_P(index_html2, strlen_P(index_html2));
  m_server->sendContent(THEME_COLOR);
  m_server->sendContent_P(index_html3, strlen_P(index_html3));
  if (SVG_LOGO != "") {
    m_server->sendContent(SVG_LOGO);
  }
  m_server->sendContent_P(index_html4, strlen_P(index_html4));
  if (PROJECT_TITLE != "") {
    m_server->sendContent(PROJECT_TITLE);
  }
  m_server->sendContent_P(index_html5, strlen_P(index_html5));
  if (PROJECT_INFO != "") {
    m_server->sendContent(PROJECT_INFO);
  }
  m_server->sendContent_P(index_html6, strlen_P(index_html6));
  if (INPUT_TEXT != "") {
    m_server->sendContent(INPUT_TEXT);
  }
  m_server->sendContent_P(index_html7, strlen_P(index_html7));
  if (INPUT_PLACEHOLDER != "") {
    m_server->sendContent(INPUT_PLACEHOLDER);
  }
  m_server->sendContent_P(index_html8, strlen_P(index_html8));
  if (INPUT_LENGTH != "") {
    m_server->sendContent(INPUT_LENGTH);
  } else {
    m_server->sendContent("1000");
  }
  m_server->sendContent_P(index_html9, strlen_P(index_html9));
  if (FOOTER_INFO != "") {
    m_server->sendContent(FOOTER_INFO);
  }
  m_server->sendContent_P(index_html10, strlen_P(index_html10));

  String javascriptVariables;

  javascriptVariables += "var invalid_code_lenght = \"";
  javascriptVariables += INPUT_INVALID_LENGTH;
  javascriptVariables += "\";\n";

  javascriptVariables += "var invalid_code = \"";
  javascriptVariables += INPUT_NOT_VALID;
  javascriptVariables += "\";\n";

  javascriptVariables += "var connection_successful_text = \"";
  javascriptVariables += CONNECTION_SUCCESSFUL;
  javascriptVariables += "\";\n";

  javascriptVariables += "var reset_confirmation_text = \"";
  javascriptVariables += RESET_CONFIRMATION_TEXT;
  javascriptVariables += "\";\n";
  m_server->sendContent(javascriptVariables);
  m_server->sendContent_P(index_html11, strlen_P(index_html11));
  if (INPUT_LENGTH != "") {
    m_server->sendContent("&& code_listener.value.length !=");
    m_server->sendContent(INPUT_LENGTH);
  } else {
    m_server->sendContent("&& code_listener.value.length ==");
    m_server->sendContent("-1");
  }
  m_server->sendContent_P(index_html12, strlen_P(index_html12));
  m_server->sendContent(""); // Mark the end of the response
  // m_server->send(200, "text/html", index_html);
  m_server->client().stop();
}

void WiFiProvisioner::handleUpdateRequest() {
  m_server->send(200, "application/json", getAvailableNetworks());
  m_server->client().stop();
}
void WiFiProvisioner::handleConfigureRequest() {
  m_server->client().setTimeout(30);

  if (!m_server->hasArg("plain")) {
    sendBadRequestResponse();
    return;
  }

  StaticJsonDocument<256> configrequest;
  deserializeJson(configrequest, m_server->arg("plain"));
  JsonObject conf = configrequest.as<JsonObject>();

  bool hasSSID = conf.containsKey("ssid");
  bool hasPASS = conf.containsKey("password");
  bool hasINPUT = conf.containsKey("code");

  String ssid_connect = hasSSID ? conf["ssid"].as<String>() : "";
  String pass_connect = hasPASS ? conf["password"].as<String>() : "";
  String input_connect = hasINPUT ? conf["code"].as<String>() : "";

  debugPrintln("SSID: " + ssid_connect);
  debugPrintln("Password: " + pass_connect);
  debugPrintln("INPUT: " + input_connect);

  if (!hasSSID) {
    sendBadRequestResponse();
    return;
  }

  WiFi.disconnect();
  delay(wifiDelay);
  connectToWiFiNetwork(ssid_connect, pass_connect);

  if (WiFi.status() == WL_CONNECTED) {
    // Wifi Credientials are correct, check if input check needed.
    bool inputOK = true;
    if (hasINPUT && inputCheckCallback) {
      inputOK = inputCheckCallback(input_connect);
      debugPrintln("Called input check callback, result : ");
      debugPrintln((inputOK) ? "true" : "false");
    }
    handleSuccessfulConnection(inputOK, ssid_connect, pass_connect);
  } else {
    handleUnsuccessfulConnection(ssid_connect);
  }
}

void WiFiProvisioner::connectToWiFiNetwork(const String &ssid,
                                           const String &password) {
  if (password.isEmpty()) {
    WiFi.begin(ssid.c_str());
  } else {
    WiFi.begin(ssid.c_str(), password.c_str());
  }
  debugPrintln("Starting to connect");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - startTime) <= newWifiConnectionTimeout) {
    delay(wifiDelay);
    debugPrintln("Trying to connect...");
  }
}

void WiFiProvisioner::handleSuccessfulConnection(bool input_ok,
                                                 const String &ssid,
                                                 const String &password) {
  if (input_ok) {
    // Save Credientials
    saveNetworkConnectionDetails(ssid, password);
  }

  StaticJsonDocument<256> finalres;
  finalres["success"] = input_ok;
  finalres["ssid"] = ssid;
  if (!input_ok) {
    // Wifi was correct but input not, return reason to client
    WiFi.disconnect();
    delay(wifiDelay);
    finalres["reason"] = "code";
  }
  String finalout;
  serializeJson(finalres, finalout);
  m_server->send(200, "application/json", finalout);
  m_server->client().stop();

  // Make sure that response is send & displayed to client properly
  if (input_ok && restartOnSuccess) {
    // Input was ok, restarting ESP32
    delay(5000);
    ESP.restart();
  } else if (input_ok) {
    delay(5000);
    // Input was ok, signal to break from loop
    stopServerLoop(true);
  }
}

void WiFiProvisioner::handleUnsuccessfulConnection(const String &ssid) {
  // Called when connection wasn't successful with new credentials
  WiFi.disconnect();
  delay(wifiDelay);

  StaticJsonDocument<256> res_false;
  res_false["success"] = false;
  res_false["ssid"] = ssid;
  res_false["reason"] = "ssid";

  String output;
  serializeJson(res_false, output);
  m_server->send(200, "application/json", output);
  m_server->client().stop();
}

void WiFiProvisioner::saveNetworkConnectionDetails(const String &ssid,
                                                   const String &password) {
  m_preferences.begin("network", false);
  m_preferences.putString("ssid", ssid);
  m_preferences.putString("password", password);
  m_preferences.end();
}

void WiFiProvisioner::sendBadRequestResponse() {
  m_server->send(400, "application/json", "{}");
  m_server->client().stop();
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
