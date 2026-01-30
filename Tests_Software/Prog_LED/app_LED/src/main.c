#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>


#define RED_NODE DT_NODELABEL(pwm_led_r)
#define GREEN_NODE DT_NODELABEL(pwm_led_g)
#define BLUE_NODE DT_NODELABEL(pwm_led_b)

#define STEP_SIZE PWM_USEC(200) /* pulse width step size */

LOG_MODULE_REGISTER(test_LED4P, LOG_LEVEL_DBG);

static const struct pwm_dt_spec red_pwm_led =
        PWM_DT_SPEC_GET(RED_NODE);
static const struct pwm_dt_spec green_pwm_led =
	PWM_DT_SPEC_GET(GREEN_NODE);
static const struct pwm_dt_spec blue_pwm_led =
	PWM_DT_SPEC_GET(BLUE_NODE);


int main(void)
{
        uint32_t pulse_red, pulse_green, pulse_blue; /* pulse widths */
	int ret;

        pwm_set_pulse_dt(&red_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&green_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&blue_pwm_led, PWM_MSEC(0));

        k_sleep(K_MSEC(5000));
        
        while(1) {
        pwm_set_pulse_dt(&red_pwm_led, PWM_MSEC(1));
        pwm_set_pulse_dt(&green_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&blue_pwm_led, PWM_MSEC(0));
        
        k_sleep(K_MSEC(1000));

        pwm_set_pulse_dt(&red_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&green_pwm_led, PWM_MSEC(1));
        pwm_set_pulse_dt(&blue_pwm_led, PWM_MSEC(0));

        k_sleep(K_MSEC(1000));

        pwm_set_pulse_dt(&red_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&green_pwm_led, PWM_MSEC(0));
        pwm_set_pulse_dt(&blue_pwm_led, PWM_MSEC(1));

        
        k_sleep(K_MSEC(1000));
        }

        return 0;
}
