
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>

#include "capteurs_Tir.h"
#include "../../Lib/NEC/NEC.h"




//initialize logger
LOG_MODULE_REGISTER(capteur_tir, LOG_LEVEL_DBG);

// ----- GPIO and DEVICES DEFINITIONS FROM NODES ----- //

// Dk LED for debugging emission and reception
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

// IR receiver node
static const struct gpio_dt_spec irecv = GPIO_DT_SPEC_GET(IR_REICV_NODE, gpios);

// IR and LASER emitter nodes (respectively)
static const struct pwm_dt_spec irmitterPWM_dt = PWM_DT_SPEC_GET(IR_EMIT_NODE); 
static const struct gpio_dt_spec lasmitter = GPIO_DT_SPEC_GET(LASER_EMIT_NODE, gpios);

//Switch used as trigger for IR emission
static const struct gpio_dt_spec switch2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

//GPIO used as trigger for IR emission
static const struct gpio_dt_spec pwmOut0 = GPIO_DT_SPEC_GET(PWM_NODE, gpios);

// ----- DEFINITION OF WORKQUEUES AND STACK AREAS ----- //

struct k_work_q reicv_work_q;
struct k_work_q emit_work_q;
struct k_work_q decode_work_q;

K_THREAD_STACK_DEFINE(reicv_stack_area, REICV_STACK_SIZE);
K_THREAD_STACK_DEFINE(emit_stack_area, EMIT_STACK_SIZE);
K_THREAD_STACK_DEFINE(decode_stack_area, DECODE_STACK_SIZE);

// ----- DEFINITION OF WORK VARIABLES ----- //

struct k_work lR_IR_Work;
struct k_work emitIR_Work;
struct k_work decodeIR_Work;

struct k_work_delayable lR_IR_DelayableWork;
struct k_work_delayable emitIR_DelayableWork;
struct k_work_delayable decode_IR;

// ----- DEFINITION OF STRUCTURES FOR IR RECEPTION ----- //

irparams_t irParams = { .recvState = STATE_IDLE, .timer = 0, .rawlen = 0 };
decoded_result_t decodedResult = { .value = 0, .bits = 0 };
unsigned long lastDecodedValue = 0;
bool isDataDecoded = false;


// ----- OTHERS DEFINITIONS ----- //

//init data to send
unsigned long dataToSend = 0xAFAF1234;

//init callback de l'interruption de la gachette
static struct gpio_callback switch2_cb_data;






// ----- GPIO INTERRUPT AND WORKHANDLER FOR IR EMISSION AND RECEPTION ----- //

// ISR that detect when the trigger (button) is pressed
void trigger_pulled(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    LOG_DBG("Switch 2 Pressed");
    k_work_schedule_for_queue(&emit_work_q, &emitIR_DelayableWork, K_MSEC(10));
}

// workhandler that call the listening routine to get raw IR signals
void lR_IR_WorkHandler(struct k_work *work)
{
    uint8_t rawSignal = gpio_pin_get_dt(&irecv);

    //for debug with led
    int val, val2;
    val = rawSignal;
    gpio_pin_set_dt(&led1, val); 
    val2 =  gpio_pin_get_dt(&pwmOut0);
    gpio_pin_set_dt(&led3, val2);

    //call the listening routine
    lR_IR_Receive(&irParams, rawSignal);

    //reschedule the workhandler
    k_work_reschedule_for_queue(&reicv_work_q, &lR_IR_DelayableWork, K_USEC(TICK_LENGHT));
}

// workhandler that emit IR signals after the trigger was pulled
void emitIR_WorkHandler(struct k_work *work)
{

    gpio_pin_toggle_dt(&led2);
    //dataToSend = 0xABCDEF12;

    while (gpio_pin_get_dt(&switch2) == 1)
    {

        gpio_pin_set_dt(&lasmitter, 1); // turn on laser
        sendNEC(dataToSend);            // example data to send
        // LOG_DBG("TIR");
        gpio_pin_set_dt(&lasmitter, 0); // turn on laser

        k_sleep(K_MSEC(100)); // cooldown
    }

    gpio_pin_toggle_dt(&led2);

    // k_work_schedule_for_queue(&emit_work_q,&emitIR_DelayableWork, K_MSEC(500));
}

