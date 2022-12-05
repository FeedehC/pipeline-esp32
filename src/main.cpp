#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <stdlib.h>
#include <PubSubClient.h>

#include "param.h"

//Get credentials from environment variables
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASS;

//Dirección IP del BROKER MQTT
const char *mqtt_server = "terrariapancho.ddns.net";

int humidity = 0;

long lastMsg = 0;
int value = 0;
float nivelLuz = 0;
float humedad = 0;
float humedadSuelo = 0;
float temperature = 0;
int tiempoMuestras = 1;
int pesoMuestras = 1;

uint8_t tempArray[20] = {0};
uint8_t N_fil = 5;
uint8_t current_temp = 0; // Temperatura actual s
uint8_t prom = 0;         // Promedio
uint8_t humeArray[20] = {0};
uint8_t current_hume = 0; // Temperatura actual s
uint8_t promhume = 0;     // Promedio

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void start_ota_webserver(void);
void callback(char *topic, byte *message, unsigned int length);
void changeState(String messageTemp, int pin);
void reconnect();
double prome(uint8_t temp[], uint8_t N_filter);
void pushData(uint8_t *tempArray, uint8_t newTemp, uint8_t N_fil);
void mandarDatos(const int Read, uint8_t *datoArray, uint8_t N_fil, const char *topic, int min, int max);

void setup(void) {
  pinMode(LED_ONBOARD, OUTPUT);
  Serial.begin(115200);

  start_ota_webserver();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    Serial.println("Cliente desconectado, intentando reconexión...");
    reconnect();
  }
  client.loop();  
  /*
  long now = millis();
  
  if (now - lastMsg > tiempoMuestras * DELAY * pesoMuestras) // 1000ms de muestreo
  {
    lastMsg = now;
    
    char humString[8];
    char tempString[8];
    mandarDatos(ANALOG_1, tempArray, N_fil, "esp32/nivelLuzz", 0, 4095);
    mandarDatos(ANALOG_2, humeArray, N_fil, "esp32/humedadSueloo", 2370, 4095);//880, 1540); //2370, 4095);

    humedad = dht.readHumidity();
    dtostrf(humedad, 1, 2, humString);
    client.publish("esp32/humidityy", humString); // esp32/humidity

    temperature = dht.readTemperature();
    dtostrf(temperature, 1, 2, tempString);
    client.publish("esp32/temperaturee", tempString); // esp32/temperature

    digitalWrite(LED_ONBOARD, !digitalRead(LED_ONBOARD));
  }*/

  digitalWrite(LED_ONBOARD, !digitalRead(LED_ONBOARD));
  delay(DELAY);
}

/*
void loop(void) {
    digitalWrite(LED, !digitalRead(LED));
    //humidity = analogRead(HUMIDITY_SENSOR);
    //Serial.println("");
    //Serial.print("Sensor de humedad: ");
    //Serial.println(humidity);
    delay(DELAY);
}
*/


//------------------------Funciones------------------------------//

void start_ota_webserver(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Bienvenido a ESP32 over-the-air (OTA).");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Mensaje Recibido en topic: ");
  Serial.print(topic);
  Serial.print(", Mensaje: ");
  String messageTemp;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  //------------------Primer Output topic esp32/output1------------------
  if (String(topic) == "esp32/output1")
  {
    changeState(messageTemp, LED_ONBOARD);
  }
  //------------------Segundo Output topic esp32/output2------------------
  if (String(topic) == "esp32/output2")
  {
    changeState(messageTemp, LED_ONBOARD);
  }
  //------------------Tercer Output topic esp32/output3------------------
  if (String(topic) == "esp32/output3")
  {
    changeState(messageTemp, LED_ONBOARD);
  }
  //------------------Cuarto Output topic esp32/output4------------------
  if (String(topic) == "esp32/output4")
  {
    // Serial.print("Cambio de salida PWM");
    // Serial.println("messageTemp");
    //ledcWrite(PWM_LED_CHANNEL, messageTemp.toInt());
  }
  // AGREGAR MAS TOPICOS PARA PODER TENER MAS GPIO O CONFIG
  if (String(topic) == "esp32/output5")
  {
    // Serial.print("Cambio de NUmero del filtro");
    // Serial.println("messageTemp");
    int aux = messageTemp.toInt();
    if (0 < aux && aux < 20)
    {
      N_fil = aux;
    }
    // Serial.println(N_fil);
  }
  // AGREGAR MAS TOPICOS PARA PODER TENER MAS GPIO O CONFIG
  if (String(topic) == "esp32/output6")
  {
    // Serial.print("Cambio de tiempo muestra");
    // Serial.println("messageTemp");
    int aux = messageTemp.toInt();
    if (0 < aux && aux < 120)
    {
      long now = millis();
      lastMsg = now;
      tiempoMuestras = aux;
    }
  }
  // AGREGAR MAS TOPICOS PARA PODER TENER MAS GPIO O CONFIG
  if (String(topic) == "esp32/output7")
  {
    // Serial.print("Cambio de tiempo muestra");
    // Serial.println("messageTemp");
    int aux = messageTemp.toInt();
    if (0 < aux && aux < 60)
    {
      long now = millis();
      lastMsg = now;
      pesoMuestras = aux;
    }
  }
}

void changeState(String messageTemp, int pin)
{
  Serial.print("Cambio de salida: ");
  if (messageTemp == "on")
  {
    Serial.println("on");
    digitalWrite(pin, HIGH);
  }
  else if (messageTemp == "off")
  {
    Serial.println("off");
    digitalWrite(pin, LOW);
  }
}

void reconnect()
{
  // Bucle hasta que se reconecte
  while (!client.connected())
  {
    Serial.print("Intentando conexion MQTT... ");
    if (client.connect("ESP32Client-fede", mqtt_user, mqtt_pass))
    {
      Serial.println("Conectado");
      client.subscribe("esp32/output1");
      client.subscribe("esp32/output2");
      client.subscribe("esp32/output3");
      client.subscribe("esp32/output4");
      client.subscribe("esp32/output5");
      client.subscribe("esp32/output6");
      client.subscribe("esp32/output7");
    }
    else
    {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

double prome(uint8_t temp[], uint8_t N_filter)
{
  uint16_t acum = 0;

  for (uint8_t i = 0; i < N_filter; i++)
  {
    acum += temp[i];
  }
  return acum / N_filter;
}

void pushData(uint8_t *tempArray, uint8_t newTemp, uint8_t N_fil)
{
  // Corro el valor anterior en el arreglo per copio todo uno a la derecha, luego pongo el valor recibido en este en CERO como en mas actual
  for (int i = (N_fil - 1); i > 0; i--)
  {
    tempArray[i] = tempArray[i - 1];
  }
  tempArray[0] = newTemp;
}

void mandarDatos(const int Read, uint8_t *datoArray, uint8_t N_fil, const char *topic, int min, int max)
{
  float dato;
  char datoString[8];
  for (size_t i = 0; i < N_fil; i++)
  {
    delay(100);
    dato = 100 - map(analogRead(Read), min, max, 0, 100);
    pushData(datoArray, dato, N_fil);
  }
  prom = prome(datoArray, N_fil);
  dtostrf(prom, 1, 2, datoString);
  client.publish(topic, datoString);
}