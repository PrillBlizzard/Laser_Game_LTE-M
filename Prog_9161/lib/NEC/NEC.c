#include "NEC.h"

// completed for nrf9161 DK

// -------------------Transmitter functions------------------- //


//sends 32 bits of data using the NEC protocol
void sendNEC(unsigned long data){
    //send header
    sendPulse(HEADER_PULSE);
    sendSpace(HEADER_SPACE);

    //send data bits
    for(int i = 0; i < NEC_BITS; i++){
        if( data & (1 << NEC_BITS-i-1) ){
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
    
    //send end space
    sendSpace(0); //no need to wait after final space
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
                    irparams->recvState = STATE_STOP;
                }
            }
            break;
        case STATE_STOP:
            if (irsignal == 0)
            {
                //mark detected, ignore
                irparams->timer = 0; //reset timer
            }
            //waiting for activation from the decode function
            break;
        



    }  

}


// Helper function to compare measured duration with desired duration
int MATCH(uint16_t measured, uint16_t desired){

    // Allow a tolerance
    const uint16_t tolerance = desired / 2; // 50% tolerance

    // Check if measured value is what we expect (within tolerance)
    return (measured >= (desired - tolerance)) && (measured <= (desired + tolerance));
}   

//Decode the raw data from irparams into a decoded_result_t structure
//returns 0 on success, 1 for invalid header, 2 for invalid bit
int decodeNEC(irparams_t* irparams, decoded_result_t* result){
    result->value = 0;
    

    // Check for the header pulse and space
    if(!MATCH(irparams->rawdata[1]*TICK_LENGHT, HEADER_PULSE) || !MATCH(irparams->rawdata[2]*TICK_LENGHT, HEADER_SPACE)){
        irparams->recvState = STATE_IDLE; // Reset state
        return 1; // Invalid header
    }
    int offset = 3; // Start after header Pulse and Space

    // Decode each bit
    for(int i =0; i < NEC_BITS; i++){

        // test if each duration and the next are of the correct length for "1" or "0" (convert ticks to microseconds by multiplying by 10)
        if(MATCH(irparams->rawdata[offset]*TICK_LENGHT, ONE_PULSE) && MATCH(irparams->rawdata[offset + 1]*TICK_LENGHT, ONE_SPACE)){
            result->value = (result->value << 1) | 1; // Bit "1"
        } else if(MATCH(irparams->rawdata[offset]*TICK_LENGHT, ZERO_PULSE) && MATCH(irparams->rawdata[offset + 1]*TICK_LENGHT, ZERO_SPACE)){
            result->value = (result->value << 1); // Bit "0"
        } else {
            irparams->recvState = STATE_IDLE; // Reset state
            // Clear rawdata for next reception
            for (int i = 0; i < irparams->rawlen; i++)
            {
                irparams->rawdata[i] = 0;
            }
            return 2; // Invalid bit
        }
        offset += 2; // Move to the next pulse/space pair
    }

    // Clear rawdata for next reception
    for (int i = 0; i < irparams->rawlen; i++)
    {
        irparams->rawdata[i] = 0;
    }
    

    result->bits = NEC_BITS;
    irparams->recvState = STATE_IDLE; // Reset state

    return 0 ; // Success

}

