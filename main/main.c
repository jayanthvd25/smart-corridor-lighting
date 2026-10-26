// #include <stdio.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// #define LED GPIO_NUM_4
// #define PIR GPIO_NUM_5
// #define LDR GPIO_NUM_6

// #define LIGHT_ON_LEVEL 1
// #define DARK_LEVEL 1

// #define ON_TIME_US (120LL * 1000000LL)

// static int sec = 18 * 3600;
// static int64_t last_clock_us;
// static int64_t last_motion_us = -1;

// static void update_clock(void)
// {
//     int64_t now = esp_timer_get_time();
//     int64_t elapsed = (now - last_clock_us) / 1000000LL;

//     if (elapsed > 0) {
//         sec = (sec + (int)(elapsed % 86400)) % 86400;
//         last_clock_us += elapsed * 1000000LL;
//     }
// }

// static bool active_hours(void)
// {
//     int h = sec / 3600;
//     return h >= 18 || h < 5;
// }

// static void print_time(void)
// {
//     int h = sec / 3600;
//     int m = (sec % 3600) / 60;
//     int s = sec % 60;

//     printf("Time: %02d:%02d:%02d | ", h, m, s);
// }

// void app_main(void)
// {
//     gpio_reset_pin(LED);
//     gpio_set_direction(LED, GPIO_MODE_OUTPUT);
//     gpio_set_level(LED, 0);

//     gpio_reset_pin(PIR);
//     gpio_set_direction(PIR, GPIO_MODE_INPUT);

//     gpio_reset_pin(LDR);
//     gpio_set_direction(LDR, GPIO_MODE_INPUT);

//     last_clock_us = esp_timer_get_time();

//     printf("\nSmart Corridor Lighting System Started\n");
//     printf("Initial software time: 18:00:00\n");

//     while (1)
//     {
//         update_clock();

//         int p = gpio_get_level(PIR);
//         int l = gpio_get_level(LDR);
//         int64_t now = esp_timer_get_time();

//         bool night = active_hours();
//         bool dark = (l == DARK_LEVEL);
//         bool motion = (p == 1);

//         if (night && dark && motion) {
//             last_motion_us = now;
//         }

//         bool timer_active =
//             last_motion_us >= 0 &&
//             (now - last_motion_us) < ON_TIME_US;

//         bool light_on = night && dark && timer_active;

//         gpio_set_level(LED, light_on ? LIGHT_ON_LEVEL : !LIGHT_ON_LEVEL);

//         print_time();
//         printf("LDR DO: %d | PIR: %d | Night: %d | Dark: %d | Light: %s\n",
//                l, p, night, dark, light_on ? "ON" : "OFF");

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

// #include <stdio.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// #define LED GPIO_NUM_4
// #define PIR GPIO_NUM_5
// #define LDR GPIO_NUM_6

// // Adjust to match your hardware
// #define LIGHT_ON_LEVEL 1   // LED/relay output level that turns the light ON
// #define DARK_LEVEL     1   // LDR module DO level when it is dark

// #define ON_TIME_US      (120LL * 1000000LL)  // light stays on 2 min after last motion
// #define PIR_WARMUP_US   (60LL * 1000000LL)   // ignore PIR for 60 s after boot
// #define LOG_PERIOD_US   (5LL * 1000000LL)    // heartbeat log every 5 s

// #define START_TIME_SEC  (18 * 3600)          // software clock starts at 18:00:00

// static int sec = START_TIME_SEC;
// static int64_t last_clock_us;

// static void update_clock(void)
// {
//     int64_t now = esp_timer_get_time();
//     int64_t elapsed = (now - last_clock_us) / 1000000LL;

//     if (elapsed > 0) {
//         sec = (sec + (int)(elapsed % 86400)) % 86400;
//         last_clock_us += elapsed * 1000000LL;
//     }
// }

// static bool active_hours(void)
// {
//     int h = sec / 3600;
//     return h >= 18 || h < 5;   // 18:00 - 05:00
// }

// static void print_time(void)
// {
//     int h = sec / 3600;
//     int m = (sec % 3600) / 60;
//     int s = sec % 60;

//     printf("Time: %02d:%02d:%02d | ", h, m, s);
// }

// void app_main(void)
// {
//     gpio_reset_pin(LED);
//     gpio_set_direction(LED, GPIO_MODE_OUTPUT);
//     gpio_set_level(LED, !LIGHT_ON_LEVEL);

//     gpio_reset_pin(PIR);
//     gpio_set_direction(PIR, GPIO_MODE_INPUT);

//     gpio_reset_pin(LDR);
//     gpio_set_direction(LDR, GPIO_MODE_INPUT);

//     int64_t start_us = esp_timer_get_time();
//     last_clock_us = start_us;

//     bool light_on = false;
//     int64_t last_motion_us = 0;

//     // For change-based logging
//     int prev_p = -1, prev_l = -1;
//     bool prev_night = false, prev_light = false;
//     int64_t last_log_us = 0;

