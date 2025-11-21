#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>

#include "../../Lib/NEC/NEC.h"

// ---

#define PWM_NODE DT_NODELABEL(pwm_led0) 
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

#define PWM0_OUT_NODE DT_NODELABEL(pwm_out0)

#define IRMITTER_NODE DT_NODELABEL(irmitter0)

#define IR_REICV_NODE DT_NODELABEL(capteur0)

#define SW2_NODE DT_ALIAS(sw3)

// ---

// 

static const struct pwm_dt_spec irmitterPWM_dt = PWM_DT_SPEC_GET(PWM_NODE); 
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec pwmOut0 = GPIO_DT_SPEC_GET(PWM0_OUT_NODE, gpios);
static const struct gpio_dt_spec irmitter0 = GPIO_DT_SPEC_GET(IRMITTER_NODE, gpios);
static const struct gpio_dt_spec irecv = GPIO_DT_SPEC_GET(IR_REICV_NODE, gpios);


static const struct gpio_dt_spec switch2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

//static const struct gpio_dt_spec pwmOutput0 = GPIO_DT_SPEC_GET(PWM0_OUT_NODE, gpios);

// ---

irparams_t irParams = { .recvState = STATE_IDLE, .timer = 0, .rawlen = 0 };

struct k_work lR_IR_Work;

static void lR_IR_WorkHandler(struct k_work *work)
{
    gpio_pin_toggle_dt(&led2);

    uint8_t rawSignal = gpio_pin_get_dt(&irecv);

    //lR_IR_Receive(&irParams, rawSignal);

    k_sleep(K_MSEC(500));
    k_work_submit(&lR_IR_Work);      
}

// --- 

//turns on the PWM for "duration" microseconds
void sendPulse(int duration){
    pwm_set_dt(&irmitterPWM_dt, PWM_KHZ(38), PWM_HZ(9500)); //set duty cycle to 25% 38kHz/4 = 9500Hz
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

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&irmitter0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&pwmOut0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&irecv, GPIO_INPUT);

    
    gpio_pin_configure_dt(&switch2, GPIO_INPUT);
    

    int val;
    int val2;
    int val3;     
    
    k_work_init(&lR_IR_Work, lR_IR_WorkHandler);
    k_work_submit(&lR_IR_Work);  

    while(1){
            val = gpio_pin_get_dt(&irecv);
            gpio_pin_set_dt(&led1, val); 

            val2 =  gpio_pin_get_dt(&pwmOut0);
            gpio_pin_set_dt(&irmitter0, val2);

            
            val3 = gpio_pin_get_dt(&switch2);
            if(val3 == 1 ){
                pwm_set_dt(&irmitterPWM_dt, 0, 0); //set duty cycle to 50%
                k_sleep(K_MSEC(3000));
            }
            
            pwm_set_dt(&irmitterPWM_dt, PWM_MSEC(250), PWM_MSEC(125)); //set duty cycle to 25%
            
            k_sleep(K_MSEC(1));
        
    } 
    
    return 0;
}
