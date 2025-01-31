//*****************************************//
//** Dry Contact Wi-Fi to Home Assistant **//
//**             Versione 1.0            **//
//**             GALAXI SOLAR            **//
//**              31/01/2005             **//
//*****************************************//
#include <LittleFS.h>
#include <ArduinoJson.h> 
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWiFiManager.h>
#include <FS.h>

//variabili tensioni Alta, bassa, calibra
const char* PARAM_TENS_ALTA = "tens_alta";
const char* PARAM_TENS_BASSA = "tens_bassa";
const char* CALIBRA_TENSIONE = "calib_tens";
const char* TENSIONE_BATTERIA = "tens_batt";
const char* PARAM_TENS_CUTOFF = "tens_cutoff";

Adafruit_ADS1X15 adc;
const int pinRELE = D5;  // pin relè
#define TRIGGER_PIN D6 // pin trigger
#define pinLEDWIFI D7  // pin led Wifi

String tensione_alta;
String tensione_altaHA;
String tensione_bassa;
String tensione_bassaHA;
String tensione_cutoff;
String tensione_cutoffHA;
String calibrazione;
String calibHA;
int16_t adc0;
float volts0;

String inputMessage;
String inputMessage1;
String inputMessage2 = "0";
String inputMessage3;
String payload;
String tensioni_L_H;
String stato_rele;
String switch_HA;
String pinRele_HW;
String stato_pin_rele;

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

//MQTT in Ascolto
#define DRY_RELE "dry_contact/switchHA"
#define DRY_V_ALTA "dry_contact/tensione_altaHA"
#define DRY_V_BASSA "dry_contact/tensione_bassaHA"
#define DRY_CALIB "dry_contact/calibHA"
#define DRY_CUT_OFF "dry_contact/tensione_cut_offHA"

char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_user[34];
char mqtt_passw[34];
char mqtt_id[34] = "ESP_Dry_Contact";

const char* LWT_TOPIC = "dry_contact/stato";
const char* LWT_MESSAGE = "offline";

//gets called when WiFiManager enters configuration mode
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

AsyncWebServer server(80);
DNSServer dns;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Dry Contact Server</title>
 
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:-15px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 58px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #cc3300; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 42px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #33cc33}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    .label {
      color: white;
      padding: 8px;
      font-family: Arial;
}
.tensione_batt {
  max-width: 500px;
  background-color:rgb(255, 236, 139);
  color: rgb(104,34,139);
  margin: auto;
  border-radius: 7px;
  border: 2px solid #2F2F2F;
} 

.config_tens_alta {
  height: 18px;
  width: 50px;
  line-height: 40px; 
  background-color:rgb(0, 255, 0);
  color: rgb(104,34,139);
  margin-top: 0px !important; 
  border-radius: 7px;
  border: 1px solid #2F2F2F;
  font-size: 14px;
} 

.config_tens_bassa {
  height: 18px;
  width: 50px;
  line-height: 40px; 
  background-color:rgb(255, 0, 0);
  color: rgb(104,34,139);
  margin-top: 0px !important; 
  border-radius: 7px;
  border: 1px solid #2F2F2F;
  font-size: 14px;
} 

.config_calibrazione {
  height: 18px;
  width: 50px;
  line-height: 40px; 
  background-color:rgb(64, 224, 208);
  color: rgb(104,34,139);
  margin-top: 0px !important; 
  border-radius: 7px;
  border: 1px solid #2F2F2F;
  font-size: 14px;
} 

.config_tens_cutoff {
  height: 18px;
  width: 50px;
  line-height: 40px; 
  background-color:rgb(238, 0, 238);
  color: rgb(104,34,139);
  margin-top: 0px !important; 
  border-radius: 7px;
  border: 1px solid #2F2F2F;
  font-size: 14px;
} 

.rele_stato {
  font-size: 14px;
  line-height: 20px; 
  color: black;
  width: 250px;
  margin: auto;
  margin-bottom:20px;
  margin-top:10px;
}
body {
  background-color: lightblue; /* For browsers that do not support gradients */
  background-image: linear-gradient(to bottom right, lightblue, yellow);
}
h4 {
  background-color:rgba(255, 99, 71, 0.4);
  font-size: 14px;
  border-style: ridge;
  width: 250px;
  margin: auto;
  margin-bottom:20px;
  margin-top:10px;
}

</style>
</head>

