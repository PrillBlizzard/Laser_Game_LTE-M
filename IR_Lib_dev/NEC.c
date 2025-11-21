#include "NEC.h"

// -------------------Transmitter functions------------------- //

//turns on the PWM for "duration" microseconds
void sendPulse(int duration){

    //depends on the hardware way to activate/deactivate the PWM signal

}

//turns off the PWM for "duration" microseconds
void sendSpace(int duration){

    //depends on the hardware way to activate/deactivate the PWM signal

}

//sends 32 bits of data using the NEC protocol
void sendNEC(unsigned long data){
    //send header
    sendPulse(HEADER_PULSE);
    sendSpace(HEADER_SPACE);

    //send data bits
    for(int i = 0; i < NEC_BITS; i++){
        if(data & (1UL << (32 - 1 - i))){
            //send a "1"
            sendPulse(ONE_PULSE);
            sendSpace(ONE_SPACE);
        } else {
            //send a "0"
            sendPulse(ZERO_PULSE);
            sendSpace(ZERO_SPACE);
        }
    }

    //send final pulse
    sendPulse(ONE_PULSE);
}

// -------------------Receiver functions------------------- //

//Interrupt Service Routine for IR receiver
ISR_IR_Receive(irparams_t* irparams){
    //depends on the hardware way to handle interrupts and read pin states  
}

int MATCH(uint16_t measured, uint16_t desired){

    // Allow a tolerance
    const uint16_t tolerance = desired / 4; // 25% tolerance

    // Check if measured value is what we expect (within tolerance)
    return (measured >= (desired - tolerance)) && (measured <= (desired + tolerance));
}   

//Decode the raw data from irparams into a decoded_result_t structure
//returns 0 on success, 1 for invalid header, 2 for invalid bit
//carefull when coding the ISR, it usualy uses ticks of X us mucriroseconds instead of microseconds directly
void decodeNEC(irparams_t* irparams, decoded_result_t* result){
    result->value = 0;
    

    // Check for the header pulse and space
    if(!MATCH(irparams->rawdata[0], HEADER_PULSE) || !MATCH(irparams->rawdata[1], HEADER_SPACE)){
        return 0; // Invalid header
    }
    int offset = 2; // Start after header Pulse and Space

    // Decode each bit
    for(int i =0; i < NEC_BITS; i++){
        if(MATCH(irparams->rawdata[offset], ONE_PULSE) && MATCH(irparams->rawdata[offset + 1], ONE_SPACE)){
            result->value = (result->value << 1) | 1; // Bit "1"
        } else if(MATCH(irparams->rawdata[offset], ZERO_PULSE) && MATCH(irparams->rawdata[offset + 1], ZERO_SPACE)){
            result->value = (result->value << 1); // Bit "0"
        } else {
            return 2; // Invalid bit
        }
        offset += 2; // Move to the next pulse/space pair
    }
    result->bits = NEC_BITS;
    return 0 ; // Success

}