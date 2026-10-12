#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h> // Librería para el sensor DHT11

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

const char* WIFI_SSID = "S22_Ultra_de_Luis";
const char* WIFI_PASSWORD = "luisito123";

// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

const char* MQTT_SERVER = "10.104.6.141"; 
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";

// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

#define DHTPIN 4       // Pin digital GPIO 4 conectado a 'out' del DHT11
#define DHTTYPE DHT11  // Seleccionamos el tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

const int LDR_PIN = 32; // Pin analógico GPIO 32 conectado a la unión de la LDR y la resistencia

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;
unsigned long ultimoEnvio = 0;

void conectarWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" conectado");
    } else {
      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");
      delay(2000);
    }
  }
}

// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {
  // 1 y 2. Obtener la temperatura y humedad
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // validacion por error en la medicion
  if (isnan(t) || isnan(h)) {
    Serial.println("Error al leer el sensor DHT11!");
    return; 
  }

  // 3. Obtener la lectura del sensor de iluminación
  int valorLDR = analogRead(LDR_PIN);
  
  // Mapero del ldr
  int porcentajeLuz = map(valorLDR, 0, 4095, 0, 100);

  // 4. Preparar los valores para poder publicarlos mediante MQTT (como texto)
  String tempStr = String(t, 2); // 2 indica los decimales
  String humStr = String(h, 2);
  String luzStr = String(porcentajeLuz);

  // 5. Publicar cada dato en su topic correspondiente
  // El parámetro 'true' al final hace que el mensaje sea "retained" (retenido por el broker)
  mqttClient.publish(TOPIC_TEMPERATURA, tempStr.c_str(), true);
  mqttClient.publish(TOPIC_HUMEDAD, humStr.c_str(), true);
  mqttClient.publish(TOPIC_LUZ, luzStr.c_str(), true);

  // 6. Mostrar las mediciones en el Monitor Serial
  Serial.print("Temperatura: ");
  Serial.print(tempStr);
  Serial.println(" °C");
  
  Serial.print("Humedad: ");
  Serial.print(humStr);
  Serial.println(" %");
  
  Serial.print("LDR ADC: ");
  Serial.print(valorLDR);
  Serial.print(" | Luz: ");
  Serial.print(luzStr);
  Serial.println(" %");
  Serial.println("------------------------------");
}

void setup() {
  Serial.begin(115200);

  // Inicializar los sensores
  dht.begin();
  
  // Opcional pero recomendado: forzar la resolución de lectura del ESP32 a 12 bits
  analogReadResolution(12);

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();

  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = millis();
    publicarSensores();
  }
}