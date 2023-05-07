#include <WiFiProvisioner.h>

WiFiProvisioner::WiFiProvisioner provisioner;

bool checkInputCode(const String &input) { return false; }

void setup() {
  // Only when trialing, trigger provision everytime
  provisioner.resetCredentials();

  provisioner.setInputCheckCallback(checkInputCode);
  provisioner.setShowInputField(true);
  provisioner.AP_NAME = "Another Custom AP Name";
  provisioner.SVG_LOGO =
      R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="5rem" height="5rem" preserveAspectRatio="xMidYMid meet"><g transform="translate(0 -1020.362)"><circle cx="16" cy="1036.362" r="16" fill="var(--theme-color)" fill-rule="evenodd"></circle><path fill="rgba(0,0,0,0.3)" style="line-height:normal;text-indent:0;text-align:start;text-decoration-line:none;text-decoration-style:solid;text-decoration-color:#000;text-transform:none;block-progression:tb;isolation:auto;mix-blend-mode:normal" d="M24.17 29.734a16 16 0 0 0 .475-.271 16 16 0 0 0 1.3-.93 16 16 0 0 0 1.201-1.054 16 16 0 0 0 1.092-1.172 16 16 0 0 0 .967-1.272 16 16 0 0 0 .836-1.365 16 16 0 0 0 .695-1.44 16 16 0 0 0 .383-1.05l-9.629-9.63-.02-.019A15.39 15.39 0 0 0 10.54 7a.534.534 0 0 0-.54.543.534.534 0 0 0 .541.525l3.467 3.467-.053-.02A11.426 11.426 0 0 0 10.553 11a.546.546 0 0 0-.553.555.546.546 0 0 0 .553.537l4.19 4.19c-.129-.088-.264-.164-.397-.243-.078-.046-.152-.097-.23-.14h-.003a7.392 7.392 0 0 0-.646-.313h-.002a7.37 7.37 0 0 0-.68-.25h-.002a7.328 7.328 0 0 0-2.203-.334.572.572 0 0 0-.58.58.572.572 0 0 0 .58.563l6.275 6.275a.572.572 0 0 0 .573.572l6.742 6.742zm-4.451 1.809a16 16 0 0 0 .56-.127 16 16 0 0 0 1.518-.504 16 16 0 0 0 1.46-.652 16 16 0 0 0 .526-.301l-10.37-10.371A1.993 1.993 0 0 0 12 19c-1.1 0-2 .9-2 2 0 .55.225 1.05.588 1.412l9.13 9.131z" color="#000" font-family="sans-serif" font-weight="400" transform="translate(0 1020.362)"></path><path style="line-height:normal;text-indent:0;text-align:start;text-decoration-line:none;text-decoration-style:solid;text-decoration-color:#000;text-transform:none;block-progression:tb;isolation:auto;mix-blend-mode:normal" fill="#fff" d="M.541 0A.534.534 0 0 0 0 .543a.534.534 0 0 0 .541.525c7.954 0 14.39 6.437 14.39 14.391a.534.534 0 0 0 1.069 0C16 6.928 9.072 0 .541 0Zm.012 4A.546.546 0 0 0 0 4.555a.546.546 0 0 0 .553.537c5.725 0 10.355 4.63 10.355 10.355a.546.546 0 0 0 1.092 0C12 9.132 6.868 4 .553 4ZM.58 8.002a.572.572 0 0 0-.58.58.572.572 0 0 0 .58.563 6.266 6.266 0 0 1 6.275 6.275.572.572 0 0 0 1.145 0C8 11.329 4.671 8.002.58 8.002ZM2 12c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm0 .8c.667 0 1.2.533 1.2 1.2 0 .668-.533 1.201-1.2 1.201-.667 0-1.2-.534-1.2-1.201s.533-1.2 1.2-1.2Z" color="#000" font-family="sans-serif" font-weight="400" overflow="visible" transform="translate(10 1027.362)"></path></g></svg>)rawliteral";
  provisioner.THEME_COLOR = "green";
  provisioner.HTML_TITLE = "Different Custom Provisioning Page";
  provisioner.PROJECT_TITLE = "Different Custom Project Title";
  provisioner.PROJECT_INFO = "Connect to another custom network";
  provisioner.INPUT_LENGTH = "5";
  provisioner.FOOTER_INFO = "Different Footer Info - Another Custom App";
  provisioner.INPUT_INVALID_LENGTH = "Custom message for invalid input length";
  provisioner.INPUT_NOT_VALID = "Custom not valid!";
  provisioner.CONNECTION_SUCCESSFUL =
      "Device will now restart and status led will turn to green as a sign of "
      "succesful connection";

  provisioner.connectToWiFi();
}

void loop() {
  // Your main loop code here
}
