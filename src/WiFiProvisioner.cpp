#include "WiFiProvisioner.h"
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
#define WIFI_PROVISIONER_DEBUG_LOG(level, format, ...)                         \
  do {                                                                         \
  } while (0) // Empty macro
#endif

namespace {

/**
 * @brief Converts a Received Signal Strength Indicator (RSSI) value to a signal
 * strength level.
 *
 * This function maps RSSI values to a step level ranging from 0 to 4 based on
 * predefined minimum and maximum RSSI thresholds. The returned level provides
 * an approximation of the signal quality.
 *
 * @param rssi The RSSI value (usually in dBm) representing the signal strength
 * of a Wi-Fi network. Typical values range from -100 (weakest) to -55
 * (strongest).
 * @return An integer in the range [0, 4], where 0 indicates very poor signal
 * strength and 4 indicates excellent signal strength.
 */
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

/**
 * @brief Scans for available Wi-Fi networks and populates a JSON document with
 * the results.
 *
 * This function performs a Wi-Fi network scan, collecting information about
 * each detected network, including its SSID, signal strength (converted to a
 * level), and authentication mode. The results are stored in the provided `doc`
 * JSON document under the "network" array.
 *
 * @param doc A reference to a `JsonDocument` object where the scan results will
 * be stored. The document will contain an array of networks, each represented
 * as a JSON object with the following keys:
 *
 *            - `ssid`: The network SSID (string).
 *
 *            - `rssi`: The signal strength level (integer, 0 to 4).
 *
 *            - `authmode`: The authentication mode (0 for open, 1 for secured).
 */
void networkScan(JsonDocument &doc) {
  JsonArray networks = doc["network"].to<JsonArray>();

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Starting Network Scan...");
  int n = WiFi.scanNetworks(false, false);
  if (n) {
    for (int i = 0; i < n; ++i) {
      JsonObject network = networks.add<JsonObject>();
      network["rssi"] = convertRRSItoLevel(WiFi.RSSI(i));
      network["ssid"] = WiFi.SSID(i);
      network["authmode"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 0 : 1;
    }
  }
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Network scan complete");
}

/**
 * @brief Sends an HTTP header response to the client.
 *
 * This function constructs and sends the HTTP response header to the connected
 * client, specifying the HTTP status code, content type, and content length.
 *
 * @param client A reference to the `WiFiClient` object representing the
 * connected client.
 * @param statusCode The HTTP status code (e.g., 200 for success, 404 for not
 * found).
 * @param contentType The MIME type of the content (e.g., "text/html",
 * "application/json").
 * @param contentLength The size of the content in bytes to be sent in the
 * response.
 */
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

/**
 * @brief Constructs a new WiFiProvisioner instance with the specified
 * configuration.
 *
 * @param config A reference to a `WiFiProvisioner::Config` structure
 * containing the configuration for the WiFi provisioning process. This
 * includes settings such as the Access Point name, HTML title, theme color,
 * and other UI elements.
 *
 * Example:
 * ```
 * WiFiProvisioner::Config customConfig;
 * customConfig.AP_NAME = "MyCustomAP";
 * WiFiProvisioner provisioner(customConfig);
 * ```
 */
WiFiProvisioner::WiFiProvisioner(const Config &customConfig)
    : _config(customConfig), _server(nullptr), _dnsServer(nullptr),
      _apIP(192, 168, 4, 1), netMsk(255, 255, 255, 0) {}

WiFiProvisioner::~WiFiProvisioner() { releaseResources(); }

void WiFiProvisioner::releaseResources() {
  _serverLoopFlag = false;

  // Webserver
  if (_server != nullptr) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO, "Stopping server");
    _server->stop();
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

  // WiFi
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(_wifiDelay);
  }
}

