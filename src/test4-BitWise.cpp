#include "Arduino.h"
#include "lin_bus.h"
#include "FlexCAN_T4.h"

LIN lin(&Serial3, 19200);                                                                                           //Set Up LIN on Serial3 Pins
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3;                                                                     // Set Up CAN2.0 on CAN3 Pins

#define Slave_PID 0x42                                                                                             //LIN ID of the Slave Module We Are Emulating
#define b0 0x01
#define b1 0x02
#define b2 0x04
#define b3 0x08
#define b4 0x10
#define b5 0x20
#define b6 0x40
#define b7 0x80

int lin_cs = 32;                                                                                                    //Pin 32 on the Teensy is connected to CS pin on LIN chip
int lin_fault = 28;                                                                                                 //Pin 28 on the Teensy is connected to Fault pin on LIN chip
CAN_message_t msg;                                                                                                  //This is the CAN message from CAN3
uint8_t buffer_data[] = {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff};                                           //This is the data we will send on the LIN Bus when asked for it from the Master
uint8_t  pos_options[4] [15] = {
    {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70}, 
    {0x01, 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39, 0x41, 0x49, 0x51, 0x59, 0x61, 0x69, 0x71}, 
    {0x02, 0x0a, 0x12, 0x1a, 0x22, 0x2a, 0x32, 0x3a, 0x42, 0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x72}, 
    {0x03, 0x0b, 0x13, 0x1b, 0x23, 0x2b, 0x33, 0x3b, 0x43, 0x4b, 0x53, 0x5b, 0x63, 0x6b, 0x73}};        //These are the counter values that Byte 1 cycles through
uint8_t pos = 0;
uint8_t array = 0;
//uint8_t current_pos = pos_options[pos][pos_array];
uint8_t can_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t data[3] = {0x00, 0x00, 0x00};
uint8_t i = 0;
uint8_t flash_requested = 0;
uint8_t flash_on = 1;
uint8_t flash_on_count = 0;
uint8_t flash_off = 3;
uint8_t flash_off_count = 0;
uint8_t number_of_flashes = 5;
uint8_t flash_count = 0;
uint8_t indicator_toggle = 8;
uint8_t indicator_counter_R = 0;
uint8_t indicator_counter_L = 0;

void setup() {
    pinMode(lin_fault, INPUT);
    pinMode(lin_cs, OUTPUT);
    digitalWrite(lin_cs, HIGH);

    Can3.begin();
    Can3.setBaudRate(1000000);
    //Can3.enableFIFO();
    Serial.begin(9600);



}

int doCheck(uint8_t data[]){
    if (data[0] == 0x00 && data[1] == 0x55 && data[2] == Slave_PID){
        return 1;
    }
    return 0;

}

