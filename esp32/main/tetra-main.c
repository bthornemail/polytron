/*
 * TETRA-MAIN — ESP32 Tetragrammatron Node
 * 
 * Pure C. No Python. Constitutional stack on embedded hardware.
 * 
 * This implements the complete Tetragrammatron stack:
 *   pair → kernel → COBS → SID → witness → NRR → block chain → mesh
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"

#include "tetra-kernel.h"

static const char *TAG = "TETRA";

#define TETRA_PORT 31415
#define MAX_PACKET 1024

/* -------------------------------------------------------------------------- */
/* Wi-Fi Configuration                                                     */
/* -------------------------------------------------------------------------- */

#define EXAMPLE_ESP_WIFI_SSID      "YourWiFi"
#define EXAMPLE_ESP_WIFI_PASS      "YourPassword"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static int s_retry_num = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi connection...");
        }
    }
}

static void wifi_init_sta(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               &wifi_event_handler, NULL));
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    strncpy((char *)wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID, 
             sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS,
             sizeof(wifi_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Wi-Fi connecting to SSID:%s", EXAMPLE_ESP_WIFI_SSID);
}

/* -------------------------------------------------------------------------- */
/* Tetra-Node UDP Server Task                                              */
/* -------------------------------------------------------------------------- */

static void tetra_node_task(void *pvParameters) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Socket creation failed: %s", strerror(errno));
        vTaskDelete(NULL);
        return;
    }
    
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(TETRA_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        ESP_LOGE(TAG, "Bind failed: %s", strerror(errno));
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "═══════════════════════════════════════════════════════════════");
    ESP_LOGI(TAG, "  TETRAGRAMMATRON ESP32 NODE");
    ESP_LOGI(TAG, "═══════════════════════════════════════════════════════════════");
    ESP_LOGI(TAG, "Listening on port %d", TETRA_PORT);
    ESP_LOGI(TAG, "Kernel K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C");
    ESP_LOGI(TAG, "Constitutional constant: 0x%02X (GS - Group Separator)", CONSTITUTIONAL_C);
    ESP_LOGI(TAG, "Block chain layers: FS ← GS ← RS ← US");
    ESP_LOGI(TAG, "═══════════════════════════════════════════════════════════════");
    
    /* Initialize block chain and NRR log */
    BlockChain chain;
    chain_init(&chain, 0x4242);
    
    NRRLog nrr;
    nrr_init(&nrr);
    
    ESP_LOGI(TAG, "Block chain initialized: root=0x%04X", chain.root_sid);
    ESP_LOGI(TAG, "NRR log initialized: genesis=0x%04X", nrr.genesis_sid);
    
    uint8_t packet[MAX_PACKET];
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    
    while (1) {
        memset(packet, 0, sizeof(packet));
        memset(&client, 0, sizeof(client));
        
        ssize_t n = recvfrom(sock, packet, sizeof(packet) - 1, 0,
                            (struct sockaddr *)&client, &client_len);
        
        if (n < 0) {
            ESP_LOGE(TAG, "recvfrom error: %s", strerror(errno));
            continue;
        }
        if (n < 2) continue;
        
        int s_bit;
        Pair value = cobs_decode(packet, n, &s_bit);
        Pair result = K(value, CONSTITUTIONAL_C);
        Pair sid = compute_sid(result);
        int holonomy = holonomy_of(result);
        
        ESP_LOGI(TAG, "RX: len=%d s_bit=%d value=0x%04X -> result=0x%04X SID=0x%04X hol=%d",
                 (int)n, s_bit, value, result, sid, holonomy);
        
        /* Create witness */
        WitnessHeader wp = make_witness(value, CONSTITUTIONAL_C, 1);
        
        /* Log to NRR */
        uint8_t witness_bytes[16];
        witness_bytes[0] = (wp.sid >> 8) & 0xFF;
        witness_bytes[1] = wp.sid & 0xFF;
        witness_bytes[2] = (wp.void_sid >> 8) & 0xFF;
        witness_bytes[3] = wp.void_sid & 0xFF;
        witness_bytes[4] = wp.holonomy;
        witness_bytes[5] = wp.flags;
        
        Pair receipt = nrr_append(&nrr, witness_bytes, 6, 0);
        ESP_LOGI(TAG, "NRR receipt: 0x%04X, log tip: 0x%04X", receipt, nrr.tip_sid);
        
        /* Send response */
        uint8_t response[8];
        response[0] = 0x01;  /* Response type */
        response[1] = (sid >> 8) & 0xFF;
        response[2] = sid & 0xFF;
        response[3] = s_bit;
        response[4] = (uint8_t)holonomy;
        response[5] = (nrr.tip_sid >> 8) & 0xFF;
        response[6] = nrr.tip_sid & 0xFF;
        response[7] = receipt & 0xFF;
        
        sendto(sock, response, sizeof(response), 0,
               (struct sockaddr *)&client, client_len);
    }
    
    close(sock);
    vTaskDelete(NULL);
}

/* -------------------------------------------------------------------------- */
/* Main Entry Point                                                        */
/* -------------------------------------------------------------------------- */

void app_main(void) {
    /* Print banner */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     TETRAGRAMMATRON ESP32 NODE — CONSTITUTIONAL STACK    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    /* Print chip info */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("ESP32: %d cores, Wi-Fi%s%s, %d MB flash\n",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
           (int)(spi_flash_get_chip_size() / (1024 * 1024)));
    printf("Constitutional: pair → kernel → COBS → SID → witness → NRR → chain\n");
    printf("\n");
    
    /* Initialize Wi-Fi */
    wifi_init_sta();
    
    /* Start Tetra-Node task on core 0 */
    xTaskCreatePinnedToCore(tetra_node_task, "tetra_node", 4096, NULL, 5, NULL, 0);
    
    ESP_LOGI(TAG, "Tetra-Node task started");
}