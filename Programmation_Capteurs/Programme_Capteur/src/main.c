#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>

#include <zephyr/logging/log.h>

#include "../../Lib/NEC/NEC.h"

// ---

#define PWM_NODE DT_NODELABEL(pwm_led0) 
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

#define PWM0_OUT_NODE DT_NODELABEL(pwm_out0)

#define IR_REICV_NODE DT_NODELABEL(capteur0)
#define LASER_EMIT_NODE DT_NODELABEL(laser0)

#define SW2_NODE DT_ALIAS(sw3)

#define REICV_STACK_SIZE 1024
#define EMIT_STACK_SIZE 1024
// ---

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// ---

    struct k_work_q reicv_work_q;
    struct k_work_q emit_work_q;
    K_THREAD_STACK_DEFINE(reicv_stack_area, REICV_STACK_SIZE);
    K_THREAD_STACK_DEFINE(emit_stack_area, EMIT_STACK_SIZE);

//---
static const struct pwm_dt_spec irmitterPWM_dt = PWM_DT_SPEC_GET(PWM_NODE); 
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec pwmOut0 = GPIO_DT_SPEC_GET(PWM0_OUT_NODE, gpios);
static const struct gpio_dt_spec irecv = GPIO_DT_SPEC_GET(IR_REICV_NODE, gpios);

static const struct gpio_dt_spec lasmitter = GPIO_DT_SPEC_GET(LASER_EMIT_NODE, gpios);


static const struct gpio_dt_spec switch2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

//static const struct gpio_dt_spec pwmOutput0 = GPIO_DT_SPEC_GET(PWM0_OUT_NODE, gpios);

// ---

struct k_work lR_IR_Work;
struct k_work emitIR_Work;

struct k_work_delayable lR_IR_DelayableWork;
struct k_work_delayable emitIR_DelayableWork;

// ---

irparams_t irParams = { .recvState = STATE_IDLE, .timer = 0, .rawlen = 0 };
decoded_result_t decodedResult = { .value = 0, .bits = 0 };

unsigned long dataToSend = 0x00000000;

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
    gpio_pin_set_dt(&lasmitter, 1); //turn on laser
    dataToSend = 0xABCDEF12; 
    
    while(gpio_pin_get_dt(&switch2) == 1){



        sendNEC(dataToSend); //example data to send
        LOG_DBG("TIR");

        k_sleep(K_MSEC(250)); //cooldown
    }

    gpio_pin_set_dt(&lasmitter, 0); //turn on laser
    gpio_pin_toggle_dt(&led2);

    //k_work_schedule_for_queue(&emit_work_q,&emitIR_DelayableWork, K_MSEC(500));
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
    //k_work_schedule_for_queue(&emit_work_q,&emitIR_DelayableWork, K_MSEC(2000));

    // pwm_set_dt(&irmitterPWM_dt, PWM_KHZ(38), PWM_HZ(9500)); //set duty cycle to 25% 38kHz/4 = 9500Hz

    gpio_pin_interrupt_configure_dt(&switch2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&switch2_cb_data, trigger_pulled, BIT(switch2.pin));
    gpio_add_callback(switch2.port, &switch2_cb_data);

    while(1){
        val = gpio_pin_get_dt(&irecv);
        gpio_pin_set_dt(&led1, val); 

        val2 =  gpio_pin_get_dt(&pwmOut0);
            
        int res = decodeNEC(&irParams, &decodedResult);

        if(res == 0){
            LOG_DBG("Decoded Result: 0x%08lX, bits: %d, res: %d \n", decodedResult.value, decodedResult.bits, res);  
   
        }
        else if (res == 2)
        {
            //LOG_DBG("Error Detected : Invalid BIT \n");
            

        }

        k_sleep(K_MSEC(500));
        
    } 
    
    return 0;
}
