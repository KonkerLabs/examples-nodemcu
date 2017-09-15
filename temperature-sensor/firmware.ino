// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


// Internet access
const char* ssid = "demokonker";
const char* password = "yellowkonker";

// Konker MQTT
const char* mqtt_server = "mqtt.demo.konkerlabs.net";

const char* USER = "7ljdqsu38okd";
const char* PWD = "HElUADf2mwzY";
const char* PUB = "data/7ljdqsu38okd/pub/temperatura";

//global scope
char bufferJ[256];
char *message;
float temperature;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Entra no Loop ate estar conectado
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Usando um ID unico (Nota: IDs iguais causam desconexao no Mosquito)
    // Tentando conectar
    if (client.connect(USER, USER, PWD)) {
      Serial.println("connected");      
    } else {
      Serial.print("Falhou! Codigo rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Esperando 5 segundos para tentar novamente
      delay(5000);
    }
  }
}


char *jsonMQTTmsgDATA(const char *device_id, const char *metric, float value)
{
    StaticJsonBuffer<200> jsonMQTT;
    JsonObject& jsonMSG = jsonMQTT.createObject();
      jsonMSG["deviceId"] = device_id;
      jsonMSG["metric"] = metric;
      jsonMSG["value"] = value;
      jsonMSG.printTo(bufferJ, sizeof(bufferJ));
      return bufferJ;
}

void setup_wifi() {
  delay(10);
  // Agora vamos nos conectar em uma rede Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Imprimindo pontos na tela ate a conexao ser estabelecida!
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereco de IP: ");
  Serial.println(WiFi.localIP());
}

/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Start GDC Temperature sensor demo");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Start up the library
  sensors.begin();
}

/*
 * Main function, get and show the temperature
 */
void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  temperature = 0;
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
  
  if (!client.connected()) {
    reconnect();
  }

  Serial.println("Send data to Konker Platform...");
  
  message = jsonMQTTmsgDATA("My_favorite_thermometer", "Celsius", temperature);
  
  client.publish(PUB, message); 
  client.loop();
  Serial.println("Done, sleep for 5 seconds...");
  delay(5000);
  
}