void loop () {
    while (Serial3.available() > 0 && i < 2 ) {
        data[i] = Serial.read();
        i++;
        if (doCheck(data) == 1){
            buffer_data[1] = pos_options[array][pos];
            lin.write_slave(Slave_PID, buffer_data, 8, lin2x);
            pos++;
            if (pos ==15){
                pos = 0;
            } 
        }
    }

    while (Serial3.available() > 0){
        uint8_t newData = Serial3.read();
        uint8_t tmp2 = data[2];
        uint8_t tmp1 = data[1];
        data[2] = newData;
        data[1] = tmp2;
        data[0] = tmp1;
        if (doCheck(data) == 1){
            buffer_data[1] = pos_options[array][pos];
            if (flash_requested == 1){
                Serial.println("Flash Requested");
                if (flash_count < number_of_flashes){
                    Serial.println("Flash Counting");
                    if (flash_on_count < flash_on){
                        Serial.println("Flash On");
                        buffer_data[2] |= (b5);
                        flash_on_count++;
                    }
                    if ((flash_on_count == flash_on) && (flash_off_count < flash_off)){
                        Serial.println("Flash Off");
                        buffer_data[2] &= ~(b5);
                        flash_off_count++;
                    }
                    if ((flash_on_count == flash_on) && (flash_off_count == flash_off)){
                        Serial.println("Flash Counter Incrimanting");
                        flash_count++;
                        flash_on_count = 0;
                        flash_off_count = 0;
                    }
                }
                
                if (flash_count == number_of_flashes){
                    Serial.println("Reset");
                    flash_on_count = 0;
                    flash_off_count = 0;
                    flash_count = 0;
                    flash_requested = 0;
                }

                
            }

            lin.write_slave(Slave_PID, buffer_data, 8, lin2x);
            pos ++;
            if (pos == 15){
                pos = 0;
            }


        }
    }


    if (Can3.read(msg) > 0){

        // No Buttons Pressed
        if (msg.buf[7] == 0x00){
            buffer_data[2] = 0x00;
        //    flash_on = 0;
        }
        // Radio Button
        // if (msg.buf[7] == 0x01){
        if (msg.buf[7] & (b0)){    

        }

        // Left Indicator Button 
        if (msg.buf[7] & (b1)){
            indicator_counter_L++;
            if (indicator_counter_L <= indicator_toggle){
                buffer_data[2] |= (b2);
                buffer_data[2] &= ~(b3);
            }
            if (indicator_counter_L > indicator_toggle){
                buffer_data[2] &= ~(b2);
                buffer_data[2] |= (b3);
            }
        }
        else {
            buffer_data[2] &= ~(b2); 
            buffer_data[2] &= ~(b3); 
            indicator_counter_L = 0;
        }
        
        // Wiper Button
        if (msg.buf[7] & (b2)){
            buffer_data[0] |= (b3);
        }
        else {
            buffer_data[0] &= ~(b3);
        }

        // Dash Button
        if (msg.buf[7] & (b3) ){

        }

        // Drink Button
        if (msg.buf[6] & (b0)){
            buffer_data[0] |= (b4);           
        }
        else {
            buffer_data[0] &= ~(b4);
        }

        // PSL Button
        if (msg.buf[7] & (b7)){

        }

        // Right Indicator Button 
        if (msg.buf[7] & (b6)){
            indicator_counter_R++;
            if (indicator_counter_R <= indicator_toggle){
                buffer_data[2] |= (b0);
                buffer_data[2] &= ~(b1);
            }
            if (indicator_counter_R > indicator_toggle){
                buffer_data[2] &= ~(b0);
                buffer_data[2] |= (b1);
            }
        }
        else {
            buffer_data[2] &= ~(b0); 
            buffer_data[2] &= ~(b1); 
            indicator_counter_R = 0;
        }

            
        

        // High Button 
        if ((msg.buf[7] & (b5)) ){
            // buffer_data[2] = 0x20;
           flash_requested = 1;   
        }

        // Wiper position 0
        if (((msg.buf[0] | ~(b2)) == ~(b2)) && ((msg.buf[0] | ~(b1)) == ~(b1)) && (msg.buf[0] & (b0))){
            array = 0;
            buffer_data[0] &= ~(b0);
            buffer_data[0] &= ~(b1);
            
        }

        // Wiper position 1
        if (((msg.buf[0] | ~(b2)) == ~(b2)) && (msg.buf[0] & (b1)) && ((msg.buf[0] | ~(b0)) == ~(b0))){
            array = 3;
            buffer_data[0] |= (b0);
            buffer_data[0] &= ~(b1);
        


        }

        // Wiper position 2
        if (((msg.buf[0] | ~(b2)) == ~(b2)) && (msg.buf[0] & (b1)) && (msg.buf[0] & (b0))){
            array = 3;
            buffer_data[0] &= ~(b0);
            buffer_data[0] |= (b1);
            
        }

        // Wiper position 3
        if ((msg.buf[0] & (b2)) && ((msg.buf[0] | ~(b1)) == ~(b1)) &&  ((msg.buf[0] | ~(b0)) == ~(b0))){
            array = 3;
            buffer_data[0] |= (b0);
            buffer_data[0] |= (b1);
            
        }

        // Dim position 1
        if (msg.buf[0] == 0x11 || 0x12 || 0x13 || 0x14){

        }

        // Dim position 2
        if (msg.buf[0] == 0x21 || 0x22 || 0x23 || 0x24){
            
        }

        // Dim position 3
        if (msg.buf[0] == 0x31 || 0x32 || 0x33 || 0x34){
            
        }

        // Dim position 4
        if (msg.buf[0] == 0x41 || 0x42 || 0x43 || 0x44){
            
        }

        // Dim position 5
        if (msg.buf[0] == 0x51 || 0x52 || 0x53 || 0x54){
            
        }

        // Dim position 6
        if (msg.buf[0] == 0x61 || 0x62 || 0x63 || 0x64){
            
        }

        

    }

}
    
















