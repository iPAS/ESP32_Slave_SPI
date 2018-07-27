/**
 * SlaveSPI Class
 * - adopt from gist:shaielc/SlaveSPIClass.cpp at https://gist.github.com/shaielc/e0937d68978b03b2544474b641328145
 */
#include "SlaveSPI.h"

#include <SPI.h>

#define MO   22
#define MI   23
#define MCLK 19
#define MS   18

#define SO   (gpio_num_t)32
#define SI   (gpio_num_t)25
#define SCLK (gpio_num_t)27
#define SS   (gpio_num_t)34

SPIClass master(VSPI);  // HSPI
SPISettings spi_setting(1000000, MSBFIRST, SPI_MODE);

SlaveSPI slave(HSPI_HOST);  // VSPI_HOST

static String master_msg = "";
static String slave_msg = "";

int callback_after_slave_tx_finish() {
    // Serial.println("[slave_tx_finish] slave transmission has been finished!");
    // Serial.println(slave[0]);

    return 0;
}

void printHex(String str) {
    for (int i = 0; i < str.length(); i++) {
        Serial.print(str[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    
    master.begin(MCLK, MI, MO);

    pinMode(MS, OUTPUT);
    digitalWrite(MS, HIGH);
    // slave.begin(SO, SI, SCLK, SS, 8, callback_after_slave_tx_finish);  // seems to work with groups of 4 bytes
    // slave.begin(SO, SI, SCLK, SS, 4, callback_after_slave_tx_finish);
    slave.begin(SO, SI, SCLK, SS, 2, callback_after_slave_tx_finish);
    // slave.begin(SO, SI, SCLK, SS, 1, callback_after_slave_tx_finish);  // at least 2 word in an SPI frame
}

void loop() {
    if (slave.getInputStream()->length() && digitalRead(SS) == HIGH) {  // Slave SPI has got data in.
        while (slave.getInputStream()->length()) 
            slave_msg += slave.read();
        Serial.print("slave input: ");
        printHex(slave_msg);
    }

    while (Serial.available()) {  // Serial has got data in 
        master_msg += (char)Serial.read();
    }

    while (slave_msg.length() > 0) {  // Echo it back. Slave SPI output
        slave.write(slave_msg);
        Serial.print("slave output: ");
        printHex(slave_msg);
        slave_msg = "";
    }

    while (master_msg.length() > 0) {  // From serial to Master SPI
        Serial.print("master output (serial-in): ");
        Serial.println(master_msg);

        digitalWrite(MS, LOW);
        master.beginTransaction(spi_setting);
        for (int i = 0; i < master_msg.length(); i++) {
            master_msg[i] = master.transfer(master_msg[i]);  // Return received data
            // master.transfer16(master_msg[i]);
        }  
        master.endTransaction();
        digitalWrite(MS, HIGH);

        Serial.print("master input: ");
        printHex(master_msg);
        master_msg = "";
    }
}
