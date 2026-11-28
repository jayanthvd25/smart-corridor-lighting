// #include <stdio.h>
// #include <stdbool.h>
// #include <stdint.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "driver/gpio.h"
// #include "driver/i2c_master.h"

// #include "esp_log.h"
// #include "esp_timer.h"
// #include "esp_err.h"

// #define SDA_PIN GPIO_NUM_4
// #define SCL_PIN GPIO_NUM_5

// #define PIR_PIN GPIO_NUM_6

// #define RELAY_1 GPIO_NUM_7
// #define RELAY_2 GPIO_NUM_8

// #define BH_ADDR 0x23

// #define DARK_LUX 50.0f

// #define OFF_TIME 120000000LL
// #define SETTLE_TIME 10000000LL

// #define RELAY_ON 0
// #define RELAY_OFF 1

// static const char *TAG = "CORRIDOR";

// static i2c_master_bus_handle_t bus;
// static i2c_master_dev_handle_t bh;

// static bool light_on = false;
// static bool lux_valid = false;

// static float lux = 0;

// static int64_t last_motion = 0;
// static int64_t last_lux_read = 0;
// static int64_t settle_until = 0;


// static void relay_control(bool state)
// {
//     int v = state ? RELAY_ON : RELAY_OFF;

//     gpio_set_level(RELAY_1, v);
//     gpio_set_level(RELAY_2, v);

//     light_on = state;

//     if (state)
//     {
//         ESP_LOGI(TAG, "CORRIDOR LIGHTS ON");
//     }
//     else
//     {
//         ESP_LOGI(TAG, "CORRIDOR LIGHTS OFF");

//         settle_until = esp_timer_get_time() + SETTLE_TIME;
//     }
// }


// static void gpio_init_all(void)
// {
//     gpio_config_t out = {
//         .pin_bit_mask = (1ULL << RELAY_1) | (1ULL << RELAY_2),
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE
//     };

//     ESP_ERROR_CHECK(gpio_config(&out));

//     gpio_set_level(RELAY_1, RELAY_OFF);
//     gpio_set_level(RELAY_2, RELAY_OFF);

//     gpio_config_t in = {
//         .pin_bit_mask = (1ULL << PIR_PIN),
//         .mode = GPIO_MODE_INPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE
//     };

//     ESP_ERROR_CHECK(gpio_config(&in));
// }


// static void i2c_init_all(void)
// {
//     i2c_master_bus_config_t cfg = {
//         .i2c_port = I2C_NUM_0,
//         .sda_io_num = SDA_PIN,
//         .scl_io_num = SCL_PIN,
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .flags.enable_internal_pullup = true
//     };

//     ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));

//     i2c_device_config_t dev = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = BH_ADDR,
//         .scl_speed_hz = 100000
//     };

//     ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev, &bh));

//     uint8_t cmd = 0x10;

//     ESP_ERROR_CHECK(
//         i2c_master_transmit(bh, &cmd, 1, 1000)
//     );

//     vTaskDelay(pdMS_TO_TICKS(200));

//     ESP_LOGI(TAG, "BH1750 INITIALIZED");
// }


// static float read_lux(void)
// {
//     uint8_t data[2];

//     esp_err_t err = i2c_master_receive(
//         bh,
//         data,
//         2,
//         1000
//     );

//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "BH1750 READ FAILED");
//         lux_valid = false;
//         return 0;
//     }

//     uint16_t raw = ((uint16_t)data[0] << 8) | data[1];

//     lux_valid = true;

//     return raw / 1.2f;
// }


// static void sensor_task(void *arg)
// {
//     while (1)
//     {
//         int64_t now = esp_timer_get_time();

//         // Read ambient light only when lights are OFF.

//         if (!light_on && now >= settle_until)
//         {
//             if (now - last_lux_read >= 1000000LL)
//             {
//                 lux = read_lux();

//                 last_lux_read = now;

