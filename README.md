# HomeAutomation
O Arduino Mega serve como apoio à CPU Servidor do Home Assistant.
-> Redundância de Ethernet. Em caso de CPU offline, ligar CPU com relé.
-> Recebe dados MQTT e atualiza uma página web. 
-> Recebe horário do servidor NTP
-> Loga eventos num cartão SD. (online, offline, payloads)

Página web:
    Output: Ethernet status (connected)
    Output: MQTT status (connected, disconnected)
    Output: SD files
    Output: Temperature
    Output: Humidity
    Output: Luminosity
    Input: "Text"

///////// PINS
#define BUZZER_PIN 53
#define LED_PIN 51
//#define DHT11_PIN 49
#define PUSHBUTTON_PIN 47
#define PIR01_VCC 7
#define PIR01_PIN 6
#define PIR01_GND 5

1 - Estabelecendo o servidor
    No cmd, **net start mosquitto**
            **mosquitto_sub -h localhost -t "MakerIOTopic"**
    
    
    
2 - Conectando arduino à internet

3 - Enviando mensagem do arduino para o servidor

4 - Monitorando o primeiro sensor

