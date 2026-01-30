#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>

#include "../../lib/NEC/NEC.h"

#define PWM_IR_NODE DT_NODELABEL(emetteur_ir)
#define GPIO_LASER_NODE DT_NODELABEL(laser0)
#define EMIT_STACK_SIZE 1024

static const struct pwm_dt_spec ir_emmit = PWM_DT_SPEC_GET(PWM_IR_NODE);
static const struct gpio_dt_spec las_emit = GPIO_DT_SPEC_GET(GPIO_LASER_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

struct k_work_q emit_work_q;
struct k_work emitIR_Work;
struct k_work_delayable emitIR_DelayableWork;
K_THREAD_STACK_DEFINE(emit_stack_area, EMIT_STACK_SIZE);

unsigned long dataToSend = 0x000000FF;

// turns on the PWM for "duration" microseconds
void sendPulse(int duration)
{
    pwm_set_dt(&ir_emmit, PWM_USEC(28), PWM_USEC(9)); // set duty cycle to 25% 38kHz/4 = 9500Hz
    k_sleep(K_USEC(duration));
}

// turns off the PWM for "duration" microseconds
void sendSpace(int duration)
{
    pwm_set_dt(&ir_emmit, 0, 0); // turn off PWM
    k_sleep(K_USEC(duration));
}

// workhandler that emit IR signals after the trigger was pulled
void emitIR_WorkHandler(struct k_work *work)
{

        //gpio_pin_toggle_dt(&led2);
        dataToSend += 1;

        gpio_pin_set_dt(&las_emit, 1); // turn on laser
        sendNEC(dataToSend);            // example data to send
        LOG_DBG("TIR \n Data : 0x%08lX", dataToSend);
        k_sleep(K_MSEC(250)); // laser on duration
        gpio_pin_set_dt(&las_emit, 0); // turn on laser

        k_sleep(K_MSEC(100)); // cooldown

        k_work_reschedule_for_queue(&emit_work_q, &emitIR_DelayableWork, K_MSEC(5000)); // reschedule work
}

int main(void)
{
        gpio_pin_configure_dt(&las_emit, GPIO_OUTPUT_INACTIVE);

        k_work_queue_init(&emit_work_q);
        k_work_queue_start(&emit_work_q, emit_stack_area, K_THREAD_STACK_SIZEOF(emit_stack_area), 5, NULL);

        k_work_init_delayable(&emitIR_DelayableWork, emitIR_WorkHandler);
        k_work_schedule_for_queue(&emit_work_q, &emitIR_DelayableWork, K_MSEC(30000));

        LOG_DBG("Start emitting in 30s");


        return 0;
}
