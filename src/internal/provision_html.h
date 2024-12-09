#ifndef PROVISION_HTML_H
#define PROVISION_HTML_H

#include <Arduino.h>

namespace WiFiProvisioner {

static constexpr const char index_html1[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <title>)rawliteral";
// HTML_TITLE
static constexpr const char index_html2[] PROGMEM = R"rawliteral(</title>
          <meta charset="utf-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0,
              maximum-scale=1">
          <!-- Styles -->
          <style>
        :root {
          --card-background: #ffffff;
          --font-color: #1a1f36;
          --theme-color:)rawliteral";
// THEME_COLOR
static constexpr const char index_html3[] PROGMEM = R"rawliteral(;
      }
      * {
        font-family: Poppins-Regular,sans-serif;
        color:var(--font-color);
      }
      body {
        touch-action: manipulation;
        
        }
      table {
        border-collapse: collapse;
        width: 100%;
        margin-bottom: 0.5rem;
        margin-top: 0.5rem;
        }
      input[type="text"]:disabled ,input[type="password"]:disabled ,input[type="radio"]:disabled {
        cursor: not-allowed;
        background-color: rgb(229, 229, 229) !important;
        pointer-events:none;
      }
      th, td {
        text-align: left;
        padding: 0.5rem;
        white-space: nowrap;
      }
      .signal, .radiossid{
        text-align: center;
      }
      .card {
        box-shadow: 0 0.5rem 1rem 0 rgb(0 0 0 / 20%);
        transition: 0.3s;
        border-radius: 0.3125rem; /* 0.3125rem rounded corners */
        padding-bottom: 1.25rem;
        background: var(--card-background);
        max-width: 50rem;
        margin: auto;
        text-align: center;
      }
      .container {
        padding-top: 1.25rem;
        text-align:center;
      }
      #hidden-network-selector{
        margin-left: 7%;
        margin-right: 7%;
        text-align: left;
      }
      input[type="radio"]:checked {
        accent-color: var(--theme-color) !important;
      }
      .icon_button{
        background-color: transparent;
        background-repeat: no-repeat;
        border: none;
        cursor: pointer;
        overflow: hidden;
        outline: none;
      }
      .icon_button:disabled{
        cursor: not-allowed; 
        pointer-events:none;
      }
      .icon_button:disabled > #refresh-icon { opacity: 0.5; }
      .icn-spinner {
        animation: spin-animation 0.5s infinite;
        display: inline-block;
      }
      @keyframes spin-animation {
        0% {
          transform: rotate(0deg);
        }
        100% {
          transform: rotate(359deg);
        }
      }
      .textinput{
        width: 75%;
        box-sizing: border-box;
        max-width: 18.75rem;
        padding: 0.625rem;
        border: 0.125rem solid #cccccc;
        -webkit-border-radius: 0.3125rem;
        border-radius: 0.3125rem;
        -webkit-transition: 0.5s;
        transition: 0.5s;
        outline: none;
      }
      #password{
        padding: 0.625rem 1.5625rem 0.625rem 0.625rem;
      }
      .textinput:focus {
        border-color: var(--theme-color);
      }
      label {
        display: block;
        font-size:0.8125rem;
        padding-bottom: 0.125rem;
      }
      #footer .copyright {
        width: 100%;
        margin: 2.5em 0 2em 0;
        font-size: 0.8em;
        text-align: center;
      }
      .error-message {
        color: #cc0033;
        display: block;
        font-size: 0.75rem;
        line-height: 0.9375rem;
        margin: 0.3125rem 0 0;
        
            }
      .error input[type=text], .error input[type=password]{
          background-color: #fce4e4;
          border-color: #cc0033;
      }
      .btn-process {
          background-color: var(--theme-color);
          box-shadow: 0 0.5rem 2rem 0 rgb(0 0 0 / 20%);
          border: none;
          outline: none;
          padding: 0.625rem;
          border-radius: 0.3125rem;
          color: #fff;
          cursor: pointer;
      }
      .btn-process:disabled{
          cursor: not-allowed; 
          pointer-events:none;
          opacity: 0.5;
      }
      #connecting-ring:after {
        content: "";
        display: inline-block;
        width: 1em;
        margin-left: 0.5rem;
        height: 1em;
        vertical-align: middle;
        border-radius: 50%;
        border: 0.1875rem solid #fff;
        border-color: #fff transparent #fff transparent;
        animation: ring 1.2s linear infinite;
      }
      @keyframes ring {
        0% {
          transform: rotate(0deg);
        }
        100% {
          transform: rotate(360deg);
        }
        }
      .wrapper{
        display:flex;
        justify-content:center;
        align-items:center;
      }
      .checkmark__circle{
        stroke-dasharray: 166;
        stroke-dashoffset: 166;
        stroke-width: 2;stroke-miterlimit: 10;
        stroke: #7ac142;
        fill: none;
        animation: stroke 0.6s cubic-bezier(0.65, 0, 0.45, 1) forwards}
      .checkmark{
        width: 3.5rem;
        height: 3.5rem;
        border-radius: 50%;
        display: block;
        stroke-width: 2;
        stroke: #fff;
        stroke-miterlimit: 10;
        margin-top: 3rem;
        box-shadow: inset 0rem 0rem 0rem #7ac142;
        animation: fill .4s ease-in-out .4s forwards, scale .3s ease-in-out .9s both
      }
      .checkmark__check{
        transform-origin: 50% 50%;
        stroke-dasharray: 48;
        stroke-dashoffset: 48;
        animation: stroke 0.3s cubic-bezier(0.65, 0, 0.45, 1) 0.8s forwards
      }
      @keyframes stroke{100%{stroke-dashoffset: 0}}
      @keyframes scale{0%, 100%{transform: none}50%{transform: scale3d(1.1, 1.1, 1)}}
      @keyframes fill{100%{box-shadow: inset 0rem 0rem 0rem 1.875rem #7ac142}}
      .header {
        text-align: center;
      }
      a.disabled {
        pointer-events: none;
      }
    </style>
    </head>
    <body>
        <div class="header">
)rawliteral";
// SVG_LOGO
static constexpr const char index_html4[] PROGMEM =
    R"rawliteral(<h2 style="margin-top: 0.5rem;margin-bottom: 0rem;">)rawliteral";
// PROJECT_TITLE
static constexpr const char index_html5[] PROGMEM =
    R"rawliteral(</h2><h4 style="margin-top:0.2rem;">Wifi Provision</h4><p id="codetext" style="opacity: 0.5;">)rawliteral";
// PROJECT_INFO
static constexpr const char index_html6[] PROGMEM =
    R"rawliteral(</p><!-- Device Block -->
                    </div>

                    <div id="main-card" class="card">
                        <h3 style="padding-top: 1rem;margin: 0rem;">Connect to
                            Wifi</h3>
                        <div style="overflow-x: auto;">
                            <form id="network_form">
                                <table>
                                    <thead>
                                        <tr>
                                            <th class="radiossid">
                                                <button class="icon_button"
                                                    type="button"
                                                    onclick="loadSSID();">
                                                    <svg
                                                        xmlns="http://www.w3.org/2000/svg"
                                                        id="refresh-icon"
                                                        style="vertical-align:
                                                        -0.125em;" width="1em"
                                                        height="1em"
                                                        preserveAspectRatio="xMidYMid
                                                        meet" viewBox="0 0 1536
                                                        1536"><path
                                                            fill="var(--font-color)"
                                                            d="M1511 928q0 5-1
                                                            7q-64 268-268
                                                            434.5T764
                                                            1536q-146
                                                            0-282.5-55T238
                                                            1324l-129 129q-19
                                                            19-45
                                                            19t-45-19t-19-45V960q0-26
                                                            19-45t45-19h448q26 0
                                                            45 19t19
                                                            45t-19 45l-137
                                                            137q71 66 161
                                                            102t187 36q134 0
                                                            250-65t186-179q11-17
                                                            53-117q8-23
                                                            30-23h192q13 0 22.5
                                                            9.5t9.5
                                                            22.5zm25-800v448q0
                                                            26-19 45t-45
                                                            19h-448q-26
                                                            0-45-19t-19-45t19-45l138-138Q969
                                                            256 768 256q-134
                                                            0-250
                                                            65T332 500q-11 17-53
                                                            117q-8 23-30
                                                            23H50q-13
                                                            0-22.5-9.5T18
                                                            608v-7q65-268
                                                            270-434.5T768 0q146
                                                            0 284 55.5T1297
                                                            212l130-129q19-19
                                                            45-19t45 19t19
                                                            45z"/></svg>
                                                    </button>
                                                </th>
                                                <th>SSID</th>
                                                <th class="signal">
                                                    Signal
                                                </th>
                                            </tr>
                                        </thead>
                                        <tbody id="table-body">
                                            <!-- Table Contents -->
                                        </tbody>
                                    </table>
                                </div>

                                <div id="hidden-network-selector">
                                    <p>Hidden Network ?</p>
                                    <div>
                                        <input type="radio"
                                            id="hiddennetwork_radio"
                                            name="ssid" value="hiddennetwork"
                                            onclick="onRadio(this);"> Hidden
                                        Network</input>
                                </div>
                            </div>

                            <div class="container" id="hiddenNetwork"
                                style="display:none;">
                                <label for="ssdiInput">Network SSID</label>
                                <div id="error-ssid-input">
                                    <input type="text" name="hidden_ssid"
                                        id="ssid" class="textinput"
                                        placeholder="SSID">
                                    <div id="error-ssid-message"
                                        class="error-message"></div>
                                </div>
                            </div>

                            <div class="container" id="hiddenPassword"
                                style="display:none;">
                                <label for="password">Network Password</label>
                                <div id="error-password-input">
                                    <input type="password" name="password"
                                        id="password" style="position:
                                        relative;" class="textinput"
                                        placeholder="Password">
                                    <i style="margin-left: -1.25rem;position:
                                        absolute;margin-top: 0.5625rem;padding:
                                        0rem;">
                                        <button class="icon_button"
                                            type="button" style="padding:0rem"
                                            onclick="tooglePassShow();">
                                            <svg
                                                xmlns="http://www.w3.org/2000/svg"
                                                style="vertical-align:
                                                -0.125em;" width="1em"
                                                height="1em"
                                                preserveAspectRatio="xMidYMid
                                                meet" viewBox="0 0 24 24"><path
                                                    id="eye_icon"
                                                    fill="var(--font-color)"
                                                    d="M10.94 6.08A6.93 6.93 0 0
                                                    1 12 6c3.18 0 6.17 2.29 7.91
                                                    6a15.23 15.23 0 0 1-.9
                                                    1.64a1 1 0 0 0-.16.55a1 1 0
                                                    0 0 1.86.5a15.77 15.77 0 0 0
                                                    1.21-2.3a1 1 0 0 0
                                                    0-.79C19.9 6.91 16.1 4 12
                                                    4a7.77 7.77 0 0 0-1.4.12a1 1
                                                    0 1 0 .34 2ZM3.71 2.29a1 1 0
                                                    0 0-1.42 1.42l3.1 3.09a14.62
                                                    14.62 0 0 0-3.31 4.8a1 1 0 0
                                                    0 0 .8C4.1 17.09 7.9 20 12
                                                    20a9.26 9.26 0 0 0
                                                    5.05-1.54l3.24 3.25a1 1 0 0
                                                    0 1.42 0a1 1 0 0 0
                                                    0-1.42Zm6.36 9.19l2.45
                                                    2.45A1.81 1.81 0 0 1 12 14a2
                                                    2 0 0 1-2-2a1.81 1.81 0 0 1
                                                    .07-.52ZM12 18c-3.18
                                                    0-6.17-2.29-7.9-6a12.09
                                                    12.09 0 0 1 2.7-3.79L8.57
                                                    10A4 4 0 0 0 14 15.43L15.59
                                                    17A7.24 7.24 0 0 1 12 18Z"/></svg>
                                            </button>
                                        </i>
                                    </div>
                                    <div id="error-password-message"
                                        class="error-message"></div>
                                </div>

                                <div class="container" id="codeinputBlock"
                                    style="display:none;"> <!-- style="display:none;"> -->
                                    <div style="text-align:center;""><label
                                            for="code">)rawliteral";
