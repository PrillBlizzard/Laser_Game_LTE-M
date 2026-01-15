cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(App_Carte)


target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/capteurs_Tir.c)
target_sources(app PRIVATE ../Lib/NEC/NEC.c)
target_sources(app PRIVATE src/LTE-M.c)


set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

generate_inc_file_for_target(
    app
    src/https-cert.der
    ${gen_dir}/https-cert.der.inc
    )