/**
 * @brief Starts the provisioning process, setting up the device in Access
 * Point (AP) mode with a captive portal for Wi-Fi configuration.
 *
 * Access Instructions:
 *
 * 1. Open your device's Wi-Fi settings.
 *
 * 2. Connect to the Wi-Fi network specified by `_config.AP_NAME`.
 *    - Default: "ESP32 Wi-Fi Provisioning".
 *
 * 3. Once connected, the provisioning page should open automatically. If it
 * does not, open a web browser and navigate to `192.168.4.1`.
 *
 * @return `true` if provisioning was successful `false` otherwise.
 *
 * Example Usage:
 * ```
 * WiFiProvisioner provisioner;
 * if (!provisioner.startProvisioning()) {
 *     Serial.println("Provisioning failed. Check logs for details.");
 * }
 * ```
 *
 * @note
 * - The `Config` object within the `WiFiProvisioner` is used to customize the
 * behavior and appearance of the provisioning system.
 */
bool WiFiProvisioner::startProvisioning() {
  WiFi.disconnect(false, true);
  delay(_wifiDelay);

  releaseResources();

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
  _server->on("/factoryreset", HTTP_POST,
              [this]() { this->handleResetRequest(); });
  _server->onNotFound([this]() { this->handleRootRequest(); });

  // Start the web server
  _server->begin();
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "HTTP server started at %s",
                             WiFi.softAPIP().toString());

  loop();
  return true;
}

/**
 * @brief Handles the main loop for the Wi-Fi provisioning process.
 *
 * This function continuously processes DNS and HTTP server requests while the
 * provisioning process is active. It ensures that DNS requests are resolved to
 * redirect clients to the provisioning page and handles HTTP client
 * interactions.
 *
 * The loop runs until the `_serverLoopFlag` is set to `true`, indicating that
 * provisioning is complete or the server needs to shut down.
 */
void WiFiProvisioner::loop() {
  while (!_serverLoopFlag) {
    // DNS
    if (_dnsServer) {
      _dnsServer->processNextRequest();
    }

    // HTTP
    if (_server) {
      _server->handleClient();
    }
  }
  releaseResources();
}

/**
 * @brief Sets the callback function to validate user input during
 * provisioning.
 *
 * @param callback A callable object or lambda that accepts a `const char*` as
 * input and returns a `bool`. The callback should return `true` if the input
 * is valid, or `false` otherwise.
 *
 * @note Wifi is succesfully connected when this
 * function gets called.
 *
 * Example:
 * ```
 * provisioner->setInputCheckCallback([](const char* input) {
 *     return strcmp(input, "1234") == 0; // Example: Validate input as "1234"
 * });
 * ```
 */
void WiFiProvisioner::setInputCheckCallback(InputCheckCallback callback) {
  inputCheckCallback = std::move(callback);
}

/**
 * @brief Sets the callback function to handle factory reset operations.
 *
 * The callback function is invoked when a factory reset is triggered. The
 * callback should perform necessary cleanup operations and return a boolean
 * value indicating whether the factory reset was successful or not.
 *
 * @param callback A callable object or lambda to execute when a factory reset
 * request is made. The callback should return `true` if the reset is
 * successful, and `false` otherwise.
 *
 * Example:
 * ```
 * provisioner->setFactoryResetCallback([]() -> bool {
 *     Serial.println("Factory reset triggered!");
 *     // Perform additional cleanup here
 *     bool resetSuccessful = true; // Change to false if reset fails
 *     return resetSuccessful;
 * });
 * ```
 */
void WiFiProvisioner::setFactoryResetCallback(FactoryResetCallback callback) {
  factoryResetCallback = std::move(callback);
}

/**
 * @brief Sets the callback function to handle a successful provisioning
 * event.
 *
 * @param callback A callable object or lambda that accepts three `const
 * char*` parameters: `ssid`, `password`, and `input`. This is executed after
 * a successful connection to Wi-Fi and optional input validation has been
 * succesfully completed.
 *
 * @note
 * - If the `SHOW_INPUT_FIELD` configuration was not enabled, the `input`
 * parameter will be `nullptr`.
 *
 * - If the Wi-Fi network is open (no password required), the `password`
 * parameter will also be `nullptr`.
 *
 * Example:
 * ```
 * provisioner->setOnSuccessCallback([](const char* ssid, const char*
 * password, const char* input) { Serial.printf("Connected to SSID: %s with
 * password: %s\n", ssid, password); if (input) { Serial.printf("Input: %s\n",
 * input);
 *     }
 * });
 * ```
 */