// INPUT_TEXT
const char index_html7[] PROGMEM = R"rawliteral(</label></div>
                                    <div id="error-code-input">
                                        <input type="text" name="code" id="code"
                                            class="textinput"
                                            placeholder=")rawliteral";
// INPUT_PLACEHOLDER
const char index_html8[] PROGMEM = R"rawliteral(" maxlength=")rawliteral";
// INPUT_LENGHT
const char index_html9[] PROGMEM = R"rawliteral(">
                                    </div>
                                    <div id="error-code-message"
                                        class="error-message"></div>
                                </div>

                                <div class="container">
                                    <button class="btn-process" id="submit-btn"
                                        type="submit" form="network_form">
                                        Connect
                                        <span id="connecting-ring"
                                            style="display: none;"></span>
                                    </button>
                                    <div id="error-submit-message"
                                        class="error-message"></div>
                                </div>

                            </form>
                        </div>

                        <footer id="footer">
                            <p class="copyright" style="opacity:0.5;"><a
                                    role="link" style="color:inherit;"
                                    id="factorylink"
                                    href="javascript:factoryReset()">Factory
                                    Reset</a></p>
                            <p class="copyright" style="opacity: 0.5;">)rawliteral";
// FOOTER_INFO
static constexpr const char index_html10[] PROGMEM = R"rawliteral(</p>
                        </footer>

                        <script>
