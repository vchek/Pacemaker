//Master Writer
/*#include "MQTTmbed.h"
#include "mbed.h"

Serial pc(USBTX, USBRX);
Serial master(p9, p10);
Countdown cd;

int main() {
    int counter = 0;
    while (true) {
        master.printf("Hello World\n");
        pc.printf("printed something%d\r\n", counter);
        cd.countdown(1);
        while(!cd.expired());
        counter++;
    }
}*/

//Slave Reader
#include "mbed.h"
#include "MQTTmbed.h"

Serial pc(USBTX, USBRX);
Serial slave(p9, p10);

int main() {
    while (true) {
        if (slave.readable()) {
            char word[128];
            pc.printf("Something to read\r\n");
            slave.gets(word, 128);
            pc.printf("%s\r\n", word);    
        }
    }
}
