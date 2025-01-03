# WiFi Provisioning Library for ESP32

This library provides an easy-to-use, customizable solution for setting up a modern-looking WiFi provisioning captive portal on an ESP32 device. This allows you to effortlessly provision your ESP32 with WiFi credentials and get custom input via an intuitive user interface.

> _**Note:** This library is designed for ESP32 devices and depends on the ESP32 core and its specific libraries (DNSServer, WebServer, and WiFi). Make sure you have the ESP32 core installed in your Arduino IDE before using this library._

## Features
- **Access Point Mode with Captive Portal**  
  Quickly set up WiFi provisioning with a captive portal, accessible easily from any device.
  <img src="extras/provision.gif" alt="Mobile" style="width: 20%;"/>
  
- **Lightweight and Modern Interface**  
  A sleek, responsive, and lightweight UI.
   <img src="extras/mobile.png" alt="Mobile" style="width: 20%;"/>

- **Simple Customization**  
  Easily adjust colors, logos, text, and themes to match your branding or project needs.
  <img src="extras/connect.png" alt="Customization" style="width: 20%;"/>

- **Optional Input Field**  
  Gather user-specific data (e.g., keys, codes) during the provisioning process.
  <img src="extras/input.png" alt="Optional Input" style="width: 20%;"/>
  
- **Event Callbacks**  
  Hook into provisioning events like input validation, factory reset, and provisioning start.
  <img src="extras/reset.gif" alt="Main Page" style="width: 20%;"/>


80A8F5E26A

## Installation
### Installation from ZIP file
1. Download the library as a ZIP file.
2. Open the Arduino IDE, go to **Sketch** > **Include Library** > **Add .ZIP Library**, and select the downloaded ZIP file.
3. The library will be installed and available in the **Examples** menu.

### Installation using Arduino Library Manager

1. Open the Arduino IDE, go to **Tools** > **Manage Libraries**.
2. In the Library Manager window, type `WiFiProvisioner` into the search bar.
3. Find the library in the search results, select the latest version, and click **Install**.

## Usage

1. Include the library in your Arduino sketch:

```cpp
#include <WiFiProvisioner.h>

void setup() {
  Serial.begin(9600);

  // Create the WiFiProvisioner instance
  WiFiProvisioner provisioner;

  // Configure to hide additional fields
  provisioner.getConfig().SHOW_INPUT_FIELD = false; // No additional input field
  provisioner.getConfig().SHOW_RESET_FIELD = false; // No reset field

  // Set the success callback
  provisioner.onSuccess(
      [](const char *ssid, const char *password, const char *input) {
        Serial.printf("Provisioning successful! Connected to SSID: %s\n", ssid);
        if (password) {
          Serial.printf("Password: %s\n", password);
        }
      });

  // Start provisioning
  provisioner.startProvisioning();
}

void loop() { delay(100); }
```
## Functions

#### `connectToWiFi()`
Initiates the connection to the WiFi network. If the device has stored credentials, it will use those. Otherwise, it will set up an access point and start the captive portal. For manual connection ip adress is `http://192.168.4.1/`

#### `setupAccessPointAndServer()`
Sets up the access point and server for the captive portal. This function is called automatically by `connectToWiFi()` if necessary.

#### `resetCredentials()`
Resets the stored WiFi credentials.

#### `setConnectionTimeout(unsigned long timeout)`
Sets the connection timeout for the WiFi connection attempt. Default timeout for existing connection is forever. If you want to try connection for a certain time, you can assign a timeout:
```cpp
provisioner.setConnectionTimeout(10000);
```
This will try to connect for 10 seconds, and if not successful, will start the provisioning.

#### `setShowInputField(bool value)`
Shows or hides the input field in the captive portal.

#### `setRestartOnSuccess(bool value)`
Sets whether the device should restart upon successful connection and input validation.

#### `enableSerialDebug(bool enable)`
Enables or disables serial debug messages.

## Callback Types