var table = document.getElementById("table-body");
var form = document.getElementById('network_form');
var code_listener = document.getElementById('code');
var ssid_listener = document.getElementById('ssid');
var password_listener = document.getElementById('password');
var isHidden = false;
var isAuth = false;
form.addEventListener('submit', submitForm);
code_listener.addEventListener('input', updateValue);
ssid_listener.addEventListener('input', updateValue);
password_listener.addEventListener('input', updateValue);)rawliteral";
// INPUT_INVALID_LENGHT
// INPUT_NOT_VALID
// CONNECTION_SUCCESSFUL
// RESET_CONFIRMATION_TEXT
static constexpr const char index_html11[] PROGMEM = R"rawliteral(
window.addEventListener("load", (event) => {
  loadSSID();
});

function updateValue(e) {
  showError(e.target.id,"",false);
};

function isRadioChecked(){
  var radios = document.getElementsByTagName('input');
  var value;
  for (var i = 0; i < radios.length; i++) {
      if (radios[i].type === 'radio' && radios[i].checked) {
        return true;  
      }
  }
  return false;
};

function showError(name, messsage, state){
  // names = code,password,ssid,submit
  let block;
  (name.toLowerCase() === "submit") ? block = null : block = document.getElementById(`error-${name}-input`);
  let message_elem = document.getElementById(`error-${name}-message`);
  if (state){
    if (block != null){block.classList.add("error");}
    message_elem.textContent = messsage;
  }
  else{
    if (block != null){block.classList.remove("error");}
    message_elem.textContent = "";
  }
};

