#define MQTTCLIENT_QOS2 1

#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#define IAP_LOCATION 0x1FFF1FF1;

ESP8266Interface wifi(p28, p27);
unsigned long command54[5];
unsigned long command58[5];
unsigned long output54[5];
unsigned long output58[5];
typedef void (*IAP) (unsigned long [], unsigned long []);
IAP iap_entry;
int bpm;
void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char* charMessage = (char*) message.payload;
    // char* salted_id;
    // char* generated;
    // salted_id = strtok(charMessage, "-");
    // generated = strtok(NULL, "-");
    printf("reset bpm to 0");
    bpm = 0;
    printf("%s\n", charMessage);
}

int main(int argc, char* argv[]) {
    //printf("Ethernet socket example\n");
    bpm = 0;
    if (wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD) != 0){
        printf("WiFi Connection Error\n");
        return 1;
    }
    printf("connected to wifi\n");
    //printf("WiFi Connection\n");
    iap_entry = (IAP) IAP_LOCATION;
    command54[0] = 54;
    command58[0] = 58;
    //printf("Runs iap_entry\n");
    iap_entry(command54, output54);
    //printf("Finishes first iap_entry\n");
    iap_entry(command58, output58);
    //printf("Finishes second iap_entry\n");
    MQTTNetwork mqttNetwork(&wifi);
    
    //printf("Gets to client\n");
    MQTT::Client<MQTTNetwork, Countdown> client = MQTT::Client<MQTTNetwork,Countdown>(mqttNetwork);
    const char* hostname = "35.188.242.1";
    int port = 1883;
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0) {
        printf("Connection Error");
        return -1;
    }
    printf("set up broker channel\n");
    //printf("gets to data\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.username.cstring = "mbed";
    data.password.cstring = "homework";
    char dataBuffer[128];
    sprintf(dataBuffer, "%x-%x%x%x%x", output54[1], output58[1], output58[2], output58[3], output58[4]);
    data.clientID.cstring = dataBuffer;
    if ((rc = client.connect(data)) != 0) {
        printf("Have a Connection Error");
        return -1;
    }
    printf("set up client\n");
    /*
    char topicBuffer[128];
    sprintf(topicBuffer, "Group3Pace");
    if ((rc = client.subscribe(topicBuffer, MQTT::QOS0, messageArrived)) != 0) {
        printf("Subscribe Error");
        return -1;
    }
    */
    printf("successfully subscribed to Group3Pace\n");
    
    char topicBuffer2[128];
    sprintf(topicBuffer2, "Group3Heart");
    //printf("%s\n", topicBuffer2);
    printf("before while loop\n");
    while (1) {

        bpm = rand()%7;
        printf("bpm is %d\n", bpm);
        while (bpm > 0) {
            printf("bpm loop\n");
            bpm--;
            wait(1);
        }
        printf("gets past bpm wait\n");
        char buf[100];
        MQTT::Message message;
        sprintf(buf,"%d", bpm);
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)buf;
        message.payloadlen = strlen(buf);
        
        sprintf(topicBuffer2, "Group3Heart");
        if ((rc = client.publish(topicBuffer2, message)) != 0) {
            printf("Publish Error");
            return -1;
        }

        client.yield(5);
        wait(5);
        
    }
    wifi.disconnect();
    return 0;
}