#define MQTTCLIENT_QOS2 1

#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "TextLCD.h"


#define HOSTNAME "35.188.242.1"
#define PORT 1883
#define IAP_LOCATION 0x1FFF1FF1

ESP8266Interface wifi(p28, p27);
TextLCD lcd(p15, p16, p17, p18, p19, p20, TextLCD::LCD16x2); // rs, e, d4-d7

unsigned long command[5];
unsigned long output[5];

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    lcd.cls();
    char *saltedID;
    char *num;
    saltedID = strtok((char*)message.payload, "-");
    num = strtok(NULL, "-");
    printf("Salted ID: %s\n", saltedID);
    printf("Generated Num: %s\n", num);
    lcd.printf("%s\n", num);
    lcd.printf("%s\n", saltedID);
}

int main(int argc, char* argv[]) {
    
    int connectStatus = 
        wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD);
    TCPSocket socket;
    socket.open(&wifi);
    socket.set_timeout(1000);
    
    // milestone 1
    /*
    const char msg[] = "GET / HTTP/1.1\r\nHost: 35.188.242.1\r\n\r\n";\
    char buff[64];
    int tcpStatus = socket.connect(HOSTNAME, 80);
    int tcpSend = socket.send(msg, strlen(msg));
    int tcpRecv = 0;
    
    while (tcpRecv != NSAPI_ERROR_WOULD_BLOCK) {
        tcpRecv = socket.recv(buff, 64);
    }
    
    
    
    printf("TCPSocket Status: %d\r\n", tcpStatus);
    printf("TCPSend Status: %d\r\n", tcpSend);
    printf("Received: \r\n%s\r\n", buff);
    */
    
    printf("Connection Status: %d\r\n", connectStatus);
    
    // Milestone 2
    
    // Create MQTT client
    MQTTNetwork mqttNetwork(&wifi);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    
    // Establish TCP connnection with MQTT broker
    int mqttStatus = mqttNetwork.connect(HOSTNAME, PORT);
    printf("MQTT Status: %d\r\n", mqttStatus);
    
    // get IDs
    typedef void (*IAP)(unsigned long[],unsigned long[]);
    IAP iap_entry;
    iap_entry=(IAP) IAP_LOCATION;
    
    char partID[100];
    command[0] = 54;
    iap_entry (command, output);
    sprintf(partID,"%x",output[1]);
    printf("Part_id: %s\n", partID);   
    
    
    char deviceID[100];
    command[0] = 58;
    iap_entry (command, output);
    for (int i = 1; i < 5; i++){
        sprintf(&deviceID[(i-1)*8],"%x",output[i]);
    }
    
    printf("Device_id: %s\n", deviceID);
    
    // connect to MQTT broker as a client
    char clientID[100];
    sprintf(clientID, "%s-%s", partID, deviceID);
    printf("UUID: %s\n", clientID);
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.username.cstring = "mbed";
    data.password.cstring = "homework";
    data.clientID.cstring = clientID;
    int mqttBroker = client.connect(data);
    printf("Broker Status: %d\r\n", mqttBroker);
    
    //subscribe
    char topicSub[100];
    sprintf(topicSub,"cis541/hw-mqtt/%s/echo", clientID);
    int subStatus = client.subscribe(topicSub, MQTT::QOS0, messageArrived);
    printf("Subscribe Status: %d\r\n", subStatus);
    
    
    // define topic
    char topic[100];
    sprintf(topic,"cis541/hw-mqtt/%s/data", clientID);
    
    printf("------------------setupComplete------------------\r\n");
    // create new messages
    while(1) {
        char buf[100];
        MQTT::Message message;
        int num = rand() % 101;
        printf("Published Number: %d\r\n", num);
        sprintf(buf,"%d", num);
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)buf;
        message.payloadlen = strlen(buf)+1;
            
        // publish
        int publishStatus = client.publish(topic, message);
        //printf("Publish Status: %d\r\n", publishStatus);
        
        client.yield(5);
        wait(5);
    }
    
    

    return 0;
}