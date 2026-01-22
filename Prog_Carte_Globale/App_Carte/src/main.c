#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "capteurs_Tir.h"
#include "LTE-M.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
        init_capteursTir_module();
        init_LTE();

        LOG_DBG("All init functions done");
        k_sleep(K_SECONDS(3));
        LOG_DBG("Starting main loop...");

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
                        }
                        LOG_DBG("Error empty data received");
                }

                k_sleep(K_SECONDS(1));
        }

        ; // example code  //to replace with decodedResult.value when hardware integration is done

        return 0;
}