function resetErrors(){
  let input_fields = ["submit","code","password","ssid"];
  for (var i = 0; i < ["submit","code","password","ssid"].length; i++) {
      showError(input_fields[i],"",false);
  }
};

function connectingState(state){
  let ring = document.getElementById("connecting-ring");
  let submitBtn = document.getElementById("submit-btn");
  let buttonTxt;
  let ring_visi = (state) ? "" : "none";
  (state) ? buttonTxt = "Connecting": buttonTxt = "Connect";
  submitBtn.innerHTML =`${buttonTxt}<span id="connecting-ring" style="display:${ring_visi};" ></span>`;
  (state) ? submitBtn.disabled = true : submitBtn.disabled = false;
};

function submitForm(event){
  event.preventDefault();

  valid = true;
  if (!isRadioChecked()){
    showError("submit","Select Network",true);
    return;
  }
  if (isCodeVisible())rawliteral";
// && code_listener.value.length != + INPUT_LENGHT or "" If INPUT_LENGHT = 0
static constexpr const char index_html12[] PROGMEM = R"rawliteral(){
    showError("code",invalid_code_lenght,true);
    valid = false;
  }
  if(isSsidVisible()&&ssid_listener.value.length===0){
    showError("ssid","SSID is required",true);
    valid = false;
  }
  if(isPasswordVisible()&&!isHidden&&password_listener.value.length===0){
    showError("password","Password is required",true);
    valid = false;
  }
  if (!valid){
    return;
  }
  resetErrors();
  var formData = new FormData(event.target);
  connectingState(true);
  disableForm(true);
  formData.delete("hidden_ssid");
  if (isHidden){
    formData.set("ssid", ssid_listener.value);
  }
  else{
    if (!isAuth){formData.set("password","");}
  }
  if (!isCodeVisible()){formData.delete("code");}
  if (formData.get("password").length === 0 ){
    formData.delete("password");
  }

  let url = "/configure";
  var payload = {};
  formData.forEach((value, key) => payload[key] = value);
  var req = new XMLHttpRequest();
  req.open('POST', url, true);
  req.setRequestHeader("Content-Type", "application/json");
  req.onload = function() {
    disableForm(false);
    connectingState(false);
    resetErrors()
    if (this.readyState == 4 && this.status == 200) {
      var jsonResponse = JSON.parse(this.responseText);
      if (jsonResponse.success === true){
        successPage(jsonResponse.ssid)
      }
      else{
        if(jsonResponse.reason === "code"){
          showError("submit",invalid_code, true);
        }
        else{
          showError("submit","Couldn't connect to "+ jsonResponse.ssid, true);
        }
      }
    }
    else{
      showError("submit","Error on connection request",true);
    }
  };
  req.onerror  = function() {
    console.log("Error on connection request..");
    showError("submit","Error on connection request",true);
    disableForm(false);
    connectingState(false);
  };
  req.send(JSON.stringify(payload));
};

