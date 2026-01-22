#ifndef CAPTEURS_TIR_H

#define CAPTEURS_TIR_H


// ----- NODE DEFINITION FROM DEVICTREE ----- //

// Dk LED  for debugging emission and reception
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

// IR receiver node
#define IR_REICV_NODE DT_NODELABEL(capteur_ir)

// IR and LASER emitter nodes (respectively)
#define IR_EMIT_NODE DT_NODELABEL(emetteur_ir) 
#define LASER_EMIT_NODE DT_NODELABEL(laser0)

// Switch used as trigger for IR emission
#define SW2_NODE DT_ALIAS(sw3)

// GPIO to send on a LED to debug EMISSION 
#define PWM_NODE DT_NODELABEL(pwm_out0)

// ----- DEFINITION OF MACROS ----- //

//lib const
#define REICV_STACK_SIZE 1024
#define EMIT_STACK_SIZE 1024
#define DECODE_STACK_SIZE 1024

// ----- GPIO INTERRUPT AND WORKHANDLER FOR IR EMISSION AND RECEPTION ----- //

//callback from a GPIO interrupt that detect when the trigger is pulled 
void trigger_pulled(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
// workhandler that call the listening routine to get raw IR signals
void lR_IR_WorkHandler(struct k_work *work);

// workhandler that emit IR signals after the trigger was pulled
void emitIR_WorkHandler(struct k_work *work);

// workhandler that decode the raw IR signals regularly
void decodeIR_WorkHandler(struct k_work *work);

// ----- OTHER FUNCTIONS ----- //

void get_decoded_results_value(unsigned long* value); // structure that holds the decoded IR data

int init_capteursTir_module(void);

bool get_isDataDecoded(void);

void reset_decoded_results_status(void);

#endif // CAPTEURS_TIR_H

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>