// workhandler that decode the raw IR signals regularly
void decodeIR_WorkHandler(struct k_work *work){
    
    

    //decode the received data
    int res = decodeNEC(&irParams, &decodedResult);
    if( res== 0 ){
        LOG_DBG("Decoded Result: 0x%08X, bits: %d, res: %d \n", decodedResult.value, decodedResult.bits, res); 
        isDataDecoded = true;
        lastDecodedValue = decodedResult.value;
    } else {
        //LOG_ERR("Decoding failed with res: %d ", res);
        if(res == 2 ){
            LOG_ERR("(Invalid bit detected)");
        }
        if(res == 1 ){
            //LOG_ERR("(Invalid start pulse)");
        }
    }

    //reschedule the workhandler
    k_work_reschedule_for_queue(&decode_work_q, &decode_IR, K_MSEC(100));

}

// ----- DEFINITION OF IR EMISSION FUNCTIONS FROM NEC LIBRARY (TO ADAPT TO THE HARDWARE) ----- //

// turns on the PWM for "duration" microseconds
void sendPulse(int duration)
{
    pwm_set_dt(&irmitterPWM_dt, PWM_USEC(28), PWM_USEC(9)); // set duty cycle to 25% 38kHz/4 = 9500Hz
    k_sleep(K_USEC(duration));
}

// turns off the PWM for "duration" microseconds
void sendSpace(int duration)
{
    pwm_set_dt(&irmitterPWM_dt, 0, 0); // turn off PWM
    k_sleep(K_USEC(duration));
}

// ----- INIT OF CAPTEURS & TIR MODULE ----- //

// initializes the "capteurs & tir" module, GPIOS, interrupts and workqueues
int init_capteursTir_module(void)
{
    LOG_DBG("Initializing Capteurs & Tir module...");

    //Initializing Worqueues...
    LOG_DBG("Initializing Worqueues...");
    k_work_queue_init(&reicv_work_q);
    k_work_queue_start(&reicv_work_q, reicv_stack_area, K_THREAD_STACK_SIZEOF(reicv_stack_area), 4, NULL);

    k_work_queue_init(&emit_work_q);
    k_work_queue_start(&emit_work_q, emit_stack_area, K_THREAD_STACK_SIZEOF(emit_stack_area), 5, NULL);

    k_work_queue_init(&decode_work_q);
    k_work_queue_start(&decode_work_q, decode_stack_area, K_THREAD_STACK_SIZEOF(decode_stack_area), 6, NULL);

    //Initializing GPIOs...
    LOG_DBG("Initializing GPIOs...");
    
    //DK LEDs for DEBUG mostly
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

    // IR receiver PIN
    gpio_pin_configure_dt(&irecv, GPIO_INPUT);

    // IR transmitter PIN (IR and laser respectiveley)
    gpio_pin_configure_dt(&pwmOut0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&lasmitter, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&switch2, GPIO_INPUT);

    //Initializing Callbacks and Interrupts for Trigger...
    LOG_DBG("Initializing Callbacks and Interrupts for Trigger...");
    gpio_pin_interrupt_configure_dt(&switch2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&switch2_cb_data, trigger_pulled, BIT(switch2.pin));
    gpio_add_callback(switch2.port, &switch2_cb_data);

    // Initializing delayable works and starting listening and decoding work to queue
    LOG_DBG("Initializing delayable works and starting listening work to queue...");
    k_work_init_delayable(&lR_IR_DelayableWork, lR_IR_WorkHandler);
    k_work_init_delayable(&emitIR_DelayableWork, emitIR_WorkHandler);
    k_work_init_delayable(&decode_IR, decodeIR_WorkHandler);

    LOG_DBG("Lauching receiver and decoder workhandlers...");
    k_work_schedule_for_queue(&reicv_work_q,&lR_IR_DelayableWork, K_USEC(10));
    k_work_schedule_for_queue(&decode_work_q,&decode_IR, K_MSEC(100));

    LOG_DBG("Capteurs & Tir module initialized.");
    return 0;
}

// ----- OTHER FUNCTIONS ----- //

void get_decoded_results_value(unsigned long* value)
{
    *value = lastDecodedValue;
}

void reset_decoded_results_status(void)
{
    isDataDecoded = false;
}

bool get_isDataDecoded(void)
{
    return isDataDecoded;
}


// ----- END OF FILE ----- //