//     printf("\nSmart Corridor Lighting System Started\n");
//     printf("Initial software time: 18:00:00\n");
//     printf("PIR warm-up: ignoring motion for %d s\n", (int)(PIR_WARMUP_US / 1000000LL));

//     while (1)
//     {
//         update_clock();

//         int p = gpio_get_level(PIR);
//         int l = gpio_get_level(LDR);
//         int64_t now = esp_timer_get_time();

//         bool night  = active_hours();
//         bool dark   = (l == DARK_LEVEL);
//         bool warmed = (now - start_us) >= PIR_WARMUP_US;
//         bool motion = (p == 1) && warmed;

//         if (!light_on) {
//             // Turn on only when it's night, dark, and motion is detected
//             if (night && dark && motion) {
//                 light_on = true;
//                 last_motion_us = now;
//             }
//         } else {
//             // While ON: ignore the LDR (the LED lights the sensor itself).
//             // Motion retriggers the timer; timeout or end of night turns it off.
//             if (motion) {
//                 last_motion_us = now;
//             }
//             if (!night || (now - last_motion_us) >= ON_TIME_US) {
//                 light_on = false;
//             }
//         }

//         gpio_set_level(LED, light_on ? LIGHT_ON_LEVEL : !LIGHT_ON_LEVEL);

//         bool changed = (p != prev_p) || (l != prev_l) ||
//                        (night != prev_night) || (light_on != prev_light);

//         if (changed || (now - last_log_us) >= LOG_PERIOD_US) {
//             print_time();
//             printf("LDR DO: %d | PIR: %d | Night: %d | Dark: %d | Warmed: %d | Light: %s\n",
//                    l, p, night, dark, warmed, light_on ? "ON" : "OFF");

//             prev_p = p;
//             prev_l = l;
//             prev_night = night;
//             prev_light = light_on;
//             last_log_us = now;
//         }

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }


// #include <stdio.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// #define LED GPIO_NUM_4
// #define PIR GPIO_NUM_5
// #define LDR GPIO_NUM_6

// #define LIGHT_ON_LEVEL 1
// #define DARK_LEVEL 1

// #define ON_TIME_US (120LL * 1000000LL)
// #define PIR_WARMUP_US (60LL * 1000000LL)
// #define LOG_PERIOD_US (5LL * 1000000LL)

// void app_main(void)
// {
// gpio_reset_pin(LED);
// gpio_set_direction(LED, GPIO_MODE_OUTPUT);
// gpio_set_level(LED, !LIGHT_ON_LEVEL);

// gpio_reset_pin(PIR);
// gpio_set_direction(PIR, GPIO_MODE_INPUT);
// gpio_reset_pin(LDR);
// gpio_set_direction(LDR, GPIO_MODE_INPUT);
// int64_t start_us = esp_timer_get_time();
// bool light_on = false;
// int64_t last_motion_us = 0;
// int prev_p = -1, prev_l = -1;
// bool prev_light = false;
// int64_t last_log_us = 0;
// printf("\nSmart Corridor Lighting System Started\n");
// printf("PIR warm-up: ignoring motion for %d s\n",
//        (int)(PIR_WARMUP_US / 1000000LL));
// while (1)
// {
//     int p = gpio_get_level(PIR);
//     int l = gpio_get_level(LDR);
//     int64_t now = esp_timer_get_time();
//     bool dark = (l == DARK_LEVEL);
//     bool warmed = (now - start_us) >= PIR_WARMUP_US;
//     bool motion = (p == 1) && warmed;
//     if (!light_on)
//     {
//         if (dark && motion)
//         {
//             light_on = true;
//             last_motion_us = now;
//         }
//     }
//     else
//     {
//         if (motion)
//         {
//             last_motion_us = now;
//         }
//         if ((now - last_motion_us) >= ON_TIME_US)
//         {
//             light_on = false;
//         }
//     }
//     gpio_set_level(LED, light_on ? LIGHT_ON_LEVEL : !LIGHT_ON_LEVEL);
//     bool changed = (p != prev_p) || (l != prev_l) ||
//                    (light_on != prev_light);
//     if (changed || (now - last_log_us) >= LOG_PERIOD_US)
//     {
//         printf("LDR DO: %d | PIR: %d | Dark: %d | Warmed: %d | Light: %s\n",
//                l, p, dark, warmed, light_on ? "ON" : "OFF");
//         prev_p = p;
//         prev_l = l;
//         prev_light = light_on;
//         last_log_us = now;
//     }
//     vTaskDelay(pdMS_TO_TICKS(500));
// }

// }


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PIR GPIO_NUM_5
#define LDR GPIO_NUM_6

void app_main(void)
{
gpio_reset_pin(PIR);
gpio_set_direction(PIR, GPIO_MODE_INPUT);

gpio_reset_pin(LDR);
gpio_set_direction(LDR, GPIO_MODE_INPUT);
while (1)
{
    int p = gpio_get_level(PIR);
    int l = gpio_get_level(LDR);
    printf("PIR: %d | LDR DO: %d | Dark: %d\n",
           p, l, l == 1);
    vTaskDelay(pdMS_TO_TICKS(500));
}

}