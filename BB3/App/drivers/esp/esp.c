/*
 * esp.c
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL    DBG_DEBUG

#include "esp.h"

#include "fc/fc.h"
#include "etc/stream.h"

#include "download/slot.h"

#include "protocol.h"
#include "scan.h"

//DMA buffer
#define ESP_DMA_BUFFER_SIZE   512
static uint8_t esp_rx_buffer[ESP_DMA_BUFFER_SIZE];

#define ESP_STREAM_BUFFER_SIZE   256
static uint8_t esp_stream_buffer[ESP_STREAM_BUFFER_SIZE];
static stream_t esp_stream;

static uint32_t esp_external_prog_timer = 0;
#define ESP_EXT_PROG_TIMEOUT    5000

static uint16_t esp_read_index = 0;

void esp_parser(uint8_t type, uint8_t * data, uint16_t len, stream_result_t res);

void esp_device_reset()
{
    GpioSetDirection(ESP_BOOT, OUTPUT, GPIO_NOPULL);
    GpioSetDirection(ESP_EN, OUTPUT, GPIO_NOPULL);

    HAL_GPIO_WritePin(ESP_EN, LOW);
    osDelay(1);
    HAL_GPIO_WritePin(ESP_BOOT, HIGH);
    osDelay(1);
    HAL_GPIO_WritePin(ESP_EN, HIGH);
    osDelay(2);

    GpioSetDirection(ESP_EN, INPUT, GPIO_PULLUP);
    GpioSetDirection(ESP_BOOT, INPUT, GPIO_NOPULL);
}

void esp_state_reset()
{
    fc.esp.mode = esp_starting;
    fc.esp.state = 0;
    fc.esp.wifi_list_cb = NULL;

    memset(fc.esp.mac_ap, 0, 6);
    memset(fc.esp.mac_sta, 0, 6);
    memset(fc.esp.mac_bt, 0, 6);

    memset(fc.esp.ip_ap, 0, 4);
    memset(fc.esp.ip_sta, 0, 4);

    memset(fc.esp.ssid, 0, PROTO_WIFI_SSID_LEN);

    download_slot_reset();
}

void esp_init()
{
	stream_init(&esp_stream, esp_stream_buffer, ESP_STREAM_BUFFER_SIZE, esp_parser);
    HAL_UART_Receive_DMA(esp_uart, esp_rx_buffer, ESP_DMA_BUFFER_SIZE);

    esp_scan_init();
    download_slot_init();

    esp_state_reset();
	esp_device_reset();
}


void esp_deinit()
{
	fc.esp.mode = esp_off;
	fc.esp.state = 0;
    HAL_UART_DeInit(esp_uart);
	HAL_GPIO_WritePin(ESP_EN, LOW);
}

void esp_enable_external_programmer(esp_mode_t prog_mode)
{
    INFO("Enabling external programmer for ESP32");

	fc.esp.mode = prog_mode;

	//Enable esp programming
	HAL_UART_DeInit(esp_uart);
	GpioSetDirection(ESP_UART_TX, INPUT, GPIO_NOPULL);
	esp_external_prog_timer = HAL_GetTick() + ESP_EXT_PROG_TIMEOUT;
}

void esp_disable_external_programmer()
{
    INFO("Disabling external programmer for ESP32");
    esp_read_index = ESP_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(esp_uart->hdmarx);
	HAL_UART_Init(esp_uart);
	esp_init();
}

void esp_parser(uint8_t type, uint8_t * data, uint16_t len, stream_result_t res)
{
    switch (res)
    {
        case(stream_res_message):
           protocol_handle(type, data, len);
        break;
        case(stream_res_dirty): //dirty string is ending with 0
        {
           INFO((char *)data);
           if (strstr(data, "DOWNLOAD_BOOT") != NULL)
           {
               esp_enable_external_programmer(esp_external_auto);
           }
        }
        break;
        case(stream_res_error):
           DUMP(data, len);
        break;

    }
}

uint16_t esp_get_waiting()
{
    uint16_t write_index = ESP_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(esp_uart->hdmarx);

    //Get number of bytes waiting in buffer
    if (esp_read_index > write_index)
    {
        return ESP_DMA_BUFFER_SIZE - esp_read_index + write_index;
    }
    else
    {
        return write_index - esp_read_index;
    }
}

uint8_t esp_read_byte()
{
    uint8_t byte = esp_rx_buffer[esp_read_index];
    esp_read_index = (esp_read_index + 1) % ESP_DMA_BUFFER_SIZE;
    return byte;
}

uint16_t esp_read_bytes(uint8_t * data, uint16_t len, uint32_t timeout)
{
    uint32_t end = HAL_GetTick() + timeout;
    uint16_t readed = 0;

    while (len > readed)
    {
        if (esp_get_waiting() > 0)
        {
            data[readed] = esp_read_byte();
            readed++;
        }

        if (HAL_GetTick() > end)
            return readed;
    }

    return readed;
}

void esp_step()
{
    //parse the data
    if (fc.esp.mode == esp_normal || fc.esp.mode == esp_starting)
    {
        uint16_t waiting = esp_get_waiting();
        for (uint16_t i = 0; i < waiting; i++)
        {
            stream_parse(&esp_stream, esp_read_byte());
        }

        esp_scan_step();
        download_slot_step();
    }

    if (fc.esp.mode == esp_external_auto)
    {
        if (HAL_GPIO_ReadPin(ESP_UART_TX) == LOW)
        {
            esp_external_prog_timer = HAL_GetTick() + ESP_EXT_PROG_TIMEOUT;
        }
        else
        {
            if (HAL_GetTick() > esp_external_prog_timer)
            {
                esp_disable_external_programmer();
            }
        }
    }
}

