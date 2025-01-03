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

// #define WIFI_PROVISIONER_DEBUG // Uncomment to enable debug prints

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
 * @param rssi The RSSI value (in dBm) representing the signal strength
 * of a Wi-Fi network.
 *
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
 * @brief Default constructor for the `WiFiProvisioner::Config` struct.
 *
 * Initializes the configuration for the WiFi provisioning process with default
 * values. These defaults provide a pre-configured setup for a typical
 * provisioning page, including Access Point (AP) details, web page appearance,
 * and behavioral settings.
 *
 * Default Values:
 *
 * - `AP_NAME`: "ESP32 Wi-Fi Provisioning" - The default name for the Wi-Fi
 * Access Point.
 *
 * - `HTML_TITLE`: "Welcome to Wi-Fi Provision" - The title of the
 * provisioning web page.
 *
 * - `THEME_COLOR`: "dodgerblue" - The primary theme color used in the
 * provisioning UI.
 *
 * - `SVG_LOGO`: An SVG logo to display on the web page.
 *
 * - `PROJECT_TITLE`: "Wifi Provisioner" - The project title shown on the
 * page.
 *
 * - `PROJECT_SUB_TITLE`: "Device Setup" - The sub-title displayed below the
 * project title.
 *
 * - `PROJECT_INFO`: "Follow the steps to provision your device" -
 * Instructions displayed on the page.
 *
 * - `FOOTER_TEXT`: "All rights reserved © WiFiProvisioner" - Text displayed
 * in the footer.
 *
 * - `CONNECTION_SUCCESSFUL`: "Your device is now provisioned and ready to
 * use." - Message shown after a successful connection.
 *
 * - `RESET_CONFIRMATION_TEXT`: "This process cannot be undone." - Text for
 * factory reset confirmation.
 *
 * - `INPUT_TEXT`: "Device Key" - Label text for an additional input field (if
 * shown).
 *
 * - `INPUT_LENGTH`: 6 - Maximum length for the additional input field.
 *
 * - `SHOW_INPUT_FIELD`: `false` - Whether to display the additional input
 * field on the page.
 *
 * - `SHOW_RESET_FIELD`: `true` - Whether to display the factory reset option.
 *
 *
 * @param apName The name of the Wi-Fi Access Point (AP).
 * @param htmlTitle The title of the provisioning web page.
 * @param themeColor The primary theme color used in the UI.
 * @param svgLogo The SVG logo to display on the web page.
 * @param projectTitle The project title displayed on the page.
 * @param projectSubTitle The sub-title displayed below the project title.
 * @param projectInfo Instructions or information displayed on the page.
 * @param footerText Text displayed in the footer.
 * @param connectionSuccessful Message shown after a successful connection.
 * @param resetConfirmationText Confirmation text for factory resets.
 * @param inputText Label for an additional input field (if shown).
 * @param inputLength Maximum length for the additional input field.
 * @param showInputField Whether to display the additional input field.
 * @param showResetField Whether to display the factory reset option.
 *
 * Example Usage:
 * ```
 * WiFiProvisioner::Config customConfig(
 *     "CustomAP", "Custom Title", "darkblue", "<custom_svg>",
 *     "Custom Project", "Custom Setup", "Custom Information",
 *     "Custom Footer", "Success Message", "Are you sure?",
 *     "Custom Key", 10, true, false);
 * ```
 */
