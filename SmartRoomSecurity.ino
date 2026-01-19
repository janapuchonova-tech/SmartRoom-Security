#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define PIRPIN 13
#define LDRPIN 1
#define LEDPIN 2
#define BUZZPIN 7

// ---------- DOMÁCA WiFi (musí mať internet) ----------
const char* ssid = "ESP32_TEST";   
const char* password = "Jp*850509";  

// ---------- Kolárovice ----------
const float LAT = 49.2530;
const float LON = 18.6760;

WebServer server(80);

bool alarmZapnuty = false;
int svetlo = 0;
bool pohyb = false;

// Počasie
float outdoorTemp = NAN;
unsigned long lastWeatherMs = 0;

void updateWeather() {
  // max raz za 60 sekúnd
  if (lastWeatherMs != 0 && (millis() - lastWeatherMs) < 60000) return;
  lastWeatherMs = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WEATHER] WiFi nie je pripojene.");
    return;
  }

  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(LAT, 4) +
               "&longitude=" + String(LON, 4) + "&current_weather=true";

  WiFiClientSecure client;
  client.setInsecure(); // zjednoduší HTTPS (na školský projekt OK)

  HTTPClient http;
  http.begin(client, url);

  int code = http.GET();
  Serial.print("[WEATHER] HTTP code: ");
  Serial.println(code);

  if (code == 200) {
    String payload = http.getString();

    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
      Serial.print("[WEATHER] JSON chyba: ");
      Serial.println(err.c_str());
    } else {
      outdoorTemp = doc["current_weather"]["temperature"] | NAN;
      Serial.print("[WEATHER] Teplota vonku: ");
      Serial.println(outdoorTemp);
    }
  } else {
    Serial.println("[WEATHER] Nepodarilo sa nacitat pocasie.");
  }

  http.end();
}

