
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>



#include <UniversalTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <DHT.h>
#define DHT_SENSOR_PIN D7  // The ESP8266 pin D7 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

#define geel D2
#define buzzer D3
#define groen D4

// Replace with your network credentials
const char* ssid = "YOUR_NETWORK_NAME";
const char* password = "YOUR_NETWORK_PASSWORD";

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "YOUR_CHAT_ID"

// Initialize Telegram BOT
#define BOTtoken "YOUR_BOT_TOKEN"  // your Bot Token (Get from Botfather)

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);


// Get DHT11 sensor readings and return them as a String variable
String getReadings() {
  float humi = dht_sensor.readHumidity();
  // read temperature in Celsius
  float temperature_C = dht_sensor.readTemperature();
  String message = "Temperature: " + String(temperature_C) + " ºC \n";
  message += "Humidity: " + String(humi) + " % \n";
  return message;
}




//Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command to get current readings.\n\n";
      welcome += "/readings \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }
  }
}



void setup() {

  pinMode(geel, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(groen, OUTPUT);
  
  Serial.begin(9600);
  dht_sensor.begin();  // initialize the DHT sensor

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");  // get UTC time via NTP
  client.setTrustAnchors(&cert);     // Add root certificate for api.telegram.org
#endif

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
}


void loop() {



  float humi = dht_sensor.readHumidity();
  // read temperature in Celsius
  float temperature_C = dht_sensor.readTemperature();
  // check whether the reading is successful or not
  if (isnan(temperature_C) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  }
    else {
      Serial.print("Humidity: ");
      Serial.print(humi);
      Serial.print("%");

      Serial.print("  |  ");

      Serial.print("Temperature: ");
      Serial.print(temperature_C);
      Serial.print("°C");
      Serial.println(" ");

      if(temperature_C > 27){
        digitalWrite(geel, HIGH);
        digitalWrite(groen, LOW);
        digitalWrite(buzzer, HIGH);
        
      }
      else{
        digitalWrite(groen, HIGH);
        digitalWrite(geel, LOW);
        digitalWrite(buzzer, LOW);
      }
    }

    // wait a 2 seconds between readings
    delay(1000);

    if (millis() > lastTimeBotRan + botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
    }
  }