<div id="main">
 
  <h1>Dry Contact</h1>
  <form action="/get" target="hidden-form"> 
  <button class="button" onclick="submitMessageHelp()">Help</button>
  </form>
  <h4>Tensione Batteria</h4>
  
  <h1><span class="label tensione_batt"id="tensionebatteria"/span></h1>
  
  %BUTTONPLACEHOLDER%
  <div><span class="label rele_stato"id="statorele"/span></div>
  
  <h4>Parametri di configurazione</h4>
  
  <div><span class="label config_tens_alta" id="tensionealta"/span></div>
  <form action="/get" target="hidden-form">  
  <input type="text" id="tensione_alta" name="tens_alta" value="" maxlength="5" size="5">
  <button class="button" onclick="submitMessage()">SALVA</button>
  <p style="margin-bottom:-47px;"><h6><font color="#800000">Tensione Alta (es. 52.95)</font></h6>
  </form>
  
  <div><span class="label config_tens_bassa" id="tensionebassa"/span></div>
  <form action="/get" target="hidden-form"> 
  <input type="text" id="tensione_bassa" name="tens_bassa" value="" maxlength="5" size="5">
  <button class="button" onclick="submitMessage()">SALVA</button>
  <p style="margin-bottom:-47px;"><h6><font color="#800000">Tensione Bassa (es. 50.90)</font></h6>
  </form>
  
  <div><span class="label config_calibrazione" id="valcalibrazione"/span></div>
  <form action="/get" target="hidden-form"> 
  <input type="text" id="calibrazione" name="calib_tens" value="" maxlength="5" size="5">
  <button class="button" onclick="submitMessage()">SALVA</button>
  <p style="margin-bottom:-47px;"><h6><font color="#800000">Calibra Lettura Tensione (default 0) - (100 = 0.1V)</font></h6>
  </form>
 
  <div><span class="label config_tens_cutoff" id="tensionecutoff"/span></div> 
  <form action="/get" target="hidden-form"> 
  <input type="text" id="tensione_cutoff" name="tens_cutoff" value="" maxlength="5" size="5">
  <button class="button" onclick="submitMessage()">SALVA</button>
  <p style="margin-bottom:-47px;"><h6><font color="#800000">Tensione CutOff (default 49.60)</font></h6>
  </form>
   
 <h5><U>Versione 1.0 by Galaxi Solar</U></h5>
 <iframe style="display:none" name="hidden-form"></iframe>
</div>

<script>
//var statorele = 0;

setInterval(loadTensioneBatteria, 1000);
setInterval(statoRele, 1000);
setInterval(loadTensioneAlta, 1000);
setInterval(loadTensioneBassa, 1000);
setInterval(loadCalibrazione, 1000);
setInterval(loadTensioneCutoff, 1000);

function loadTensioneBatteria() {
  var tensione_batteria = 0; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tensionebatteria").innerHTML = this.responseText + " V";		    
    }
  };
  xhttp.open("GET", "tens_batt", true);
  xhttp.send();
}

function loadTensioneAlta() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {    
         document.getElementById("tensionealta").innerHTML = this.responseText + " V"; 
    }
  };
  xhttp.open("GET", "tens_alta", true);
  xhttp.send();
}

function loadTensioneBassa() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tensionebassa").innerHTML = this.responseText + " V";
    }
  };
  xhttp.open("GET", "tens_bassa", true);
  xhttp.send();
}

function loadCalibrazione() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("valcalibrazione").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "calib_tens", true);
  xhttp.send();
}

function loadTensioneCutoff() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tensionecutoff").innerHTML = this.responseText + " V";
    }
  };
  xhttp.open("GET", "tens_cutoff", true);
  xhttp.send();
}

function statoRele() { 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (statorele == 0) {
          document.getElementById("14").checked = false;
          document.getElementById("statorele").innerHTML = "Spento";
          statorele = 1; 
      } else {
           document.getElementById("14").checked = true;
           document.getElementById("statorele").innerHTML = "Acceso";
           statorele = 0; 
      }
      statorele = parseFloat(this.responseText);  
    }
  };
  xhttp.open("GET", "status_rele", true);
  xhttp.send();	
}
 
function toggleCheckbox(element) { 
  var xhttp = new XMLHttpRequest();
  if(element.checked){
    xhttp.open("GET", "/update?output="+element.id+"&state=1", true);
    document.getElementById("statorele").innerHTML = "Acceso";
    statorele = 1; 
    } else {
    xhttp.open("GET", "/update?output="+element.id+"&state=0", true);
    document.getElementById("statorele").innerHTML = "Spento";
    statorele = 0;
   } 
  xhttp.send(); 
   document.getElementById("statorele").innerHTML = statorele;
}

