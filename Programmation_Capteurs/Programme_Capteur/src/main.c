#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/spi.h>

#include <zephyr/logging/log.h>

#include "../../Lib/NEC/NEC.h"

// ---

//Node Definition From Devicetree
#define PWM_NODE DT_NODELABEL(pwm_led0) 
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

#define PWM0_OUT_NODE DT_NODELABEL(pwm_out0)

#define IR_REICV_NODE DT_NODELABEL(capteur0)
#define LASER_EMIT_NODE DT_NODELABEL(laser0)

#define SW2_NODE DT_ALIAS(sw3)

//SPI ports: SCK:13 MISO:12 MOSI:11
#define STRIP_NODE DT_NODELABEL(spi2)

// ---

//local const
#define REICV_STACK_SIZE 1024
#define EMIT_STACK_SIZE 1024

// ---

//initialize logger
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// ---

    //initialize workqueues
    struct k_work_q reicv_work_q;
    struct k_work_q emit_work_q;
    K_THREAD_STACK_DEFINE(reicv_stack_area, REICV_STACK_SIZE);
    K_THREAD_STACK_DEFINE(emit_stack_area, EMIT_STACK_SIZE);

//---

//GPIO configurations from devicetree NODE
static const struct pwm_dt_spec irmitterPWM_dt = PWM_DT_SPEC_GET(PWM_NODE); 
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec pwmOut0 = GPIO_DT_SPEC_GET(PWM0_OUT_NODE, gpios);
static const struct gpio_dt_spec irecv = GPIO_DT_SPEC_GET(IR_REICV_NODE, gpios);

static const struct gpio_dt_spec lasmitter = GPIO_DT_SPEC_GET(LASER_EMIT_NODE, gpios);

static const struct gpio_dt_spec switch2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);



// ---

//initialisation des works
struct k_work lR_IR_Work;
struct k_work emitIR_Work;

struct k_work_delayable lR_IR_DelayableWork;
struct k_work_delayable emitIR_DelayableWork;

// ---

//Paramètres recepteur (1)
irparams_t irParams = { .recvState = STATE_IDLE, .timer = 0, .rawlen = 0 };
decoded_result_t decodedResult = { .value = 0, .bits = 0 };

//init data to send
unsigned long dataToSend = 0x00000000;

//init callback de l'interruption de la gachette
static struct gpio_callback switch2_cb_data;


// ---

// ISR that detect when the trigger (button is pressed)
void trigger_pulled(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    LOG_DBG("Switch 2 Pressed");
    k_work_schedule_for_queue(&emit_work_q,&emitIR_DelayableWork, K_MSEC(10));
}

// workhandler that call the listening routine to get raw IR signals
static void lR_IR_WorkHandler(struct k_work *work)
{
    uint8_t rawSignal = gpio_pin_get_dt(&irecv);
    
    lR_IR_Receive(&irParams, rawSignal);

    k_work_reschedule_for_queue(&reicv_work_q,&lR_IR_DelayableWork, K_USEC(TICK_LENGHT));
}

// workhandler that emit IR signals after the trigger was pulled
static void emitIR_WorkHandler(struct k_work *work)
{

    gpio_pin_toggle_dt(&led2);
    dataToSend = 0xABCDEF12; 
    
    while(gpio_pin_get_dt(&switch2) == 1){


        gpio_pin_set_dt(&lasmitter, 1); //turn on laser
        sendNEC(dataToSend); //example data to send
        //LOG_DBG("TIR");
        gpio_pin_set_dt(&lasmitter, 0); //turn on laser

        k_sleep(K_MSEC(100)); //cooldown
    }

    gpio_pin_toggle_dt(&led2);

    //k_work_schedule_for_queue(&emit_work_q,&emitIR_DelayableWork, K_MSEC(500));
}

// --- 