WiFiProvisioner::Config::Config(const char *apName, const char *htmlTitle,
                                const char *themeColor, const char *svgLogo,
                                const char *projectTitle,
                                const char *projectSubTitle,
                                const char *projectInfo, const char *footerText,
                                const char *connectionSuccessful,
                                const char *resetConfirmationText,
                                const char *inputText, int inputLength,
                                bool showInputField, bool showResetField)
    : AP_NAME(apName), HTML_TITLE(htmlTitle), THEME_COLOR(themeColor),
      SVG_LOGO(svgLogo), PROJECT_TITLE(projectTitle),
      PROJECT_SUB_TITLE(projectSubTitle), PROJECT_INFO(projectInfo),
      FOOTER_TEXT(footerText), CONNECTION_SUCCESSFUL(connectionSuccessful),
      RESET_CONFIRMATION_TEXT(resetConfirmationText), INPUT_TEXT(inputText),
      INPUT_LENGTH(inputLength), SHOW_INPUT_FIELD(showInputField),
      SHOW_RESET_FIELD(showResetField) {}

/**
 * @brief Constructs a new `WiFiProvisioner` instance with the specified
 * configuration.
 *
 * Initializes the WiFiProvisioner with either the provided configuration or the
 * default configuration. The configuration dictates the behavior and appearance
 * of the WiFi provisioning process, including AP details, UI elements, and
 * behavioral options.
 *
 * @param config A reference to a `WiFiProvisioner::Config` structure containing
 * the configuration for the WiFi provisioning process. If no configuration is
 * provided, the default configuration is used.
 *
 * Example Usage:
 *
 * Default configuration
 *
 * ```
 * WiFiProvisioner provisioner;
 * ```
 *
 *
 * Custom configuration
 *
 * ```
 * WiFiProvisioner::Config customConfig(
 *     "CustomAP", "Custom Title", "darkblue", "<custom_svg>",
 *     "Custom Project", "Custom Setup", "Custom Information",
 *     "Custom Footer", "Success Message", "Are you sure?",
 *     "Custom Key", 10, true, false);
 * WiFiProvisioner provisioner(customConfig);
 * ```
 *
 * @note Modifications to the configuration can be made after initialization via
 * the `getConfig()` method:
 * ```
 * provisioner.getConfig().AP_NAME = "UpdatedAP";
 * ```
 */
WiFiProvisioner::WiFiProvisioner(const Config &config)
    : _config(config), _server(nullptr), _dnsServer(nullptr),
      _apIP(192, 168, 4, 1), _netMsk(255, 255, 255, 0), _dnsPort(53),
      _serverPort(80), _wifiDelay(100), _wifiConnectionTimeout(10000),
      _serverLoopFlag(false) {}

WiFiProvisioner::~WiFiProvisioner() { releaseResources(); }

/**
 * @brief Provides access to the configuration structure.
 *
 * This method returns a reference to the `Config` structure, allowing
 * controlled access to modify the configuration values after the instance
 * has been created. Users should always modify the configuration through
 * this method and never directly edit the `Config` object submitted to
 * the constructor. This ensures consistent behavior and avoids unexpected
 * results during the provisioning process.
 *
 * @return A reference to the `Config` structure of the current WiFiProvisioner
 * instance.
 *
 * @note Modifications to the configuration should always be done through
 * this method to ensure that changes are properly reflected within the
 * `WiFiProvisioner` instance.
 *
 * Example Usage:
 * ```
 * provisioner.getConfig().AP_NAME = "UpdatedAP";
 * provisioner.getConfig().SHOW_INPUT_FIELD = true;
 * ```
 */
WiFiProvisioner::Config &WiFiProvisioner::getConfig() { return _config; }