function submitMessage() {
      alert("Valore Salvato Correttamente");
      setTimeout(function(){ document.location.reload(false); }, 500);
    }
function submitMessageHelp() {
      alert("Parametri Configurazione: \n\nTENSIONE ALTA - Imposta la soglia del passaggio a Solare\nTENSIONE BASSA - Imposta la soglia del passaggio a Rete\nCALIBRA TENSIONE - Calibra la lettura della tensione della Batteria\nTENSIONE CUTOFF - Imposta la tensione di sicurezza dello sgancio\n");
	  alert("***********FUNZIONAMENTO***********\nQuando la tensione della batteria arriva alla soglia tensione alta attiva il relè, invece quando scende sotto la soglia della tensione bassa lo disattiva.\nQuando il relè è spento perchè non si è ancora raggiuna la soglia alta, è possibile attivarlo manuamente con il pulsante web. Una volta raggiunta la la soglia di riaggancio il dry contact disattiva  l'azione manuale e riattiva l'automatismo");
      setTimeout(function(){ document.location.reload(false); }, 500);
    }

</script>

</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Pagina Non Trovata");
}

String readFile(fs::FS &fs, const char * path){
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- file vuoto o apertura fallita");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Scrittura file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- apertura file fallita per la scrittura");
    return;
  }
  if(file.print(message)){
    Serial.println("- Scrittura OK");
  } else {
    Serial.println("- Scrittura fallita");
  }
  file.close();
  }
// Replaces placeholder with button section in your web page
String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Stato Rele'</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"14\" " + outputState(14) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  // Sostituisce il segnaposto sul web con i valori archiviati
  else if(var == "tens_bassa"){
    return readFile(LittleFS, "/tensione_bassa.txt");
  }
  else if(var == "tens_alta"){
    return readFile(LittleFS, "/tensione_alta.txt");
  }
  else if(var == "calib_tens"){
    return readFile(LittleFS, "/calib_tensione.txt");
  }
  else if(var == "tens_cutoff"){
    return readFile(LittleFS, "/tensione_cutoff.txt");
  }
  return String();
  }


String outputState(int output){
  if(digitalRead(pinRELE)){
    return "checked";
  }
  else {
    return "";
  }
}


WiFiClient espClient;
PubSubClient client(espClient);

// Oled dimensions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// INIT display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


void callback(char* topic, byte* payload, unsigned int length) {
  
 if (strcmp(topic,DRY_RELE)==0){
     switch_HA = "";
     for (int i = 0; i < length; i++) {
     switch_HA = switch_HA + (char)payload[i];
     //Converte il messaggio da byte a String
      }
  
   if (switch_HA == "On") {
      inputMessage2 = 1;
      } else {
      inputMessage2 = 0;
    }
  } 
  if (strcmp(topic,DRY_V_ALTA)==0){
     tensione_altaHA = "";
     for (int i = 0; i < length; i++) {
     tensione_altaHA = tensione_altaHA + (char)payload[i];
     //Converte il messaggio da byte a String
   } 
    tensione_alta = tensione_altaHA;
    writeFile(LittleFS, "/tensione_alta.txt", tensione_alta.c_str());
  }  
  if (strcmp(topic,DRY_V_BASSA)==0){
     tensione_bassaHA = "";
     for (int i = 0; i < length; i++) {
     tensione_bassaHA = tensione_bassaHA + (char)payload[i];
     //Converte il messaggio da byte a String
   } 
    tensione_bassa = tensione_bassaHA;
    writeFile(LittleFS, "/tensione_bassa.txt", tensione_bassa.c_str());
  }  
  if (strcmp(topic,DRY_CALIB)==0){
     calibHA = "";
     for (int i = 0; i < length; i++) {
     calibHA = calibHA + (char)payload[i];
     //Converte il messaggio da byte a String
   } 
    calibrazione = calibHA;
    writeFile(LittleFS, "/calib_tensione.txt", calibrazione.c_str());
  }  
    if (strcmp(topic,DRY_CUT_OFF)==0){
     tensione_cutoffHA = "";
     for (int i = 0; i < length; i++) {
     tensione_cutoffHA = tensione_cutoffHA + (char)payload[i];
     //Converte il messaggio da byte a String
   }  
    tensione_cutoff = tensione_cutoffHA;
    writeFile(LittleFS, "/tensione_cutoff.txt", tensione_cutoff.c_str());
  } 
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Connessione MQTT...");
    if (client.connect(mqtt_id, mqtt_user, mqtt_passw, LWT_TOPIC, 0, true, LWT_MESSAGE)) {
      Serial.println("Connesso");
      client.subscribe(DRY_RELE);
      client.subscribe(DRY_CUT_OFF);
      client.subscribe(DRY_V_ALTA);
      client.subscribe(DRY_V_BASSA);
      client.subscribe(DRY_CALIB);
    } else {
      Serial.print("error, rc=");
      Serial.print(client.state());
      Serial.println(" I'll try again in 5 seconds");
      
      lcdClear();
      lcdFontsmall();
      lcdPrintln(1,"MQTT non Connesso!!");
     
      delay(5000);
    }
  }
}