function successPage(ssid_text){
  let card = document.getElementById('main-card');
  card.innerHTML = "";
  card.innerHTML = `
  <div class="wrapper">
     <svg class="checkmark" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 52 52"> <circle class="checkmark__circle" cx="26" cy="26" r="25" fill="none"/> <path class="checkmark__check" fill="none" d="M14.1 27.2l7.1 7.2 16.7-16.8"/></svg>
  </div>
  <div class="container" style="padding: 1rem;">
    <h2 style="color:#7ac142;word-break: break-word;">Success</h2>
    <p style="color:#7ac142;word-break: break-word;font-size:1.2rem;margin-bottom: 0.5rem;">Succesfully connected to</p>
    <p style="color:#7ac142;word-break: break-word;margin-top: 0rem;">${ssid_text}</p>
    <p style="opacity: 0.5;">${connection_successful_text}</p>
    <p style="opacity: 0.5;">You can close the window</p>
  </div>
  `
};

function onRadio(element){
  resetErrors();
  if (element.id === "hiddennetwork_radio"){
    showssidField(true);
    showpasswordField(true);
    isHidden = true;
    isAuth = false;
  }
  else{
    isHidden = false;
    showssidField(false);
    if (element.dataset.auth > 0){
      showpasswordField(true);
      isAuth = true;
    }
    else{
      showpasswordField(false);
      isAuth = false;
    }
  }
};

