#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "xxxxxxxxxxx" // Put you SSID and Password here
	#define WIFI_PWD "yyyyyyyyyyy"
#endif

// ... and/or MQTT username and password
#ifndef MQTT_USERNAME
	#define MQTT_USERNAME ""
	#define MQTT_PWD ""
#endif

// ... and/or MQTT host and port
#ifndef MQTT_HOST
	#define MQTT_HOST "192.168.2.16"
	#define MQTT_PORT 1883
#endif


#define FGDATA_ENDPOINT "fgdata"
#define LIGHT1_ENDPOINT "light1"
#define CHECK_ENDPOINT "status/check"
#define MIN_MSG_LENGTH 10
#define RELAY_PIN 4 // GPIO4
#define STATION_NAME "st1"


// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);


Timer procTimer;
bool light_status = false;


// MQTT client
// For quick check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient mqtt(MQTT_HOST, MQTT_PORT, onMessageReceived);

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){

	// Called whenever MQTT connection is failed.
	if (flag == true)
		Serial.println("MQTT Broker Disconnected!!");
	else
		Serial.println("MQTT Broker Unreachable!!");

	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}


// Publish our message
void publishMessage(bool ls)
{

  bool result = false;

	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

  if(ls == true)
    result = mqtt.publish(LIGHT1_ENDPOINT, "ON"); // or publishWithQoS
  else
    result = mqtt.publish(LIGHT1_ENDPOINT, "OFF"); // or publishWithQoS
	//Serial.println("Let's publish message now!");
	//mqtt.publish("main/frameworks/sming", "Hello friends, from Internet of things :)"); // or publishWithQoS

  if (result == false) {
    Serial.println("Error on publishing message");
    // startMqttClient(); // Auto reconnect
  }
}



// // Publish our message
// void publishMessage()
// {
// 	if (mqtt.getConnectionState() != eTCS_Connected)
// 		startMqttClient(); // Auto reconnect

// 	Serial.println("Let's publish message now!");
// 	mqtt.publish("main/frameworks/sming", "Hello friends, from Internet of things :)"); // or publishWithQoS
// }


// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{

  Vector < String > stringVector;  // String Vector


	// Serial.print(topic);
	// Serial.print(":\r\n\t"); // Pretify alignment for printing
	// Serial.println(message);

  // Serial.println("Messaggio ricevuto");

  if(message.length() > MIN_MSG_LENGTH) {
    splitString(message,'#',stringVector);

    if(!stringVector.isEmpty()) {

      // Serial.println(stringVector[0]);
      // Serial.println(stringVector[1]);
      // Serial.println(stringVector[2]);

      if(stringVector[0].compareTo("1") == 0) {
        //Serial.println("Acceso");
        if(light_status == false) {
          light_status = !light_status;
          digitalWrite(RELAY_PIN, light_status);
        }
        publishMessage(true);
      } else {
        if(light_status == true) {
          light_status = !light_status;
          digitalWrite(RELAY_PIN, light_status);
        }
        publishMessage(false);
      }

    }
  }
}



// // Callback for messages, arrived from MQTT server
// void onMessageReceived(String topic, String message)
// {
// 	Serial.print(topic);
// 	Serial.print(":\r\n\t"); // Pretify alignment for printing
// 	Serial.println(message);
// }

// Run MQTT client
void startMqttClient()
{
	procTimer.stop();
	if(!mqtt.setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
  mqtt.setKeepAlive(180);
	mqtt.connect("esp8266v11");
	// Assign a disconnect callback function
	mqtt.setCompleteDelegate(checkMQTTDisconnect);
  mqtt.subscribe("fgdata");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	//procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

  pinMode(RELAY_PIN, OUTPUT);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds for connection timeout at start
}
