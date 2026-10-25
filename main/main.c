#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LED GPIO_NUM_4
#define PIR GPIO_NUM_5
#define LDR GPIO_NUM_6

#define LIGHT_ON_LEVEL 1
#define DARK_LEVEL 1

#define ON_TIME_US (120LL * 1000000LL)

static int sec = 18 * 3600;
static int64_t last_clock_us;
static int64_t last_motion_us = -1;

static void update_clock(void)
{
    int64_t now = esp_timer_get_time();
    int64_t elapsed = (now - last_clock_us) / 1000000LL;

    if (elapsed > 0) {
        sec = (sec + (int)(elapsed % 86400)) % 86400;
        last_clock_us += elapsed * 1000000LL;
    }
}

static bool active_hours(void)
{
    int h = sec / 3600;
    return h >= 18 || h < 5;
}

static void print_time(void)
{
    int h = sec / 3600;
    int m = (sec % 3600) / 60;
    int s = sec % 60;

    printf("Time: %02d:%02d:%02d | ", h, m, s);
}

void app_main(void)
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);

    gpio_reset_pin(PIR);
    gpio_set_direction(PIR, GPIO_MODE_INPUT);

    gpio_reset_pin(LDR);
    gpio_set_direction(LDR, GPIO_MODE_INPUT);

    last_clock_us = esp_timer_get_time();

    printf("\nSmart Corridor Lighting System Started\n");
    printf("Initial software time: 18:00:00\n");

    while (1)
    {
        update_clock();

        int p = gpio_get_level(PIR);
        int l = gpio_get_level(LDR);
        int64_t now = esp_timer_get_time();

        bool night = active_hours();
        bool dark = (l == DARK_LEVEL);
        bool motion = (p == 1);

        if (night && dark && motion) {
            last_motion_us = now;
        }

        bool timer_active =
            last_motion_us >= 0 &&
            (now - last_motion_us) < ON_TIME_US;

        bool light_on = night && dark && timer_active;

        gpio_set_level(LED, light_on ? LIGHT_ON_LEVEL : !LIGHT_ON_LEVEL);

        print_time();
        printf("LDR DO: %d | PIR: %d | Night: %d | Dark: %d | Light: %s\n",
               l, p, night, dark, light_on ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}