String webPage() {
  String html = R"rawliteral(
<!doctype html>
<html lang="sk">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartRoom Security</title>
<style>
body{
  font-family: Arial, sans-serif;
  background: linear-gradient(135deg,#0f2027,#203a43,#2c5364);
  color:white;
  text-align:center;
  margin:0;
  padding:20px;
}
h1{margin-bottom:5px}
.container{max-width:520px;margin:auto}
.card{
  background:rgba(255,255,255,0.12);
  border-radius:16px;
  padding:14px;
  margin:12px 0;
}
.value{font-size:24px;font-weight:bold}
.ok{color:#2ecc71}
.alert{color:#e74c3c}
button{
  width:100%;
  padding:14px;
  font-size:18px;
  border-radius:12px;
  border:none;
  margin-top:10px;
}
.on{background:#e74c3c;color:white}
.off{background:#2ecc71;color:white}
canvas{
  width:100%;
  height:160px;
  background:rgba(0,0,0,0.25);
  border-radius:12px;
}
.small{font-size:12px;opacity:0.85}
</style>
</head>

<body>
<div class="container">
<h1>🔐 SmartRoom</h1>
<p>Inteligentná bezpečnosť (Kolárovice)</p>

<div class="card">
  🕒 <span id="cas"></span><br>
  📅 <span id="datum"></span>
</div>

<div class="card">
  🌡️ Vonkajšia teplota
  <div class="value" id="vonku">načítavam...</div>
  <div class="small">Zdroj: Open-Meteo (cez ESP32)</div>
</div>

<div class="card">
  💡 Svetlo
  <div class="value" id="svetlo">--</div>
  <div class="small">ADC 0–4095</div>
</div>

<div class="card">
  📈 Graf svetla (posledná minúta)
  <canvas id="graf" width="480" height="160"></canvas>
  <div class="small">Nový bod každú 1 sekundu</div>
</div>

<div class="card">
  🚶 Pohyb
  <div class="value" id="pohyb">--</div>
</div>

<div class="card">
  🚨 Alarm
  <div class="value" id="alarm">--</div>
  <div class="value" id="stav"></div>
</div>

<a href="/alarmOn"><button class="on">ZAPNÚŤ ALARM</button></a>
<a href="/alarmOff"><button class="off">VYPNÚŤ ALARM</button></a>

</div>

<script>
// čas a dátum
function updateTime(){
  const d = new Date();
  document.getElementById('cas').textContent = d.toLocaleTimeString('sk-SK');
  document.getElementById('datum').textContent = d.toLocaleDateString('sk-SK');
}
setInterval(updateTime, 1000); updateTime();

// graf
const MAX_POINTS = 60;
let history = [];
const canvas = document.getElementById('graf');
const ctx = canvas.getContext('2d');

function drawGraph(){
  const h = canvas.height, w = canvas.width;
  ctx.clearRect(0,0,w,h);

  // mriežka
  ctx.globalAlpha = 0.35;
  ctx.beginPath();
  for(let i=1;i<5;i++){
    let y = (h/5)*i;
    ctx.moveTo(0,y); ctx.lineTo(w,y);
  }
  ctx.strokeStyle = "#ffffff";
  ctx.lineWidth = 1;
  ctx.stroke();
  ctx.globalAlpha = 1.0;

  if(history.length < 2) return;

  ctx.beginPath();
  for(let i=0;i<history.length;i++){
    const x = (w/(MAX_POINTS-1))*i;
    const v = history[i];
    const y = h - (v/4095.0)*h;
    if(i==0) ctx.moveTo(x,y);
    else ctx.lineTo(x,y);
  }
  ctx.strokeStyle = "#00e5ff";
  ctx.lineWidth = 2;
  ctx.stroke();
}

// stav
async function loadState(){
  const r = await fetch('/state');
  const j = await r.json();

  document.getElementById('svetlo').textContent = j.svetlo;
  document.getElementById('pohyb').textContent = j.pohyb ? "ÁNO" : "NIE";
  document.getElementById('alarm').textContent = j.alarm ? "ZAPNUTÝ" : "VYPNUTÝ";
  document.getElementById('stav').textContent = j.poplach ? "POPLACH!" : "OK";
  document.getElementById('stav').className = j.poplach ? "alert" : "ok";

  history.push(j.svetlo);
  if(history.length > MAX_POINTS) history.shift();
  drawGraph();
}
setInterval(loadState, 1000); loadState();

// počasie
async function loadWeather(){
  const r = await fetch('/weather');
  const j = await r.json();
  if(j.ok){
    document.getElementById('vonku').textContent = j.temp.toFixed(1) + " °C";
  } else {
    document.getElementById('vonku').textContent = "nedostupné";
  }
}
setInterval(loadWeather, 5000); loadWeather();
</script>

</body>
</html>
)rawliteral";
  return html;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIRPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUZZPIN, OUTPUT);

  digitalWrite(LEDPIN, LOW);
  digitalWrite(BUZZPIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Pripajam WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");
  Serial.print("Otvor v prehliadaci: http://");
  Serial.println(WiFi.localIP());

  server.on("/", [](){ server.send(200, "text/html", webPage()); });

  server.on("/alarmOn", [](){
    alarmZapnuty = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/alarmOff", [](){
    alarmZapnuty = false;
    digitalWrite(LEDPIN, LOW);
    digitalWrite(BUZZPIN, LOW);
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/state", [](){
    bool poplach = alarmZapnuty && pohyb;
    String json = "{";
    json += "\"svetlo\":" + String(svetlo) + ",";
    json += "\"pohyb\":" + String(pohyb ? 1 : 0) + ",";
    json += "\"alarm\":" + String(alarmZapnuty ? 1 : 0) + ",";
    json += "\"poplach\":" + String(poplach ? 1 : 0);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/weather", [](){
    bool ok = !isnan(outdoorTemp);
    String json = "{";
    json += "\"ok\":" + String(ok ? 1 : 0) + ",";
    json += "\"temp\":" + String(ok ? outdoorTemp : 0.0, 1);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();

  updateWeather(); // prvé načítanie počasia
}

void loop() {
  server.handleClient();

  svetlo = analogRead(LDRPIN);
  pohyb = digitalRead(PIRPIN);

  bool poplach = alarmZapnuty && pohyb;
  digitalWrite(LEDPIN, poplach);
  digitalWrite(BUZZPIN, poplach);

  updateWeather();
}