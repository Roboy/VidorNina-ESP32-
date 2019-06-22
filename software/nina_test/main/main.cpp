#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
//#include "driver/spi_master.h"
#include "SPIbus.hpp"
#include "FpgaVidor.hpp"
#include "interface.hpp"
#include "mode_ctl.hpp"

static const char *TAG = "MQTTS_SAMPLE";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

//Config
#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "roboy"
#endif

#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "wiihackroboy"
#endif

static esp_mqtt_client_handle_t mqtt_client = NULL;



#define SPI_MODE  1//original 0
#define MISO_PIN  19
#define MOSI_PIN  23
#define SCLK_PIN  18
#define CS_PIN    5
#define SPI_CLOCK 1000000//SPI_MASTER_FREQ_16M   // 1 MHz



static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_ap_config_t wifi_cfg = {CONFIG_WIFI_SSID,CONFIG_WIFI_PASSWORD};
    wifi_config_t wifi_config = {wifi_cfg};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos3", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            //msg_id = esp_mqtt_client_publish(client, CONFIG_EMITTER_CHANNEL_KEY"/topic/", "data", 0, 0, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_app_start(void)
{
  const esp_mqtt_client_config_t mqtt_cfg = {mqtt_event_handler, "mqtt://192.168.1.1", "mqtt://192.168.1.1"};

  //esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(mqtt_client);

}




//static void wifi_init(void)

extern "C" void app_main() {
    printf("--START-- \n");
    fflush(stdout);


    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //nvs_flash_init();
    //wifi_init();
    //mqtt_app_start();

    SPI_t &mySPI = vspi;  // vspi and hspi are the default objects

    //nvs_flash_init();

    printf("\nbegin SPI");
    ESP_ERROR_CHECK( mySPI.begin(MOSI_PIN, MISO_PIN, SCLK_PIN,8));
    ESP_ERROR_CHECK( mySPI.addDevice(SPI_MODE, SPI_CLOCK, CS_PIN, &mySPI.device_fpga));

    ros_interface ros_i(&mqtt_client);
    hardware_interface hw (&mySPI,&ros_i);
    fpga_mode modef(&hw);

    uint8_t buffer[6];

    printf("\n\n=====================================");
    printf("\n\n  start loopy");
    printf("\n\n=====================================");

    nvs_flash_init();
    wifi_init();
    mqtt_app_start();

    hw.allow_input_trigger();
    printf("\nID: %d", hw.getID());

    hw.stop_US_out();
    hw.piezo_set_burst_cycles(3);
    hw.piezo_burst_out();


    int i = 0;
    while (1) {
        ESP_ERROR_CHECK(mySPI.readBytes(mySPI.device_fpga, 0x3B, 6, buffer));

        std::stringstream ss;
        ss << i;

        //push_pub("/topic/qos3", ss.str(), mqtt_client);

        ros_i.push_pub("/topic/qos3", ss.str());

        /*int msg_id = 0;

        msg_id = esp_mqtt_client_publish(mqtt_client, "/topic/qos3", "foo data", 0, 0, 1);

        if(msg_id==0){
          printf("\nMSG COULDN'T BE PUBLISHED");
        }*/

        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
        if(i >= 100){
          i=0;
        }
    }

    /*for(uint32_t j=0; j <10; j++){

    }*/

   mySPI.removeDevice(mySPI.device_fpga);
   mySPI.close();
   vTaskDelay(portMAX_DELAY);
}