#### `onProvision`
Defines actions to perform at the start of the provisioning process. 
- Use this callback to conditionally show or hide input fields, update interface text etc..
- The callback is invoked everytime before the provisioning portal is served.
- This callback is also triggered **after a factory reset**, ensuring the provisioning page reflects the latest configuration when it is served again.

Example:
```cpp
provisioner.onProvision([]() {
  if (hasApiKey()) {
    provisioner.getConfig().SHOW_INPUT_FIELD = false;
    Serial.println("API key exists. Input field hidden.");
  } else {
    provisioner.getConfig().SHOW_INPUT_FIELD = true;
    Serial.println("No API key found. Input field shown.");
  }
});
```
#### `onInputCheck`
Validates user input during the provisioning process. The callback function takes a single parameter of type `const char*` and returns a `bool` to indicate whether the input is valid.

- This callback acts as a **gatekeeper** to ensure the input meets specific criteria before completing the provisioning process and calling the `onSuccess` callback.
- If the input validation fails (i.e., the callback returns `false`), an error message will be displayed to the user indicating the input is invalid.
- **WiFi is already connected successfully** to reach this callback, allowing you to perform checks that require an active network connection (e.g., API calls or HTTP requests).
  
**Parameters**:
- `const char* input`: The user-provided input to validate.

Example:
```cpp
provisioner.onInputCheck([](const char *input) -> bool {
  Serial.printf("Checking if input code equals to 1234: %s\n", input);
  return strcmp(input, "1234") == 0; // Validate input
})
```
#### `onFactoryReset`
Allows you to define custom actions to execute when a factory reset is triggered. This is the ideal place to clear saved data, such as API keys, WiFi credentials, or any other stored inputs.

- Use this callback to perform cleanup tasks.
- The callback is invoked whenever a factory reset is initiated through the provisioning portal.

Example:
```cpp
provisioner.onFactoryReset([]() { 
  Serial.println("Factory reset triggered!"); 
});
```
#### `onSuccess`
Invoked after the device has been successfully connected to the Wi-Fi network and user input has been validated (if enabled). This is the final step in the provisioning process, making it an ideal place to handle post-provisioning logic, such as saving configuration.

- Use this callback to store Wi-Fi credentials and input details.

**Parameters**:
- `const char* ssid`: The SSID of the connected Wi-Fi network.
- `const char* password`: The password for the Wi-Fi network (or `nullptr` for open networks).
- `const char* input`: The user-provided input (or `nullptr` if the input field is disabled).

Example:
```cpp
provisioner.onSuccess([](const char *ssid, const char *password, const char *input) {
  Serial.printf("Provisioning successful! SSID: %s\n", ssid);
  preferences.begin("wifi-provision", false);
  // Store the credentials and API key in preferences
  preferences.putString("ssid", String(ssid));
  if (password) {
    preferences.putString("password", String(password));
  }
  if (input) {
    preferences.putString("apikey", String(input));
  }
  preferences.end();
  Serial.println("Credentials and API key saved.");
});
```

## Customization

You can customize various aspects of the library, such as the HTML content, input validation, and behavior after a successful connection. The following customization options are available:

- `AP_NAME`: Access Point name
- `HTML_TITLE`: Web page title
- `THEME_COLOR`: Theme color
- `SVG_LOGO`: Custom logo in SVG format
- `PROJECT_TITLE`: Title displayed on the provisioning page
- `PROJECT_INFO`: Description or instructions for the user
- `INPUT_TEXT`: Label for the input field
- `INPUT_PLACEHOLDER`: Placeholder text for the input field
- `INPUT_LENGTH`: Maximum input length
- `FOOTER_INFO`: Footer text
- `INPUT_INVALID_LENGTH`: Error message for invalid input length
- `INPUT_NOT_VALID`: Error message for invalid input
- `CONNECTION_SUCCESSFUL`: Success message displayed after a successful connection
- `RESET_CONFIRMATION_TEXT`: Confirmation text for resetting the device