// Disable form
function disableForm(state){
  var allElements = form.elements;
  for (var i = 0, l = allElements.length; i < l; ++i) {
        allElements[i].disabled=state;
  }
  disableLinks(state);
};

// Toogle refresh spinner
function refreshSpin(state){
  var element = document.getElementById("refresh-icon");
  if (state){
    element.classList.add("icn-spinner");
  }
  else{
    element.classList.remove("icn-spinner");
  }
};

// Load new SSIDS
function loadSSID() {
  disableForm(true);
  refreshSpin(true);
  let url = "/update";
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function() {
    resetErrors();
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("table-body").innerHTML = '';
      var jsonResponse = JSON.parse(this.responseText);

      var code = jsonResponse.show_code;
      var networks = jsonResponse.network;

      for (let i = 0; i < networks.length; i++) {
        const network = networks[i];
        addTableRow(network.ssid, network.authmode, network.rssi);
      }

      showcodeField(code);
    }
    else{
      showError("submit","Error on Refresh",true);
    }
    refreshSpin(false);
    disableForm(false);
  };
  req.onerror  = function() {
    console.log("Error on Refresh..");
    resetErrors();
    showError("submit","Error on Refresh",true);
    refreshSpin(false);
    disableForm(false);
  };
  req.send(null);
};

// Add table row
function addTableRow(ssid,authmode,rssi){
  let locked;
  if (authmode > 0){
    locked = 1;
  }
  else{
    locked = 0;
  }
  let icon = svgs["" + rssi + locked];
  table.innerHTML += `
  <tr>
      <td class="radiossid">
        <input type="radio" name="ssid" value="${ssid}" data-auth="${authmode}" onclick="onRadio(this)">
      </td>
      <td>${ssid}</td>
      <td class="signal">
        <svg xmlns="http://www.w3.org/2000/svg" style="vertical-align: -0.125em;" width="1em" height="1em" preserveAspectRatio="xMidYMid meet" viewBox="0 0 24 24"><path fill="var(--font-color)" ${icon}
      </td>
  </tr>
  `
};
    