void WiFiProvisioner::setOnSuccessCallback(OnSuccessCallback callback) {
  onSuccessCallback = std::move(callback);
}

/**
 * @brief Handles the HTTP `/` request by serving the main HTML page for the
 * Wi-Fi provisioning UI.
 *
 * This function responds to the root URL (`/`) by sending an HTML page
 * composed of several predefined fragments and dynamic content based on the
 * Wi-Fi provisioning configuration.
 *
 */
void WiFiProvisioner::handleRootRequest() {
  const char *showResetField = _config.SHOW_RESET_FIELD ? "true" : "false";
  const char *showInputField = _config.SHOW_INPUT_FIELD ? "true" : "false";

  char inputLengthStr[12];
  snprintf(inputLengthStr, sizeof(inputLengthStr), "%d", _config.INPUT_LENGTH);

  size_t contentLength =
      strlen_P(index_html1) + strlen(_config.HTML_TITLE) +
      strlen_P(index_html2) + strlen(_config.THEME_COLOR) +
      strlen_P(index_html3) + strlen(_config.SVG_LOGO) + strlen_P(index_html4) +
      strlen(_config.PROJECT_TITLE) + strlen_P(index_html5) +
      strlen(_config.PROJECT_SUB_TITLE) + strlen_P(index_html6) +
      strlen(_config.PROJECT_INFO) + strlen_P(index_html7) +
      strlen(_config.INPUT_TEXT) + strlen_P(index_html8) +
      strlen(inputLengthStr) + strlen_P(index_html9) +
      strlen(_config.CONNECTION_SUCCESSFUL) + strlen_P(index_html10) +
      strlen(_config.FOOTER_TEXT) + strlen_P(index_html11) +
      strlen(_config.RESET_CONFIRMATION_TEXT) + strlen_P(index_html12) +
      strlen(showResetField) + strlen_P(index_html13);

  WiFiClient client = _server->client();
  sendHeader(client, 200, "text/html", contentLength);

  client.write_P(index_html1, strlen_P(index_html1));
  client.print(_config.HTML_TITLE);
  client.write_P(index_html2, strlen_P(index_html2));
  client.print(_config.THEME_COLOR);
  client.write_P(index_html3, strlen_P(index_html3));
  client.print(_config.SVG_LOGO);
  client.write_P(index_html4, strlen_P(index_html4));
  client.print(_config.PROJECT_TITLE);
  client.write_P(index_html5, strlen_P(index_html5));
  client.print(_config.PROJECT_SUB_TITLE);
  client.write_P(index_html6, strlen_P(index_html6));
  client.print(_config.PROJECT_INFO);
  client.write_P(index_html7, strlen_P(index_html7));
  client.print(_config.INPUT_TEXT);
  client.write_P(index_html8, strlen_P(index_html8));
  client.print(inputLengthStr);
  client.write_P(index_html9, strlen_P(index_html9));
  client.print(_config.CONNECTION_SUCCESSFUL);
  client.write_P(index_html10, strlen_P(index_html10));
  client.print(_config.FOOTER_TEXT);
  client.write_P(index_html11, strlen_P(index_html11));
  client.print(_config.RESET_CONFIRMATION_TEXT);
  client.write_P(index_html12, strlen_P(index_html12));
  client.print(showResetField);
  client.write_P(index_html13, strlen_P(index_html13));
  client.flush();
  client.stop();
}

/**
 * @brief Handles the HTTP `/update` request by responding with a JSON payload
 *        containing network scan results and configuration information.
 *
 * This function serves the `/update` endpoint of the web server. It generates
 * a JSON response that includes a list of available Wi-Fi networks with
 * details such as SSID, signal strength (RSSI), and authentication mode. It
 * also includes a flag `show_code` indicating whether the input field for
 * additional credentials is enabled.
 *
 * Example JSON Response:
 * ```
 * {
 *   "show_code": "false",
 *   "network": [
 *     { "ssid": "Network1", "rssi": 4, "authmode": 1 },
 *     { "ssid": "Network2", "rssi": 2, "authmode": 0 },
 *     { "ssid": "Network3", "rssi": 3, "authmode": 1 }
 *   ]
 * }
 * ```
 *
 * @note
 * - The `authmode` field indicates the security mode of the network:
 *   - `0`: Open (no password required)
 *   - `1`: Secured (password required)
 */