void setup() {
 // Aperttura Seriale
 Serial.begin(9600);

  u8g2.begin();
  lcdFontsmall();  
  lcdClear();
 
 //WiFiManager
 AsyncWiFiManager wifiManager(&server,&dns);
 
// Apertura FileSystem
if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

//------Legge i file da LITTLEFS--------  
if (LittleFS.exists("/config.json")) {
    Serial.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_passw, json["mqtt_passw"]);
          strcpy(mqtt_id, json["mqtt_id"]);
          } else {
          Serial.println("failed to load json config");
          }
        }
}
  
  
  File v_alta = LittleFS.open("/tensione_alta.txt", "r");
  if(!v_alta){
    Serial.println("Failed to open file for reading");
    return;
  } 
  while(v_alta.available()){
    tensione_alta = v_alta.readString();
  } 
  v_alta.close();

  File v_bassa = LittleFS.open("/tensione_bassa.txt", "r");
  if(!v_bassa){
    Serial.println("Failed to open file for reading");
    return;
  }
  while(v_bassa.available()){
    tensione_bassa = v_bassa.readString();
  }
  v_bassa.close();

  File v_calib = LittleFS.open("/calib_tensione.txt", "r");
  if(!v_calib){
    Serial.println("Failed to open file for reading");
    return;
  }
  while(v_calib.available()){
    calibrazione = v_calib.readString();
  }
  v_calib.close();

  File v_cutoff = LittleFS.open("/tensione_cutoff.txt", "r");
  if(!v_cutoff){
    Serial.println("Failed to open file for reading");
    return;
  }
  while(v_cutoff.available()){
   tensione_cutoff = v_cutoff.readString();
  }
  v_cutoff.close();

// Setta il Pin D5 RELE come uscita (OUTPUT)
  pinMode (pinRELE, OUTPUT);
  // Setta il Pin D7 RELE come uscita (OUTPUT)
  pinMode (pinLEDWIFI, OUTPUT);
// Setta il Pin D6 Trigger come uscita (INPUT)
  pinMode (TRIGGER_PIN, INPUT);
  // Messaggio di Benvenuto
  lcdClear();
  lcdFontbig();
  lcdPrintln(0,"DRY CONTACT");
  lcdPrintln(2,"Galaxi Solar"); 
  lcdPrintln(4,"  ver. 1.0");
  delay(3000);

 

  adc.setGain(GAIN_TWOTHIRDS);
  adc.begin();


  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
  } 
   
   // Pulsante premuto all'accensione entra nella configurazione
  chiamata_wifi_manager();
  
  // Start server mqtt
  client.setServer(mqtt_server, (int) atoi(mqtt_port));
  client.setCallback(callback);

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
      lcdClear();
      lcdFontsmall();   
      lcdPrintln(0," Connesso alla rete");
      lcdPrintln(1,WiFi.SSID());
      String localIP = WiFi.localIP().toString().c_str();
      lcdPrintln(3,"IP: " + localIP);   
      lcdPrintln(4,"server MQTT Connesso");
      delay(3000);
    }
  }  
 
// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());

    }
    request->send(200, "text/text", "OK");
  });
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      if (request->hasParam(PARAM_TENS_ALTA)) {
      inputMessage = request->getParam(PARAM_TENS_ALTA)->value();
      tensione_alta = inputMessage.c_str();
      client.publish("dry_contact/tensione_altaHA", tensione_alta.c_str(),true);
    }
    else if (request->hasParam(PARAM_TENS_BASSA)) {
      inputMessage = request->getParam(PARAM_TENS_BASSA)->value();
      tensione_bassa = inputMessage.c_str();
      client.publish("dry_contact/tensione_bassaHA", tensione_bassa.c_str(),true);
    }
    else if (request->hasParam(CALIBRA_TENSIONE)) {
      inputMessage = request->getParam(CALIBRA_TENSIONE)->value();
      calibrazione = inputMessage.c_str();
      client.publish("dry_contact/calibHA", calibrazione.c_str(),true);     
    }
    else if (request->hasParam(PARAM_TENS_CUTOFF)) {
      inputMessage = request->getParam(PARAM_TENS_CUTOFF)->value();
      tensione_cutoff = inputMessage.c_str();   
      tensione_cutoffHA = tensione_cutoff;
      client.publish("dry_contact/tensione_cut_offHA", tensione_cutoff.c_str(),true);     
    }
    else {
      inputMessage = "Non salvato";
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    request->send(200, "text/text", inputMessage);
  });
  server.on("/tens_batt", HTTP_GET, [] (AsyncWebServerRequest *request) {
      request->send(200, "text/text", payload.c_str());
  });
  server.on("/status_rele", HTTP_GET, [] (AsyncWebServerRequest *request) {
      request->send(200, "text/text", stato_pin_rele.c_str());
  });
  server.on("/tens_alta", HTTP_GET, [] (AsyncWebServerRequest *request) {     
      request->send(200, "text/text", tensione_alta.c_str());
  });
  server.on("/tens_bassa", HTTP_GET, [] (AsyncWebServerRequest *request) {     
      request->send(200, "text/text", tensione_bassa.c_str());
  });
  server.on("/calib_tens", HTTP_GET, [] (AsyncWebServerRequest *request) {     
      request->send(200, "text/text", calibrazione.c_str());
  });
  server.on("/tens_cutoff", HTTP_GET, [] (AsyncWebServerRequest *request) {     
      request->send(200, "text/text", tensione_cutoff.c_str());
  });
  server.onNotFound(notFound);

 // Start server
  server.begin();

 lcdClear();
 Serial.println("fine setup");
}

void loop() {
  chiamata_wifi_manager();
 
 //Riconnette al server mqtt
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite (pinLEDWIFI, HIGH);
    if (!client.connected()) {
      reconnect();
    }    
  } else { 
     digitalWrite (pinLEDWIFI, LOW); 
  }
 client.loop();

// Legge la tensione del ADC 
adc0 = adc.readADC_SingleEnded(0);
volts0 = (adc.computeVolts(adc0) * 46.96) + (calibrazione.toInt() * 0.001);
payload = String(volts0);

//--------Publica Parametri su MQTT--------------
  client.publish("dry_contact/batteria", payload.c_str());
  client.publish("dry_contact/rele", pinRele_HW.c_str());
  client.publish("dry_contact/tensione_bassa", tensione_bassa.c_str());
  client.publish("dry_contact/tensione_alta", tensione_alta.c_str());
  client.publish("dry_contact/tensione_cutt_off", tensione_cutoff.c_str());
  client.publish("dry_contact/calibrazione", calibrazione.c_str());
  client.publish(LWT_TOPIC, "online", true);

// ACCENDE e SPEGNE il RELE  
if (inputMessage2 == "0") {
   if (volts0 < tensione_bassa.toFloat()){
      digitalWrite (pinRELE, LOW);      
    } 
   if (volts0 > tensione_alta.toFloat()){
      digitalWrite (pinRELE, HIGH);
      inputMessage2 = "0";    
    }
}   
//------On / OFF relè manuale dal webserver------
if (inputMessage2 == "1") {      
      digitalWrite (pinRELE, HIGH);    
    }
  
//--------Reset manuale al raggiungimento della tensione alta-----
if (volts0 > tensione_alta.toFloat()){
      inputMessage2 = "0";
    }
//--------Intervento CUTOFF-----
if (volts0 < tensione_cutoff.toFloat()){
      inputMessage2 = "0";
      digitalWrite(pinRELE,LOW);   
    }

stato_pin_rele = digitalRead(pinRELE);

if (stato_pin_rele == "1") {
    pinRele_HW = "On";
} else {
    pinRele_HW = "Off";
}