// Network Icons
var svgs = {
  "10":`d="m6.67 14.86l3.77 4.7c.8 1 2.32 1 3.12 0l3.78-4.7C17.06 14.65 15.03 13 12 13s-5.06 1.65-5.33 1.86z"/><path fill="var(--font-color)" fill-opacity=".3" d="M23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0L23.64 7z"/></svg>`,
  "11":`fill-opacity=".3" d="M15.5 14.5c0-2.8 2.2-5 5-5c.36 0 .71.04 1.05.11L23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0l1.94-2.42V14.5z"/><path fill="var(--font-color)" d="M15.5 14.5c0-.23.04-.46.07-.68c-.92-.43-2.14-.82-3.57-.82c-3 0-5.1 1.7-5.3 1.9l3.74 4.66c.8 1 2.32 1 3.12 0l1.94-2.42V14.5zM23 16v-1.5c0-1.4-1.1-2.5-2.5-2.5S18 13.1 18 14.5V16c-.5 0-1 .5-1 1v4c0 .5.5 1 1 1h5c.5 0 1-.5 1-1v-4c0-.5-.5-1-1-1zm-1 0h-3v-1.5c0-.8.7-1.5 1.5-1.5s1.5.7 1.5 1.5V16z"/></svg>`,
  "20":`fill-opacity=".3" d="M23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0L23.64 7z"/><path fill="var(--font-color)" d="m4.79 12.52l5.65 7.04c.8 1 2.32 1 3.12 0l5.65-7.05C18.85 12.24 16.1 10 12 10s-6.85 2.24-7.21 2.52z"/></svg>`,
  "21":`fill-opacity=".3" d="M15.5 14.5c0-2.8 2.2-5 5-5c.36 0 .71.04 1.05.11L23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0l1.94-2.42V14.5z"/><path fill="var(--font-color)" d="M15.5 14.5c0-1.34.51-2.53 1.34-3.42C15.62 10.51 13.98 10 12 10c-4.1 0-6.8 2.2-7.2 2.5l5.64 7.05c.8 1 2.32 1 3.12 0l1.94-2.42V14.5zM23 16v-1.5c0-1.4-1.1-2.5-2.5-2.5S18 13.1 18 14.5V16c-.5 0-1 .5-1 1v4c0 .5.5 1 1 1h5c.5 0 1-.5 1-1v-4c0-.5-.5-1-1-1zm-1 0h-3v-1.5c0-.8.7-1.5 1.5-1.5s1.5.7 1.5 1.5V16z"/></svg>`,
  "30":`fill-opacity=".3" d="M23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0L23.64 7z"/><path fill="var(--font-color)" d="m3.53 10.95l6.91 8.61c.8 1 2.32 1 3.12 0l6.91-8.61C20.04 10.62 16.81 8 12 8s-8.04 2.62-8.47 2.95z"/></svg>`,
  "31":`fill-opacity=".3" d="M15.5 14.5c0-2.8 2.2-5 5-5c.36 0 .71.04 1.05.11L23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0l1.94-2.42V14.5z"/><path fill="var(--font-color)" d="M15.5 14.5a4.92 4.92 0 0 1 3.27-4.68C17.29 8.98 14.94 8 12 8c-4.81 0-8.04 2.62-8.47 2.95l6.91 8.61c.8 1 2.32 1 3.12 0l1.94-2.42V14.5zM23 16v-1.5c0-1.4-1.1-2.5-2.5-2.5S18 13.1 18 14.5V16c-.5 0-1 .5-1 1v4c0 .5.5 1 1 1h5c.5 0 1-.5 1-1v-4c0-.5-.5-1-1-1zm-1 0h-3v-1.5c0-.8.7-1.5 1.5-1.5s1.5.7 1.5 1.5V16z"/></svg>`,
  "40":`d="M23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l10.08 12.56c.8 1 2.32 1 3.12 0L23.64 7z"/></svg>`,
  "41":`d="M23.21 8.24C20.22 5.6 16.3 4 12 4S3.78 5.6.79 8.24C.35 8.63.32 9.3.73 9.71l5.62 5.63l4.94 4.95c.39.39 1.02.39 1.42 0l2.34-2.34V15c0-.45.09-.88.23-1.29c.54-1.57 2.01-2.71 3.77-2.71h2.94l1.29-1.29c.4-.41.37-1.08-.07-1.47z"/><path fill="var(--font-color)" d="M22 16v-1c0-1.1-.9-2-2-2s-2 .9-2 2v1c-.55 0-1 .45-1 1v3c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3c0-.55-.45-1-1-1zm-1 0h-2v-1c0-.55.45-1 1-1s1 .45 1 1v1z"/></svg>`,
};