void WiFiProvisioner::handleUpdateRequest() {
  JsonDocument doc;

  doc["show_code"] = _config.SHOW_INPUT_FIELD;
  networkScan(doc);

  WiFiClient client = _server->client();
  sendHeader(client, 200, "application/json", measureJson(doc));
  serializeJson(doc, client);

  client.flush();
  client.stop();
}

/**
 * @brief Handles the `/configure` HTTP request to process Wi-Fi configuration
 * details provided by the user.
 *
 * This function expects a JSON payload containing Wi-Fi credentials and an
 * optional input field. It attempts to connect to the specified network and
 * validates the optional input field if provided.
 *
 * 1. Parses the incoming JSON payload for:
 *    - `ssid` (required): The Wi-Fi network name.
 *    - `password` (optional): The Wi-Fi password.
 *    - `code` (optional): Additional input for custom validation.
 *
 * 2. Validates the `ssid` and attempts to connect to the network.
 *
 * 3. If the `inputCheckCallback` is set, invokes it and returns an
 * unsuccessful response if the validation fails.
 *
 * 4. If the connection and input check is successful, invokes the
 * `onSuccessCallback` with the `ssid`, `password` and `input`.
 *
 * Example JSON Payload:
 * ```
 * {
 *   "ssid": "MyNetwork",
 *   "password": "securepassword",
 *   "code": "1234"
 * }
 * ```
 */
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

  WiFi.disconnect(false, true);
  delay(_wifiDelay);

  if (!connect(ssid_connect, pass_connect)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                               "Failed to connect to WiFi: %s with password %s",
                               ssid_connect, pass_connect ? pass_connect : "");
    handleUnsuccessfulConnection(ssid_connect, "ssid");
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
  // delay(_onSuccessDelay);

  // Signal to break from loop
  _serverLoopFlag = true;
}

/**
 * @brief Attempts to connect to the specified Wi-Fi network.
 *
 * @param ssid The SSID of the Wi-Fi network.
 * @param password The password for the Wi-Fi network. Pass `nullptr` or an
 * empty string for open networks.
 * @return `true` if the connection is successful; `false` otherwise.
 */
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

/**
 * @brief Sends a success response to the HTTP client after a successful Wi-Fi
 * connection.
 *
 * @param ssid The SSID of the connected Wi-Fi network.
 */
void WiFiProvisioner::handleSuccesfulConnection(const char *ssid) {
  JsonDocument doc;
  doc["success"] = true;
  doc["ssid"] = ssid;

  WiFiClient client = _server->client();

  sendHeader(client, 200, "application/json", measureJson(doc));

  serializeJson(doc, client);
  client.flush();
  client.stop();
}

/**
 * @brief Sends a failure response to the HTTP client when a Wi-Fi connection
 * or input check attempt fails.
 *
 * @param ssid The SSID of the Wi-Fi network.
 * @param reason The reason for the failure (e.g., "ssid" or "code").
 */
void WiFiProvisioner::handleUnsuccessfulConnection(const char *ssid,
                                                   const char *reason) {
  JsonDocument doc;
  doc["success"] = false;
  doc["ssid"] = ssid;
  doc["reason"] = reason;

  WiFiClient client = _server->client();

  sendHeader(client, 200, "application/json", measureJson(doc));

  serializeJson(doc, client);
  client.flush();
  client.stop();

  WiFi.disconnect(false, true);
}

/**
 * @brief Sends a generic HTTP 400 Bad Request response.
 */
void WiFiProvisioner::sendBadRequestResponse() {
  _server->send(400, "application/json", "{}");
  _server->client().flush();
  _server->client().stop();
}

void WiFiProvisioner::handleResetRequest() {
  // resetCredentials();
  if (factoryResetCallback) {
    bool success = factoryResetCallback();
    if (!success) {
      WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                                 "Factory reset callback failed.");
      return;
    }
  }
  handleRootRequest();
}
