
//general definitions for NEC protocol IR communication, both transmitter and receiver.
//the ISR to listen to the right pin (for reception) is left to the user implementation
//Same for the PWM activation/deactivation (for transmission) in the sendPulse and sendSpace functions




#include <stdint.h>


#define ONE_PULSE  560
#define ZERO_PULSE 560
#define ONE_SPACE  1690
#define ZERO_SPACE 560

#define HEADER_PULSE 9000
#define HEADER_SPACE 4500

#define NEC_BITS 32

// Structure to hold IR receiver parameters, pulse and space durations
typedef struct 
{
    uint8_t recvPin;                // Pin connected to the IR receiver
    uint8_t recvState;              // Current state of the receiver 

    uint8_t timer;                  // Timer for measuring pulse/space durations
    uint16_t rawdata[100];      // Array to store raw pulse/space durations
    uint8_t rawlen;                 // Length of the rawdata array

}
irparams_t;

typedef struct 
{
    unsigned long value;            // Decoded value
    unsigned int bits;              // Number of bits decoded
}
decoded_result_t;




// -------------------Transmitter functions------------------- //

//turns on the PWM for "duration" microseconds
void sendPulse(int duration);

//turns off the PWM for "duration" microseconds
void sendSpace(int duration);

//sends data using the NEC protocol
void sendNEC(unsigned long data);


// -------------------Receiver functions------------------- //

//Interrupt Service Routine for IR receiver
ISR_IR_Receive(irparams_t* irparams);

int MATCH(uint16_t measured, uint16_t desired);

//Decode the raw data from irparams into a decoded_result_t structure
void decodeNEC(irparams_t* irparams, decoded_result_t* result);