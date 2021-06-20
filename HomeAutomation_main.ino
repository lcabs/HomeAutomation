#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define CLIENT_ID "MegaMQTT"

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient;

void setup() {
    Serial.begin(9600); // Start serial port 
  // start the Ethernet connection:
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
////////////////////////////////////////////



 mqttClient.setClient(ethClient);
 mqttClient.setServer("broker.hivemq.com",1883);






}
////////////////////////////////////////////

void loop() {
//sendData();
mqttClient.loop();

}

void sendData(){ 
    char msgBuffer[20];
      if(mqttClient.connect(CLIENT_ID)) {
   mqttClient.publish("lcabs1993", dtostrf(1,2,3,msgBuffer));
 }
  
  }