//                 if (lux_valid)
//                 {
//                     ESP_LOGI(TAG, "Ambient Light: %.2f lux", lux);
//                 }
//             }
//         }

//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }


// static void lighting_task(void *arg)
// {
//     while (1)
//     {
//         int motion = gpio_get_level(PIR_PIN);

//         int64_t now = esp_timer_get_time();

//         if (motion)
//         {
//             last_motion = now;

//             if (!light_on && lux_valid && lux < DARK_LUX)
//             {
//                 relay_control(true);
//             }
//         }

//         if (light_on)
//         {
//             if (now - last_motion >= OFF_TIME)
//             {
//                 relay_control(false);
//             }
//         }

//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }


// void app_main(void)
// {
//     ESP_LOGI(TAG, "SMART CORRIDOR LIGHTING SYSTEM");

//     gpio_init_all();

//     i2c_init_all();

//     settle_until = esp_timer_get_time();

//     xTaskCreate(
//         sensor_task,
//         "sensor_task",
//         4096,
//         NULL,
//         5,
//         NULL
//     );

//     xTaskCreate(
//         lighting_task,
//         "lighting_task",
//         4096,
//         NULL,
//         5,
//         NULL
//     );

//     ESP_LOGI(TAG, "SYSTEM STARTED");
// }


//BH1750 TESTING 


// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/i2c_master.h"
// #include "esp_log.h"

// #define SDA_PIN 8
// #define SCL_PIN 9

// #define BH1750_ADDR 0x23

// void app_main(void)
// {
//     i2c_master_bus_handle_t bus;
//     i2c_master_dev_handle_t sensor;

//     i2c_master_bus_config_t bus_config = {
//         .i2c_port = I2C_NUM_0,
//         .sda_io_num = SDA_PIN,
//         .scl_io_num = SCL_PIN,
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .flags.enable_internal_pullup = true
//     };

//     ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));

//     i2c_device_config_t dev_config = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = BH1750_ADDR,
//         .scl_speed_hz = 100000
//     };

//     ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_config, &sensor));

//     // Power ON
//     uint8_t cmd = 0x01;
//     ESP_ERROR_CHECK(i2c_master_transmit(sensor, &cmd, 1, -1));

//     vTaskDelay(pdMS_TO_TICKS(10));

//     // Continuous high-resolution mode
//     cmd = 0x10;
//     ESP_ERROR_CHECK(i2c_master_transmit(sensor, &cmd, 1, -1));

//     while (1)
//     {
//         uint8_t data[2];

//         esp_err_t ret = i2c_master_receive(sensor, data, 2, -1);

//         if (ret == ESP_OK)
//         {
//             int raw = (data[0] << 8) | data[1];

//             float lux = raw / 1.2;

//             printf("Light Intensity: %.2f Lux\n", lux);
//         }
//         else
//         {
//             printf("Sensor reading failed!\n");
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }



// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/i2c_master.h"
// #include "driver/gpio.h"
// #include "esp_log.h"

// #define SDA_PIN 8
// #define SCL_PIN 9

// #define PIR_PIN 5
// #define LED_PIN 4

// #define BH1750_ADDR 0x23

// #define LUX_THRESHOLD 50.0
// #define ON_TIME 120000

// void app_main(void)
// {
//     i2c_master_bus_handle_t bus;
//     i2c_master_dev_handle_t sensor;

//     // Configure I2C
//     i2c_master_bus_config_t bus_config = {
//         .i2c_port = I2C_NUM_0,
//         .sda_io_num = SDA_PIN,
//         .scl_io_num = SCL_PIN,
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .flags.enable_internal_pullup = true
//     };

//     ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));

//     i2c_device_config_t dev_config = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = BH1750_ADDR,
//         .scl_speed_hz = 100000
//     };

//     ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_config, &sensor));

//     // Configure PIR and LED
//     gpio_reset_pin(PIR_PIN);
//     gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

//     gpio_reset_pin(LED_PIN);
//     gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

//     gpio_set_level(LED_PIN, 0);

