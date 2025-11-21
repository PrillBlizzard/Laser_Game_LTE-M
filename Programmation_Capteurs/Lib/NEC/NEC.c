#include "NEC.h"

// completed for nrf9161 DK

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

//Listening Routine for IR receiver
void lR_IR_Receive(irparams_t* irparams, uint8_t irsignal){
    
    irparams->timer++; //increment timer (in units of 10us, assuming this function is called every 10us)
    
    if(irparams->rawlen >= BUFFER_SIZE){
        irparams->rawlen = 0; //buffer overflow
    }
    
    //state machine of the receiver
    switch(irparams->recvState){

        case STATE_IDLE:                                
            //in the middle of a gap between transmissions

            if(irsignal == 0){               
                //mark detected (sensor active on low)

                if(irparams->timer < GAP_TICKS){    
                    //gap not big enough to be a new transmission

                    irparams->timer = 0;               //reset timer
                }
                else{           
                    //gap is big enough to be a new transmission                        
                    
                    irparams->rawlen = 0;                                       //reset rawlen
                    irparams->rawdata[irparams->rawlen++] = irparams->timer;    //store duration of gap
                    irparams->timer = 0;                                        //reset timer
                    irparams->recvState = STATE_MARK;                           //switch to MARK state
                }
            }
            break;
        
        case STATE_MARK:
            //a mark as been detected
            
            if(irsignal == 1 ){
                //space detected (sensor inactive on high)

                irparams->rawdata[irparams->rawlen++] = irparams->timer;    //store duration of mark
                irparams->timer = 0;                                        //reset timer
                irparams->recvState = STATE_SPACE;                          //switch to SPACE state
            }
            break;
        case STATE_SPACE:
            //a space has been detected

            if(irsignal == 0){
                //mark detected (sensor active on low)

                irparams->rawdata[irparams->rawlen++] = irparams->timer;    //store duration of space
                irparams->timer = 0;                                        //reset timer
                irparams->recvState = STATE_MARK;                           //switch to MARK state
            } 
            else {
                if(irparams-> timer > GAP_TICKS){
                    //big space detected, indicates gap between transmissions

                    //switch to STOP state
                    //don't reset timer, keep counting space width
                    irparams->recvState = STATE_IDLE;
                }
            }
            break;

        



    }  

}


// Helper function to compare measured duration with desired duration
int MATCH(uint16_t measured, uint16_t desired){

    // Allow a tolerance
    const uint16_t tolerance = desired / 4; // 25% tolerance

    // Check if measured value is what we expect (within tolerance)
    return (measured >= (desired - tolerance)) && (measured <= (desired + tolerance));
}   

//Decode the raw data from irparams into a decoded_result_t structure
//returns 0 on success, 1 for invalid header, 2 for invalid bit
void decodeNEC(irparams_t* irparams, decoded_result_t* result){
    result->value = 0;
    

    // Check for the header pulse and space
    if(!MATCH(irparams->rawdata[0], HEADER_PULSE) || !MATCH(irparams->rawdata[1], HEADER_SPACE)){
        return 1; // Invalid header
    }
    int offset = 2; // Start after header Pulse and Space

    // Decode each bit
    for(int i =0; i < NEC_BITS; i++){

        // test if each duration and the next are of the correct length for "1" or "0" (convert ticks to microseconds by multiplying by 10)
        if(MATCH(irparams->rawdata[offset]*10, ONE_PULSE) && MATCH(irparams->rawdata[offset + 1]*10, ONE_SPACE)){
            result->value = (result->value << 1) | 1; // Bit "1"
        } else if(MATCH(irparams->rawdata[offset]*10, ZERO_PULSE) && MATCH(irparams->rawdata[offset + 1]*10, ZERO_SPACE)){
            result->value = (result->value << 1); // Bit "0"
        } else {
            return 2; // Invalid bit
        }
        offset += 2; // Move to the next pulse/space pair
    }
    result->bits = NEC_BITS;
    return 0 ; // Success

}