
// completed for nrf9161 DK

#include <stdint.h>

// NEC protocol timing definitions in microseconds

#define HEADER_PULSE 9000
#define HEADER_SPACE 4500

#define ONE_PULSE  560
#define ONE_SPACE  1690

#define ZERO_PULSE 560
#define ZERO_SPACE 560

// Number of bits in NEC protocol

#define NEC_BITS 32

// Size of the raw data buffer of irparams_t
#define BUFFER_SIZE 100

//tick lenght
#define TICK_LENGHT 375

//minimum gap between transmissions
#define GAP_USEC 5000
#define GAP_TICKS (GAP_USEC / TICK_LENGHT) 

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5


// Structure to hold IR receiver parameters, pulse and space durations
typedef struct 
{
    uint8_t recvState;              // Current state of the receiver 

    uint8_t timer;                  // Timer for measuring pulse/space durations
    uint16_t rawdata[BUFFER_SIZE];      // Array to store raw pulse/space durations (in ticks of 10us)
    uint8_t rawlen;                 // Length of the rawdata array

}
irparams_t;

typedef struct 
{
    unsigned long value;            // Decoded values
    unsigned int bits;              // Number of bits decoded
}
decoded_result_t;




// -------------------Transmitter functions------------------- //


//turns on the PWM for "duration" microseconds. To implement based on hardware
void sendPulse(int duration);

//turns off the PWM for "duration" microseconds. To implement based on hardware
void sendSpace(int duration);

//sends data using the NEC protocol. To implement based on hardware
void sendNEC(unsigned long data);


// -------------------Receiver functions------------------- //

//Listening Routine for IR receiver
void lR_IR_Receive(irparams_t* irparams, uint8_t irsignal);

int MATCH(uint16_t measured, uint16_t desired);

//Decode the raw data from irparams into a decoded_result_t structure
int decodeNEC(irparams_t* irparams, decoded_result_t* result);