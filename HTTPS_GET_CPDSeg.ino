#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "DHT.h"
#define DHTPIN D1     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // En mi caso AM2302

//Declaro las variables sensores
int ldr1;//pin A0
String ldr;
String hum;
String temp;
byte vol1;// pin D2 gpio4 (el D3 gpio0 y D4 gpio2 tienen que arrancar en alto)
byte incendio1;//pin D5
byte inundacion1;//pin D6
String vol;
String incendio;
String inundacion;

DHT dht(DHTPIN, DHTTYPE); // Inicializo el sensor

const char *ssid = "vodafoneA918";  //ENTER YOUR WIFI SETTINGS
const char *password = "zapic@N14";

const char *host = "www.cpdseg.online";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

//certificado de cpdseg
const char fingerprint[] PROGMEM = "49 2B 1F 03 E6 96 EA 07 D6 66 9B 2E 43 FD A4 6C CA 5E 65 BA";

void setup() {
  delay(10);
  Serial.begin(115200);

  dht.begin();
  pinMode(D2, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado  mi IP es: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    WiFiClientSecure httpsClient;

    //Leo los valores de los sensores
    ldr1 = analogRead(A0);
    ldr = String(ldr1);
    Serial.println( ldr );
    //Serial.println(host);
    float hum1 = dht.readHumidity();
    hum = String(hum1);
    float temp = dht.readTemperature();
    //temp = String(temp1);
    vol1 = digitalRead(D2);
    vol =  String(vol1);
    incendio1 = digitalRead(D5);
    incendio = String(incendio1);
    inundacion1 = digitalRead(D6);
    inundacion = String(inundacion1);

    if (isnan(hum1) || isnan(temp))  {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    Serial.print("Humedad: ");
    Serial.print(hum);
    Serial.println(" % ");
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.println(" °C ");
    Serial.print("Volumetrico: ");
    Serial.println(vol);
    Serial.print("Incendio: ");
    Serial.println(incendio);
    Serial.print("Inundacion: ");
    Serial.println(inundacion);

    String datos_a_enviar = "ldr=" + ldr + "&hum=" + hum + "&temp=" + temp + "&vol=" + vol+ "&incendio=" + incendio+ "&inundacion=" + inundacion;

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(5000); // 5 Seconds
    //delay(1000);

    //Serial.print("HTTPS Connecting");
    int r = 0; //retry counter
    while ((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
      delay(100);
      Serial.print(".");
      r++;
    }
    if (r == 30) {
      Serial.println("Connection failed");
    }
    else {
      Serial.println("Connected to web");
    }


    String code, Link;


    Link = "/Controller/Events/EventsSensorsReceiver.php?" + datos_a_enviar;


    Serial.print("requesting URL: ");
    Serial.println(host + Link);

    httpsClient.print(String("POST ") + Link + " HTTP/1.1\r\n" +
                      "Host: " + host + "\r\n" +
                      "Connection: close\r\n\r\n");

    Serial.println("request sent");

    while (httpsClient.connected()) {
      String line = httpsClient.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    Serial.println("reply was:");
    String line;
    while (httpsClient.available()) {
      line = httpsClient.readStringUntil('\n');  //Read Line by Line
      Serial.println(line); //Print response
    }

  } else {

    Serial.println("Error en la conexión WIFI");

  }

  delay(1000);
}
