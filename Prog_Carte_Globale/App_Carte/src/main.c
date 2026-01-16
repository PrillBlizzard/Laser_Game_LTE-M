#include <zephyr/kernel.h>
#include "capteurs_Tir.h"
#include "LTE-M.h"


int main(void)
{
        init_capteursTir_module();
        init_LTE();
        k_sleep(K_SECONDS(3));

        int res = send_hit_info(0xABCDEF12); //example code  //to replace with decodedResult.value when hardware integration is done
        // fait crasher l'OS

        return 0;
}