//     // Initialize BH1750
//     uint8_t cmd = 0x01;
//     ESP_ERROR_CHECK(i2c_master_transmit(sensor, &cmd, 1, -1));

//     vTaskDelay(pdMS_TO_TICKS(10));

//     cmd = 0x10;
//     ESP_ERROR_CHECK(i2c_master_transmit(sensor, &cmd, 1, -1));

//     printf("Smart Corridor Lighting Started\n");

//     TickType_t last_motion = 0;
//     int led_state = 0;

//     while (1)
//     {
//         uint8_t data[2];

//         esp_err_t ret = i2c_master_receive(sensor, data, 2, -1);

//         if (ret == ESP_OK)
//         {
//             int raw = (data[0] << 8) | data[1];

//             float lux = raw / 1.2;

//             int motion = gpio_get_level(PIR_PIN);

//             printf("Lux: %.2f | Motion: %d\n", lux, motion);

//             if (lux < LUX_THRESHOLD && motion == 1)
//             {
//                 last_motion = xTaskGetTickCount();
//             }

//             if (lux < LUX_THRESHOLD &&
//                 (xTaskGetTickCount() - last_motion) < pdMS_TO_TICKS(ON_TIME) &&
//                 last_motion != 0)
//             {
//                 gpio_set_level(LED_PIN, 1);
//                 led_state = 1;
//             }
//             else
//             {
//                 gpio_set_level(LED_PIN, 0);
//                 led_state = 0;
//             }

//             printf("LED: %s\n", led_state ? "ON" : "OFF");
//         }
//         else
//         {
//             printf("BH1750 reading failed!\n");
//             gpio_set_level(LED_PIN, 0);
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

//PIR TESTING

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"

// #define PIR_PIN GPIO_NUM_1

// void app_main(void)
// {
//     gpio_reset_pin(PIR_PIN);
//     gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

//     printf("\nPIR Sensor Initializing...\n");

//     vTaskDelay(pdMS_TO_TICKS(60000));

//     printf("\nPIR Sensor Ready!\n");

//     while (1)
//     {
//         int state = gpio_get_level(PIR_PIN);

//         if (state == 1)
//         {
//             printf("PIR STATUS: HIGH | Motion Detected\n");
//         }
//         else
//         {
//             printf("PIR STATUS: LOW | No Motion\n");
//         }

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

//LDR TESTING FINAL 


// #include <stdio.h>
// #include <stdint.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "driver/i2c_master.h"

// #include "esp_log.h"
// #include "esp_err.h"

// #define I2C_MASTER_SCL_IO GPIO_NUM_4
// #define I2C_MASTER_SDA_IO GPIO_NUM_5

// #define I2C_MASTER_NUM I2C_NUM_0
// #define I2C_MASTER_FREQ_HZ 100000

// #define BH1750_ADDR 0x23

// static const char *TAG = "BH1750";

// void app_main(void)
// {
//     i2c_master_bus_handle_t bus_handle;
//     i2c_master_dev_handle_t dev_handle;

//     // I2C Bus Configuration
//     i2c_master_bus_config_t bus_config = {
//         .i2c_port = I2C_MASTER_NUM,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .flags.enable_internal_pullup = true,
//     };

//     ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

//     // BH1750 Configuration
//     i2c_device_config_t dev_config = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = BH1750_ADDR,
//         .scl_speed_hz = I2C_MASTER_FREQ_HZ,
//     };

//     ESP_ERROR_CHECK(
//         i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle)
//     );

//     ESP_LOGI(TAG, "I2C initialized");

//     // Check Sensor Connection
//     esp_err_t ret = i2c_master_probe(bus_handle, BH1750_ADDR, 1000);

//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "BH1750 NOT DETECTED!");
//         return;
//     }

//     ESP_LOGI(TAG, "BH1750 detected successfully!");

//     // Power ON
//     uint8_t command = 0x01;

//     ESP_ERROR_CHECK(
//         i2c_master_transmit(dev_handle, &command, 1, 1000)
//     );