void write_RGB_Strip_LEDs(const struct device *dev, struct spi_cs_control *cs, uint8_t* RGB_data, int num_leds){
    struct spi_config config;

    //frequency is 4MHz, it is more than necessary, so we will send 11110 for 1 and 10000 for 0, to follow protocol as described in the SK6812 datasheet
    //It turns byte into 4 times the size (uint32 per byte)
    config.frequency = 4000000;
    config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;
    config.slave = 0;
    config.cs = *cs;

    uint64_t converted_data;
    uint64_t uint64_buff[3]; //each LED requires 3 uint32 (G,R,B)

    for(int i = 0; i < num_leds; i++){
        // Each LED requires uint32 bytes (G, R, B)
        // Data is expected in GRB format
        // Assuming RGB_data is an array of size 3
        for(int j = 0; j < 3; j++){
            converted_data = 0;
            for(int k =0; k < 8; k++){
                //MSB first
                if(RGB_data[j] & (1 << (8-i)) ){
                    //it's a "1"
                    converted_data =(converted_data << 1) | 1; // append 1
                    converted_data =(converted_data << 1) | 1; // append 1
                    converted_data =(converted_data << 1) | 1; // append 1
                    converted_data =(converted_data << 1) | 1; // append 1
                    converted_data =(converted_data << 1) ; // append 0
                }
                else{
                    //it's a "0"
                    converted_data =(converted_data << 1) | 1; // append 1
                    converted_data =(converted_data << 1) ; // append 0
                    converted_data =(converted_data << 1) ; // append 0
                    converted_data =(converted_data << 1) ; // append 0
                    converted_data =(converted_data << 1) ; // append 0

                }
            }
            uint64_buff[j] = converted_data;
            //LOG_DBG("Converted data : %llu", converted_data);
        }

        uint8_t buff[15]; //5 octets par couleurs 

        for(int m =0; m <3; m++){
            //on redivise ce qu'on doit envoyer en octet pour le SPI (et pour éviter d'envoyer des données en trop)+
            for(int n =0; n <5; n++){
                buff[m*5 + n] = (uint64_buff[m] >> (32 - 8*(n+1) ) ) & 0xFF; //extract each byte from the uint32
            }
            
        }

        //uint8_t buff[3] = { 0xAC, 0xAD, 0xAE };
        int len = 15*sizeof(buff[0]);

        struct spi_buf tx_buf = { .buf = buff, .len = len };
        struct spi_buf_set tx_bufs = { .buffers = &tx_buf, .count = 1 };
        int ret = spi_write(dev, &config, &tx_bufs);
        if (ret != 0) {
            LOG_ERR("SPI write failed: %d", ret);
        }

    }

    
}

// ---

//turns on the PWM for "duration" microseconds
void sendPulse(int duration){
    pwm_set_dt(&irmitterPWM_dt, PWM_USEC(28), PWM_USEC(9)); //set duty cycle to 25% 38kHz/4 = 9500Hz
    k_sleep(K_USEC(duration));  
}

//turns off the PWM for "duration" microseconds
void sendSpace(int duration){
    pwm_set_dt(&irmitterPWM_dt, 0, 0); //turn off PWM
    k_sleep(K_USEC(duration));
}

// -----


int main(void)
{

    k_work_queue_init(&reicv_work_q);
    k_work_queue_start(&reicv_work_q, reicv_stack_area, K_THREAD_STACK_SIZEOF(reicv_stack_area) , 4, NULL);

    k_work_queue_init(&emit_work_q);
    k_work_queue_start(&emit_work_q, emit_stack_area, K_THREAD_STACK_SIZEOF(emit_stack_area) , 5, NULL);

    LOG_DBG("Main Started");

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&pwmOut0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&irecv, GPIO_INPUT);

    gpio_pin_configure_dt(&lasmitter, GPIO_OUTPUT_INACTIVE);
    
    gpio_pin_configure_dt(&switch2, GPIO_INPUT);
    

    int val;
    int val2;   
    
    
    k_work_init_delayable(&lR_IR_DelayableWork, lR_IR_WorkHandler);
    k_work_schedule_for_queue(&reicv_work_q,&lR_IR_DelayableWork, K_USEC(10));

    k_work_init_delayable(&emitIR_DelayableWork, emitIR_WorkHandler);

    gpio_pin_interrupt_configure_dt(&switch2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&switch2_cb_data, trigger_pulled, BIT(switch2.pin));
    gpio_add_callback(switch2.port, &switch2_cb_data);

    //RGB_LEDs
    const struct device *const dev = DEVICE_DT_GET(STRIP_NODE);
    struct spi_cs_control cs_ctrl = (struct spi_cs_control){.gpio = GPIO_DT_SPEC_GET(STRIP_NODE, cs_gpios), .delay = 0u,};

    uint8_t RGB_to_send[] = {0xFF, 0x00, 0xFF}; // carefull the format is Green Red Blue

    while(1){

        
        write_RGB_Strip_LEDs(dev, &cs_ctrl, RGB_to_send , 2); //cyan


        val = gpio_pin_get_dt(&irecv);
        gpio_pin_set_dt(&led1, val); 

        val2 =  gpio_pin_get_dt(&pwmOut0);
            
        int res = decodeNEC(&irParams, &decodedResult);

        if(res == 0){
            LOG_DBG("Decoded Result: 0x%08X, bits: %d, res: %d \n", decodedResult.value, decodedResult.bits, res);  
   
        }
        else if (res == 2)
        {
            //LOG_DBG("Error Detected : Invalid BIT \n");
            

        }

        k_sleep(K_MSEC(500));
        
    } 
    
    return 0;
}
