/*
 * The webserver will attempt to connect to the primary ssid 5 times, with a delay
 * of 1 second between each attempt. If it fails to connect, it will retry
 * using the secondary ssid. The reasoning behind this: If I am at home, I want
 * it to connect to my home network. If I'm elsewhere, I want it to connect
 * to the hotspot on my phone. If it fails to connect to either, it will default
 * to its previously saved settings.
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your primary network credentials
const char* ssid = "PrimarySSID";
const char* password = "PrimaryPassword";

// Replace with your secondary network credentials
const char* ssid2 = "SecondarySSID";
const char* password2 = "SecondaryPassword";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Web server html
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 VU Meter</title>
  <style>
    html {font-family: Arial; display: inline-block}
    h2 {font-size: 2.3rem; text-align: center}
    p {font-size: 1.9rem;}
  table {width: 100%%}
  button {width: 49%%; height: 50px; font-size: 0.9rem;}
    body {max-width: 500px; margin:0px auto; padding: 0px 7px;}
    .slider { -webkit-appearance: none; margin: 14px 0; width: 100%%; height: 15px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 25px; height: 25px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 25px; height: 25px; background: #003249; cursor: pointer; }

  .labelCol {width: 80px}
  .valCol {width: 40px; padding-left: 14px}
  </style>
</head>
<body>
  <h2>ESP32 VU Meter</h2>
  <button type="button" id="nextBtn" onclick="sendData('n',1)">Next pattern</button>
  <button type="button" id="autoBtn" onclick="sendData('a',1)">Auto Change Pattern</button>
  </br></br>
  <label id="labelAutoChangeTime" for="displayTime">Seconds to show each pattern on auto </label>
  <input id="displayTime" type="number" min="1" max="65535" onchange="sendData('t',this.value)" value="%DISPLAYTIME%">
  </br></br>
  <table border="0">
  <tr>
    <td class="labelCol"><label id="labelBrightness" for="brightnessSlider">Brightness</label></td>
    <td><input type="range" id="brightnessSlider" onchange="sendData('b',this.value)" min="0" max="255" value="%BRIGHTNESSVALUE%" step="1" ></td>
    <td class="valCol"><span id="brightnessValue">%BRIGHTNESSVALUE%</span></td>
  </tr><tr>
    <td class="sliderCol"><label id="labelGain" for="gainSlider">Gain</label></td>
    <td><input type="range" id="gainSlider" onchange="sendData('g',this.value)" min="0" max="30" value="%GAINVALUE%" step="1" ></td>
    <td class="valCol"><span id="gainValue">%GAINVALUE%</span></td>
  </tr><tr>
    <td class="labelRow"><label id="labelSquelch" for="squelchSlider">Squelch</label></td>
    <td><input type="range" id="squelchSlider" onchange="sendData('s',this.value)" min="0" max="30" value="%SQUELCHVALUE%" step="1" ></td>
    <td class="valCol"><span id="squelchValue">%SQUELCHVALUE%</span></td>
  </tr></table><table>
  <tr><td>Band Adjust : </td>
  <td>#1 <input  type="range" size=4 id="band01" onchange="sendData('j01',this.value)" min="0" max="5" value="%j01%" step="1" orient="vertical"></td>
  <td>#2 <input  type="range" size=4 id="band02" onchange="sendData('j02',this.value)" min="0" max="5" value="%j02%" step="1" orient="vertical"></td>
  <td>#3 <input  type="range" size=4 id="band03" onchange="sendData('j03',this.value)" min="0" max="5" value="%j03%" step="1" orient="vertical"></td>
  <td>#4 <input  type="range" size=4 id="band04" onchange="sendData('j04',this.value)" min="0" max="5" value="%j04%" step="1" orient="vertical"></td>
  <td>#5 <input  type="range" size=4 id="band05" onchange="sendData('j05',this.value)" min="0" max="5" value="%j05%" step="1" orient="vertical"></td>
  <td>#6 <input  type="range" size=4 id="band06" onchange="sendData('j06',this.value)" min="0" max="5" value="%j06%" step="1" orient="vertical"></td>
  <td>#7 <input  type="range" size=4 id="band07" onchange="sendData('j07',this.value)" min="0" max="5" value="%j07%" step="1" orient="vertical"></td>
  <td>#8 <input  type="range" size=4 id="band08" onchange="sendData('j08',this.value)" min="0" max="20" value="%j08%" step="1" orient="vertical"></td>
  <td>#9 <input  type="range" size=4 id="band09" onchange="sendData('j09',this.value)" min="0" max="20" value="%j09%" step="1" orient="vertical"></td>
  <td>#10 <input type="range" size=4 id="band10" onchange="sendData('j10',this.value)" min="0" max="20" value="%j10%" step="1" orient="vertical"></td>
  <td>#11 <input type="range" size=4 id="band11" onchange="sendData('j11',this.value)" min="0" max="20" value="%j11%" step="1" orient="vertical"></td>
  <td>#12 <input type="range" size=4 id="band12" onchange="sendData('j12',this.value)" min="0" max="20" value="%j12%" step="1" orient="vertical"></td>
  <td>#13 <input type="range" size=4 id="band13" onchange="sendData('j13',this.value)" min="0" max="30" value="%j13%" step="1" orient="vertical"></td>
  <td>#14 <input type="range" size=4 id="band14" onchange="sendData('j14',this.value)" min="0" max="40" value="%j14%" step="1" orient="vertical"></td>
  <td>#15 <input type="range" size=4 id="band15" onchange="sendData('j15',this.value)" min="0" max="50" value="%j15%" step="1" orient="vertical"></td>
  <td>#16 <input type="range" size=4 id="band16" onchange="sendData('j16',this.value)" min="0" max="50" value="%j16%" step="1" orient="vertical"></td>
  
  </tr>

  </table>

<script>
 var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  
  function onOpen(event) {
    console.log('Connection opened');
  }
  
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  
  function onMessage(event) {
    var dataType = event.data.charAt(0);
    var dataValue = event.data.substring(1);
    switch (dataType){
    case 't':
      document.getElementById('displayTime').value = dataValue;
      break;
    case 'b':
      document.getElementById('brightnessValue').innerHTML = dataValue;
      document.getElementById('brightnessSlider').value = dataValue;
      break;
    case 'g':
      document.getElementById('gainValue').innerHTML = dataValue;
      document.getElementById('gainSlider').value = dataValue;
      break;
    case 's':
      document.getElementById('squelchValue').innerHTML = dataValue;
      document.getElementById('squelchSlider').value = dataValue;
      break;
    case 'a':
      if (dataValue == '1') document.getElementById('autoBtn').style.backgroundColor = '#baffb3';
      else document.getElementById('autoBtn').style.backgroundColor = '';
      break;
    case 'j':
      dataValue = event.data.substring(3);
      var band = event.data.substring(1,3);
      if (dataValue > 0 && dataValue < 50 && band <= 16) {
        document.getElementById('band'+band).value = dataValue;
      }
    }
  }
  
  function onLoad(event) {
    initWebSocket();
  }

  function sendData(type, val) {
    console.log(type+val);
    websocket.send(type+val);
  }
</script>
</body>
</html>
)rawliteral";

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);
    String message = String((char*)data);
    char dataType = message.charAt(0);
    String dataValue = message.substring(1);

    switch (dataType) {
      case 't':
        displayTime = dataValue.toInt();
        ws.textAll(message);
      case 'b':
        brightness = dataValue.toInt();
        ws.textAll(message);
        break;
      case 'g':
        gain = dataValue.toInt();
        ws.textAll(message);
        break;
      case 's':
        squelch = dataValue.toInt();
        ws.textAll(message);
        break;
      case 'n':
        pattern = (pattern + 1) % 6;
        break;
      case 'a':
        autoChangePatterns = !autoChangePatterns;
        if (autoChangePatterns) ws.textAll("a1");
        else ws.textAll("a0");
        break;
      case 'j':   // j for adjust
      dataValue = message.substring(3);
      String band = message.substring(1,3);
      if (dataValue.toInt() > 0 && dataValue.toInt() < 50 && band.toInt() <= 16) {
        Serial.printf("setting pink %ld to %ld",band.toInt()-1, dataValue.toInt());
        fftResultPink[band.toInt()-1] = dataValue.toInt();
      }
        
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  if(var == "DISPLAYTIME"){
    return String(displayTime);
  }
  if(var == "BRIGHTNESSVALUE"){
    return String(brightness);
  }
  if(var == "GAINVALUE"){
    return String(gain);
  }
  if(var == "SQUELCHVALUE"){
    return String(squelch);
  }
  if(var.indexOf('j' == 0)){
    int band = var.substring(1,3).toInt();
    return String(fftResultPink[band-1]); // 1 based to 0 based
  }
  Serial.println("done socket init");
  return "";
  
}

/// connect to the network. return true if success, false if not.
bool setupWiFi() {
  Serial.println("go for setupWifi");
  WiFi.mode(WIFI_STA);
  // wait a bit for auto stored connection (esp saves previous succesfull connections)
  while (WiFi.status() != WL_CONNECTED) {
    static int i = 0;
    delay(500);
    Serial.print(".");
    if (i++ > 10) break;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("connected");
    return true;
  } else {
    Serial.println("go for smartconfig");
    WiFi.beginSmartConfig();

    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (WiFi.status() != WL_CONNECTED) {
      static int i = 0;
      delay(500);
      Serial.print(".");
      if (i++ > 20) break;   // bail out after 10 seconds
    }
  }
  if (WiFi.status() == WL_CONNECTED) {

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  return true;


  } else {   // try hard coded 

  uint8_t connectionAttempts = 0;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay (1000);
    Serial.println("Connecting to primary WiFi ...");
    connectionAttempts++;
    if (connectionAttempts > 5) {
      
      WiFi.begin(ssid2, password2);
      break;    
    }
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay (1000);
    Serial.println("Connecting to secondary WiFi ...");
    connectionAttempts++;
    if (connectionAttempts > 10) return false;    
  }

  // Print ESP Local IP Address
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  return true;

  }
  return false;
}


void setupWebServer() {
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}
