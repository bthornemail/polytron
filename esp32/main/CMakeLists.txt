#
# main/CMakeLists.txt
#

idf_component_register(
    SRCS "tetra-main.c"
    INCLUDE_DIRS "."
    REQUIRES spi_flash nvs_flash esp_wifi esp_netif esp_event
)