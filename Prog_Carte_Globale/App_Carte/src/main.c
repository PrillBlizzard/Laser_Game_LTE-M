#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "capteurs_Tir.h"
#include "LTE-M.h"

#define RGB_4P_R DT_NODELABEL(led_r)
#define RGB_4P_G DT_NODELABEL(led_g)
#define RGB_4P_B DT_NODELABEL(led_b)

static const struct gpio_dt_spec led_RGB_R = GPIO_DT_SPEC_GET(RGB_4P_R, gpios);
static const struct gpio_dt_spec led_RGB_G = GPIO_DT_SPEC_GET(RGB_4P_G, gpios);
static const struct gpio_dt_spec led_RGB_B = GPIO_DT_SPEC_GET(RGB_4P_B, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void lED_react(void)
{
        gpio_pin_set_dt(&led_RGB_R, 0);
        k_sleep(K_MSEC(3000));
}

int main(void)
{
        init_capteursTir_module();
        init_LTE();

        LOG_DBG("All init functions done");
        k_sleep(K_SECONDS(1));
        LOG_DBG("Starting main loop...");

        gpio_pin_set_dt(&led_RGB_R, 1);

        unsigned long reicv_data_value;

        while (1)
        {
                if (get_isDataDecoded())
                {
                        get_decoded_results_value(&reicv_data_value);
                        if (reicv_data_value != 0)
                        {
                                LOG_DBG("Decoded Result: %08lX \n", reicv_data_value);
                                send_hit_info(reicv_data_value);
                                reset_decoded_results_status();
                                // lED_react();
                        }
                        LOG_ERR("Error empty data received");
                }

                k_sleep(K_MSEC(100));
        }

        return 0;
}
