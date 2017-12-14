#define MQTTCLIENT_QOS2 1
#define IAP_LOCATION 0x1FFF1FF1

#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

ESP8266Interface wifi(p28, p27);
unsigned long commandP[5], outputP[5], commandD[5], outputD[5];
char partID[128], deviceID[128], uuID[128];
Countdown cd;

typedef void (*IAP)(unsigned long [], unsigned long []);
IAP iap_entry;


void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    // get salted ID from message
    char* saltedID = strtok((char*)message.payload, "-");
    // get generated number
    char* number = strtok(NULL, "-");
    //printf("Message: %s\r\n", (char*) message.payload);
    //printf("Salted ID is %s\r\n", saltedID);
    printf("Generated number is %s\r\n", number);
      
   
}

int main(int argc, char* argv[]) {
    printf("----------------Begin Setup----------------------\r\n");
    // Connect to wifi
    int status = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD);
    if (status != 0) {
        printf("Error with wifi connection!\r\n");
        return -1;    
    }
    printf("Wifi connected!\r\n");
    
    MQTTNetwork mqttNetwork(&wifi);
    
    // Create MQTT Client
    MQTT::Client<MQTTNetwork, Countdown> client = MQTT::Client<MQTTNetwork, Countdown>(mqttNetwork);
    
    //Establish TCP connection with MQTT broker
    const char* hostname = "35.188.242.1";
    int port = 1883;
    int mqttStatus = mqttNetwork.connect(hostname, port);
    if (mqttStatus != 0) {
        printf("Error with connection to MQTT broker! \r\n");
        return -1;    
    }
    
    iap_entry = (IAP) IAP_LOCATION;
    // finding partID
    commandP[0] = 54;
    iap_entry(commandP, outputP);
    sprintf(partID, "%x", outputP[1]);
    
    //finding deviceID
    commandD[0] = 58;
    iap_entry(commandD, outputD);
    snprintf(deviceID, sizeof(deviceID), "%x%x%x%x", outputD[1], outputD[2], outputD[3], outputD[4]);
    
    sprintf(uuID, "%s-%s", partID, deviceID);
    printf("UUID is %s\r\n", uuID);
    
    // Connect to MQTT broker
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = uuID;
    data.username.cstring = "mbed";
    data.password.cstring = "homework";
    if (client.connect(data) != 0) {
        printf("Failed to connect to MQTT broker! \r\n");
        return -1;     
    }
    printf("Connected to MQTT broker!\r\n");
    
    // publisher topic
    char publishTopic[128];
    snprintf(publishTopic, sizeof(publishTopic), "TopicIsPacemaker");
    printf("%s\r\n", publishTopic);
    char buffer[128];
    MQTT::Message message;        
    sprintf(buffer, "%d", 100);
    message.qos = MQTT::QOS0;        
    message.retained = false;        
    message.dup = false;        
    message.payload = (void*) buffer;        
    message.payloadlen = strlen(buffer) + 1;
    printf("----------------Setup Complete----------------------\r\n");
    while(true) {
        // create message and publish
        if (client.publish(publishTopic, message) != 0) {
            printf("Failed to publish!\r\n");  
            return -1;  
        }
        printf("published something\r\n");
        cd.countdown(1);
        while(!cd.expired());
    }
    wifi.disconnect();
    return 0;
}