// SCRIVE LA TENSIONE SU LCD 
  if(tensione_alta.length() == 4) {tensione_alta = tensione_alta + "0";}
  if(tensione_bassa.length() == 4) {tensione_bassa = tensione_bassa + "0";} 
  if(tensione_cutoff.length() == 4) {tensione_cutoff = tensione_cutoff + "0";} 
  tensioni_L_H = "H " + tensione_alta + " - L " + tensione_bassa;
  if (digitalRead(pinRELE) == HIGH) {
    stato_rele = "   ACCESO";
  } else {
    stato_rele = "   SPENTO";
  }
  drawScreen();
  
//--------------------------------  
delay(2000);

}


///// Stato pulsante chiamate Wifi Manager //////////
void chiamata_wifi_manager() {
  // is configuration portal requested?
  int statopulsante;
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
   statopulsante = 0;
   WiFi.disconnect();
  } else {
    statopulsante = 1;
    }
  
  if ( statopulsante == 0 ) {
  lcdClear();
  lcdFontsmall();   
  lcdPrintln(0," Connetti alla WiFi");
  lcdPrintln(1," ** DrycontactAP **");
  lcdPrintln(2,"  IP 192.168.4.1"); 
  lcdPrintln(3," setta la rete WiFi");
  lcdPrintln(4,"  e il server MQTT");

     AsyncWiFiManagerParameter custom_mqtt_server("server", "Server IP", mqtt_server, 40);
     AsyncWiFiManagerParameter custom_mqtt_port("port", "Server port", mqtt_port, 6);
     AsyncWiFiManagerParameter custom_mqtt_user("user", "User Name", mqtt_user, 32);
     AsyncWiFiManagerParameter custom_mqtt_passw("passw", "Password", mqtt_passw, 32);
     AsyncWiFiManagerParameter custom_mqtt_id("id", "id mqtt", mqtt_id, 32);

     AsyncWiFiManager wifiManager(&server,&dns);

     //set config save notify callback
     wifiManager.setSaveConfigCallback(saveConfigCallback);
     //add all your parameters here
     wifiManager.addParameter(&custom_mqtt_server);
     wifiManager.addParameter(&custom_mqtt_port);
     wifiManager.addParameter(&custom_mqtt_user);
     wifiManager.addParameter(&custom_mqtt_passw);
     wifiManager.addParameter(&custom_mqtt_id);
      if(!wifiManager.autoConnect("DryContactAP")) {
       Serial.println("connessione non riuscita nel tempo di timeout");
       lcdClear();
       delay(1000);
      }
      //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
        //read updated parameters
        strcpy(mqtt_server, custom_mqtt_server.getValue());
        strcpy(mqtt_port, custom_mqtt_port.getValue());
        strcpy(mqtt_user, custom_mqtt_user.getValue());
        strcpy(mqtt_passw, custom_mqtt_passw.getValue());
        strcpy(mqtt_id, custom_mqtt_id.getValue());

        //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_passw"] = mqtt_passw;
    json["mqtt_id"] = mqtt_id;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  }
}

//////////////////////IMPOSTAZIONI SCHERMO LCD
void lcdIconBattery() {
  u8g2.setFont(u8g2_font_battery24_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  }
void lcdFontbig() {
  u8g2.setFont(u8g2_font_t0_22b_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}
void lcdFontsmall() {
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}
void lcdPrepare() {
  u8g2.setFont(u8g2_font_sonicmania_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void lcdClear() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void lcdPrintln(int posy, String txt) {
  int py=posy*12;
  u8g2.drawStr(0, py, txt.c_str());
  u8g2.sendBuffer();
}

void drawScreen() {
  // 
  char buffer[128];
  int y=3;
  int x=35;
  u8g2.setColorIndex(0);
  u8g2.drawBox(30,0,60,23);

  lcdIconBattery();
  lcdPrintln(0," Batteria");
  lcdFontbig();
  sprintf(buffer,payload.c_str());  
  u8g2.drawStr(x, y, buffer);
  lcdPrintln(0,"         V");
  lcdFontsmall();
  x=8;
  y=38;  
  u8g2.drawStr(x, y, tensioni_L_H.c_str());
  x=8;
  y=53;  
  u8g2.drawStr(x, y, ("C-off " + tensione_cutoff).c_str());
  x=20;
  y=23;
  u8g2.drawStr(x, y, stato_rele.c_str());
  
  
  // OK print
  u8g2.sendBuffer();
}
