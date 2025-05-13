set (
    Header
    "GPIO" "UART_Lora" "WIFI" "SERVER" "CALL_COP"
)
set (
    Source
    "GPIO/gpio.c"
    "UART_Lora/uart_lora.c"
    "WIFI/Smart_config.c"
    "SERVER/MQTT.c"
    "SERVER/HTTP.c"
    "CALL_COP/call_cop.c"
)

idf_component_register(SRCS "main.c" ${Source}
                    INCLUDE_DIRS "." ${Header})