// Password Toogle
var eye_open = `M21.92 11.6C19.9 6.91 16.1 4 12 4s-7.9 2.91-9.92 7.6a1 1 0 0 0 0 .8C4.1 17.09 7.9 20 12 20s7.9-2.91 9.92-7.6a1 1 0 0 0 0-.8ZM12 18c-3.17 0-6.17-2.29-7.9-6C5.83 8.29 8.83 6 12 6s6.17 2.29 7.9 6c-1.73 3.71-4.73 6-7.9 6Zm0-10a4 4 0 1 0 4 4a4 4 0 0 0-4-4Zm0 6a2 2 0 1 1 2-2a2 2 0 0 1-2 2Z`
var eye_close = `M10.94 6.08A6.93 6.93 0 0 1 12 6c3.18 0 6.17 2.29 7.91 6a15.23 15.23 0 0 1-.9 1.64a1 1 0 0 0-.16.55a1 1 0 0 0 1.86.5a15.77 15.77 0 0 0 1.21-2.3a1 1 0 0 0 0-.79C19.9 6.91 16.1 4 12 4a7.77 7.77 0 0 0-1.4.12a1 1 0 1 0 .34 2ZM3.71 2.29a1 1 0 0 0-1.42 1.42l3.1 3.09a14.62 14.62 0 0 0-3.31 4.8a1 1 0 0 0 0 .8C4.1 17.09 7.9 20 12 20a9.26 9.26 0 0 0 5.05-1.54l3.24 3.25a1 1 0 0 0 1.42 0a1 1 0 0 0 0-1.42Zm6.36 9.19l2.45 2.45A1.81 1.81 0 0 1 12 14a2 2 0 0 1-2-2a1.81 1.81 0 0 1 .07-.52ZM12 18c-3.18 0-6.17-2.29-7.9-6a12.09 12.09 0 0 1 2.7-3.79L8.57 10A4 4 0 0 0 14 15.43L15.59 17A7.24 7.24 0 0 1 12 18Z`

function tooglePassShow(){
  let input = document.getElementById('password');
  if(input.type ==="password"){
      input.type = "text";
      document.getElementById('eye_icon').setAttribute("d",eye_open);
    }else{
      input.type = "password";
      document.getElementById('eye_icon').setAttribute("d",eye_close);
    }
  };

function showssidField(state){
  (state === true) ? document.getElementById('hiddenNetwork').style.display = '' : document.getElementById('hiddenNetwork').style.display = 'none';
};

function showpasswordField(state){
  (state === true) ? document.getElementById('hiddenPassword').style.display = '' : document.getElementById('hiddenPassword').style.display = 'none';
};

function showcodeField(state){
  (state === true) ? document.getElementById('codeinputBlock').style.display = '' : document.getElementById('codeinputBlock').style.display = 'none';
};

function isCodeVisible(){
  return (document.getElementById("codeinputBlock").style.display === "") ? true : false;
};
function isPasswordVisible(){
  return (document.getElementById("hiddenPassword").style.display === "") ? true : false;
};
function isSsidVisible(){
  return (document.getElementById("hiddenNetwork").style.display === "") ? true : false;
};

function disableLinks(state){
  let linkstate
  (state) ? linkstate = "none" : linkstate = "";
  document.getElementById("factorylink").style.pointerEvents = linkstate;
};

function resetAns(state){
  disableLinks(true)
  document.getElementById("resetYes").disabled = true;
  document.getElementById("resetNo").disabled = true;
  if (state){
    let url = "/factoryreset";
    var req = new XMLHttpRequest();
    req.open('POST', url, true);
    req.setRequestHeader("Content-Type", "application/json");
    req.onload = function() {
      document.location.href="/";
    };
    req.onerror  = function() {
      console.log("Error on reset request..");
      document.location.href="/";
    };
    req.send(JSON.stringify({"reset": true}));
  }
  else{
    document.location.href="/";
  }
};

function factoryReset(){
  let card = document.getElementById('main-card');
  card.innerHTML = "";
  card.innerHTML = `
  <div class="container" style="padding: 1rem;">
    <h3 style="padding-top: 1rem;margin: 0rem;">Are you sure?</h3>
    <p style="word-wrap: break-word;">${reset_confirmation_text}</p>
    <p style="word-wrap: break-word;">This action cannot be reversed.</p>
    <div>
      <button class="btn-process" id="resetYes" style="margin: 0.5rem;" onclick="resetAns(false);">No</button>
      <button class="btn-process" id="resetNo" style="margin: 0.5rem;" onclick="resetAns(true);">Yes</button>
    </div>
  </div>
  `
};
</script>
                    </body>
                </html>)rawliteral";

} // namespace WiFiProvisioner

#endif // PROVISION_HTML_H