/**
 * @brief Releases resources allocated during the provisioning process.
 *
 * This method stops the web server, DNS server, and resets the Wi-Fi mode to
 * `WIFI_STA`. It is called to clean up resources once the provisioning process
 * is complete or aborted.
 */
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

  if (!WiFi.mode(WIFI_AP_STA)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to switch to AP+STA mode");
    return false;
  }
  delay(_wifiDelay);

  if (!WiFi.softAPConfig(_apIP, _apIP, _netMsk)) {
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

  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  if (!_dnsServer->start(_dnsPort, "*", _apIP)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_ERROR,
                               "Failed to start DNS server");
    return false;
  }

  _server->on("/", [this]() { this->handleRootRequest(); });
  _server->on("/configure", HTTP_POST,
              [this]() { this->handleConfigureRequest(); });
  _server->on("/update", [this]() { this->handleUpdateRequest(); });
  _server->on("/generate_204", [this]() { this->handleRootRequest(); });
  _server->on("/fwlink", [this]() { this->handleRootRequest(); });
  _server->on("/factoryreset", HTTP_POST,
              [this]() { this->handleResetRequest(); });
  _server->onNotFound([this]() { this->handleRootRequest(); });

  _server->begin();
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Provision server started at %s",
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
 * @brief Registers a callback function to handle provisioning events.
 *
 * This callback is invoked whenever provisioning starts, allowing the user
 * to, for example, dynamically adjust the configuration (e.g., showing or
 * hiding the input field)
 *
 * @param callback A callable object or lambda that performs operations when
 * provisioning starts.
 *
 * @return A reference to the `WiFiProvisioner` instance for method chaining.
 *
 * Example:
 * ```
 * provisioner.onProvision([]() {
 *     if (hasApiKey()) {
 *         provisioner.getConfig().SHOW_INPUT_FIELD = false;
 *     } else {
 *         provisioner.getConfig().SHOW_INPUT_FIELD = true;
 *     }
 *     Serial.println("Provisioning process has started.");
 * });
 * ```
 */
WiFiProvisioner &WiFiProvisioner::onProvision(ProvisionCallback callback) {
  provisionCallback = std::move(callback);
  return *this;
}

/**
 * @brief Registers a callback function to validate user input during
 * provisioning.
 *
 * This callback is invoked to validate the additional input field (if enabled)
 * during the provisioning process. The callback should return `true` if the
 * input is valid, or `false` otherwise.
 *
 * @param callback A callable object or lambda that accepts a `const char*`
 * representing the user input and returns a `bool` indicating its validity.
 *
 * @return A reference to the `WiFiProvisioner` instance for method chaining.
 *
 * Example:
 * ```
 * provisioner.onInputCheck([](const char* input) -> bool {
 *     return strcmp(input, "1234") == 0; // Validate the input
 * });
 * ```
 */
WiFiProvisioner &WiFiProvisioner::onInputCheck(InputCheckCallback callback) {
  inputCheckCallback = std::move(callback);
  return *this;
}

/**
 * @brief Registers a callback function to handle factory reset operations.
 *
 * The callback function is triggered when a factory reset is initiated by the
 * user. It should perform necessary cleanup or reinitialization tasks required
 * for a factory reset.
 *
 * @param callback A callable object or lambda that performs operations when
 * a factory reset is triggered.
 *
 * @return A reference to the `WiFiProvisioner` instance for method chaining.
 *
 * Example:
 * ```
 * provisioner.onFactoryReset([]() {
 *     Serial.println("Factory reset triggered!");
 *     // Additional cleanup logic here
 * });
 * ```
 */
WiFiProvisioner &
WiFiProvisioner::onFactoryReset(FactoryResetCallback callback) {
  factoryResetCallback = std::move(callback);
  return *this;
}

/**
 * @brief Registers a callback function to handle successful provisioning
 * events.
 *
 * This callback is invoked after the device successfully connects to the
 * configured Wi-Fi network and validates optional user input (if required).
 *
 * @param callback A callable object or lambda that accepts the following
 * parameters:
 * - `const char* ssid`: The SSID of the connected Wi-Fi network.
 * - `const char* password`: The password of the Wi-Fi network. If the network
 * is open, this parameter will be `nullptr`.
 * - `const char* input`: The user-provided input (if enabled in the
 * configuration). If the input field is disabled, this parameter will be
 * `nullptr`.
 *
 * @return A reference to the `WiFiProvisioner` instance for method chaining.
 *
 * @note
 * - If the `SHOW_INPUT_FIELD` configuration was not enabled, the `input`
 * parameter will be `nullptr`.
 * - If the Wi-Fi network is open (no password required), the `password`
 * parameter will be `nullptr`.
 *
 * Example:
 * ```
 * provisioner.onSuccess([](const char* ssid, const char* password, const char*
 * input) { Serial.printf("Connected to SSID: %s\n", ssid); if (password) {
 *         Serial.printf("Password: %s\n", password);
 *     }
 *     if (input) {
 *         Serial.printf("Input: %s\n", input);
 *     }
 * });
 * ```
 */