//     // Continuous High Resolution Mode
//     command = 0x10;

//     ESP_ERROR_CHECK(
//         i2c_master_transmit(dev_handle, &command, 1, 1000)
//     );

//     ESP_LOGI(TAG, "Starting light measurement...");

//     while (1)
//     {
//         uint8_t data[2];

//         vTaskDelay(pdMS_TO_TICKS(200));

//         ret = i2c_master_receive(dev_handle, data, 2, 1000);

//         if (ret == ESP_OK)
//         {
//             uint16_t raw_value =
//                 ((uint16_t)data[0] << 8) | data[1];

//             float lux = raw_value / 1.2f;

//             ESP_LOGI(TAG, "Raw: %u | Light Intensity: %.2f lux",
//                      raw_value, lux);
//         }
//         else
//         {
//             ESP_LOGE(TAG, "Reading failed: %s", esp_err_to_name(ret));
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

//PIR FINAL
// #include <stdio.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "driver/gpio.h"

// #include "esp_log.h"
// #include "esp_err.h"

// #define PIR_GPIO GPIO_NUM_1

// static const char *TAG = "PIR_SENSOR";

// void app_main(void)
// {
//     // Configure PIR GPIO
//     gpio_config_t pir_config = {
//         .pin_bit_mask = (1ULL << PIR_GPIO),
//         .mode = GPIO_MODE_INPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE
//     };

//     ESP_ERROR_CHECK(gpio_config(&pir_config));

//     ESP_LOGI(TAG, "PIR Sensor Initialized");
//     ESP_LOGI(TAG, "Waiting for sensor stabilization...");

//     // Allow PIR to stabilize
//     vTaskDelay(pdMS_TO_TICKS(30000));

//     ESP_LOGI(TAG, "PIR Sensor Ready!");

//     while (1)
//     {
//         int motion = gpio_get_level(PIR_GPIO);

//         if (motion == 1)
//         {
//             ESP_LOGI(TAG, "MOTION DETECTED");
//         }
//         else
//         {
//             ESP_LOGI(TAG, "NO MOTION");
//         }

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

//RELAY FINAL TESTING 

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

// Relay GPIO pins
#define RELAY1_GPIO GPIO_NUM_10
#define RELAY2_GPIO GPIO_NUM_11

// Active LOW relay logic
#define RELAY_ON  0
#define RELAY_OFF 1

static const char *TAG = "RELAY_TEST";

void app_main(void)
{
    // Configure relay GPIO pins as outputs
    gpio_config_t relay_config = {
        .pin_bit_mask = (1ULL << RELAY1_GPIO) |
                        (1ULL << RELAY2_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&relay_config));

    // Initially turn OFF both relays
    ESP_ERROR_CHECK(gpio_set_level(RELAY1_GPIO, RELAY_OFF));
    ESP_ERROR_CHECK(gpio_set_level(RELAY2_GPIO, RELAY_OFF));

    ESP_LOGI(TAG, "2-Channel Relay Test Started");

    while (1)
    {
        // STEP 1: Relay 1 ON
        ESP_LOGI(TAG, "Relay 1 ON | Relay 2 OFF");

        gpio_set_level(RELAY1_GPIO, RELAY_ON);
        gpio_set_level(RELAY2_GPIO, RELAY_OFF);

        vTaskDelay(pdMS_TO_TICKS(3000));

        // STEP 2: Relay 2 ON
        ESP_LOGI(TAG, "Relay 1 OFF | Relay 2 ON");

        gpio_set_level(RELAY1_GPIO, RELAY_OFF);
        gpio_set_level(RELAY2_GPIO, RELAY_ON);

        vTaskDelay(pdMS_TO_TICKS(3000));

        // STEP 3: Both relays OFF
        ESP_LOGI(TAG, "Both Relays OFF");

        gpio_set_level(RELAY1_GPIO, RELAY_OFF);
        gpio_set_level(RELAY2_GPIO, RELAY_OFF);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}