For example, to change the `PROJECT_TITLE`:

```cpp
provisioner.PROJECT_TITLE = "Custom Project Title";
```
To set a custom INPUT_LENGTH:
```cpp
provisioner.INPUT_LENGTH = "5";
```
To set a custom SVG logo:
```cpp
provisioner.SVG_LOGO = R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" width="50" height="50" viewBox="0 0 50 50"><rect width="50" height="50" rx="10" ry="10" fill="#f00"/></svg>)rawliteral";
```
All of these customization options are set as strings

##  Examples
The library includes examples that demonstrate different customization options. To access the examples, go to File > Examples > WiFiProvisioner in the Arduino IDE.

### Example: Customizing Access Point Name and Theme Color

```cpp
#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

void setup() {  
  // Customize the Access Point name and theme color
  provisioner.AP_NAME = "CustomAPName";
  provisioner.THEME_COLOR = "#FFA500";
  
  provisioner.connectToWiFi();
}

void loop() {
  // Your application logic here
}
```
### Example: Using Input Validation Callback

```cpp
#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

bool inputValidationCallback(const String& input) {
  // Check if the input is valid, e.g., checking if it's the string "1234"
  return input == "1234";
}

void setup() {
  Serial.begin(115200);
  
  // Set the input validation callback
  provisioner.setInputCheckCallback(inputValidationCallback);

  // Enable the input field
  provisioner.setShowInputField(true);

  // Customize the error message for invalid input
  provisioner.INPUT_NOT_VALID = "Please enter the correct code.";

  // Change the input text and placeholder text
  provisioner.INPUT_TEXT = "Enter access code:";
  provisioner.INPUT_PLACEHOLDER = "Access code";

  // Connect to WiFi or start the provisioning process
  provisioner.connectToWiFi();
}

void loop() {
  // Your application logic here
}

```
### Example: Advanced Factory Reset with Conditional Input Field and Single Button Press Trigger
```cpp
#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

const int buttonPin = 2; // GPIO pin number for the button

bool inputValidationCallback(const String& input) {
  // Check if the input is valid, e.g., checking if it's "1234"
  return input == "1234";
}

void factoryReset() {
  // Perform your factory reset logic here
  Serial.println("Factory reset triggered.");

  // Disable the input field after factory reset
  provisioner.setShowInputField(true);
}

void onProvision() {
  // Use a simple if statement to conditionally show the input field
  // For example, if a certain condition is met, show the input field
  if (/* your condition */) {
    provisioner.setShowInputField(true);
  } else {
    provisioner.setShowInputField(false);
  }
}

void setup() {
  Serial.begin(115200);

  // Set the input validation callback
  provisioner.setInputCheckCallback(inputValidationCallback);

  // Set the factory reset callback
  provisioner.setFactoryResetCallback(factoryReset);

  // Set the onProvision callback
  provisioner.setOnProvisionCallback(onProvision);

  // Set the initial input text and placeholder text
  provisioner.INPUT_TEXT = "Enter custom value:";
  provisioner.INPUT_PLACEHOLDER = "Custom value";

  // Set the input length to 4
  provisioner.INPUT_LENGTH = "4"; // Limit the input length to 4 characters

  provisioner.connectToWiFi();

  // Set up the button pin as input
  pinMode(buttonPin, INPUT);
}

void loop() {
  // Read the state of the button
  int buttonState = digitalRead(buttonPin);

  // If the button is pressed, manually start the provisioning by calling the setupAccessPointAndServer() function
  if (buttonState == HIGH) {
    provisioner.setupAccessPointAndServer();
  }
}

```
### License

This library is licensed under the [MIT License](https://opensource.org/licenses/MIT). For more details, please see the `LICENSE` file in the repository.
### Contributing

We welcome contributions to this library! If you have found a bug, have a feature request, or want to contribute code, please open an issue or submit a pull request on the [GitHub repository](https://github.com/SanteriLindfors/WiFiProvisioner).
