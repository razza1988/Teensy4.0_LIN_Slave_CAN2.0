#include "Arduino.h"
#include "lin_bus.h"
#include "FlexCAN_T4.h"

LIN lin(&Serial3, 19200);                                                                                           //Set Up LIN on Serial3 Pins
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3;                                                                     // Set Up CAN2.0 on CAN3 Pins

#define Slave_PID 0x42                                                                                             //LIN ID of the Slave Module We Are Emulating

int lin_cs = 32;                                                                                                    //Pin 32 on the Teensy is connected to CS pin on LIN chip
int lin_fault = 28;                                                                                                 //Pin 28 on the Teensy is connected to Fault pin on LIN chip
CAN_message_t msg;                                                                                                  //This is the CAN message from CAN3
uint8_t buffer_data[] = {0x00, 0x00, 0x08, 0x00, 0xff, 0xff, 0xff, 0xff};                                           //This is the data we will send on the LIN Bus when asked for it from the Master
uint8_t pos_0[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70};       //These are the counter values that Byte 1 cycles through
uint8_t pos_1[] = {0x01, 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39, 0x41, 0x49, 0x51, 0x59, 0x61, 0x69, 0x71};       //
uint8_t pos_2[] = {0x02, 0x0a, 0x12, 0x1a, 0x22, 0x2a, 0x32, 0x3a, 0x42, 0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x72};       // 
uint8_t pos_3[] = {0x03, 0x0b, 0x13, 0x1b, 0x23, 0x2b, 0x33, 0x3b, 0x43, 0x4b, 0x53, 0x5b, 0x63, 0x6b, 0x73};       //
uint8_t pos = 0;
uint8_t current_pos = pos_0[pos];

void setup() {
    pinMode(lin_fault, INPUT);
    pinMode(lin_cs, OUTPUT);
    digitalWrite(lin_cs, HIGH);

    Can3.begin();
    Can3.setBaudRate(1000000);
    Can3.enableFIFO();
    Serial.begin(9600);



}

void loop () {
  Serial.println( "Loop Starting");
  byte tmp[2];  
  uint8_t i = 0;
  while (i < 2){
    if (Serial3.available() > 0){

      Serial.println("LIN Available");
      tmp[i] = Serial3.read();
      Serial.print("Byte "); Serial.print(i); Serial.print(":"); Serial.print(tmp[i]);
      Serial.println();
      i++;
    }
  }
  Serial.println("Loop Finished");
    // if (msg.buf[7] == 0x00){
    //     buffer_data[2] = 0x00;
    // }

    // if (msg.buf[7] == 0x02){
    //     buffer_data[2] = 0x08;
    // }

//     uint8_t i = lin.slave_response(Slave_PID, buffer_data, 8, lin2x);
//     if (i == 1){
//       buffer_data[1] = pos_0[pos];
//       pos++;
//       if (pos == 15){
//         pos = 0;
//       }
//     }
}