WiFiProvisioner &WiFiProvisioner::onSuccess(SuccessCallback callback) {
  onSuccessCallback = std::move(callback);
  return *this;
}

/**
 * @brief Handles the HTTP `/` request.
 *
 * This function responds to the root URL (`/`) by sending an HTML page
 * composed of several predefined fragments and dynamic content based on the
 * Wi-Fi provisioning configuration.
 *
 */
void WiFiProvisioner::handleRootRequest() {
  if (provisionCallback) {
    provisionCallback();
  }

  const char *showResetField = _config.SHOW_RESET_FIELD ? "true" : "false";

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

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Calculated Content Length: %zu", contentLength);

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
  client.clear();
  client.stop();
}

/**
 * @brief Handles the HTTP `/update` request.
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

  client.clear();
  client.stop();
}

/**
 * @brief Handles the `/configure` HTTP request.
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
 * 2. Attempts to connect to the network.
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
    handleUnsuccessfulConnection("ssid");
    return;
  }

  if (input_connect && inputCheckCallback &&
      !inputCheckCallback(input_connect)) {
    WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                               "Input check callback failed.");
    handleUnsuccessfulConnection("code");
    return;
  }

  handleSuccesfulConnection();

  if (onSuccessCallback) {
    onSuccessCallback(ssid_connect, pass_connect, input_connect);
  }

  // Show success page for a while before closing the server
  delay(7000);

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
 * @brief Sends a generic HTTP 400 Bad Request response.
 */
void WiFiProvisioner::sendBadRequestResponse() {
  WiFiClient client = _server->client();

  sendHeader(client, 400, "text/html", 0);

  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_WARN,
                             "Sent 400 Bad Request response to client");

  client.clear();
  client.stop();
}

/**
 * @brief Sends a success response to the HTTP client after a successful Wi-Fi
 * connection.
 */
void WiFiProvisioner::handleSuccesfulConnection() {
  JsonDocument doc;
  doc["success"] = true;

  WiFiClient client = _server->client();

  sendHeader(client, 200, "application/json", measureJson(doc));

  serializeJson(doc, client);
  client.clear();
  client.stop();
}

/**
 * @brief Sends a failure response to the HTTP client when a Wi-Fi connection
 * or input check attempt fails.
 *
 * @param reason The reason for the failure (e.g., "ssid" or "code").
 */
void WiFiProvisioner::handleUnsuccessfulConnection(const char *reason) {
  JsonDocument doc;
  doc["success"] = false;
  doc["reason"] = reason;

  WiFiClient client = _server->client();

  sendHeader(client, 200, "application/json", measureJson(doc));

  serializeJson(doc, client);
  client.clear();
  client.stop();

  WiFi.disconnect(false, true);
}

/**
 * @brief Handles the factory reset request and invokes the registered reset
 * callback.
 *
 * This function triggers the `factoryResetCallback` if set and performs any
 * required reset operations. After the reset, the provisioning UI is displayed
 * again.
 */
void WiFiProvisioner::handleResetRequest() {
  if (factoryResetCallback) {
    factoryResetCallback();
  }
  WIFI_PROVISIONER_DEBUG_LOG(WIFI_PROVISIONER_LOG_INFO,
                             "Factory reset completed. Reloading UI.");

  WiFiClient client = _server->client();

  sendHeader(client, 200, "text/html", 0);

  client.clear();
  client.stop();
}
