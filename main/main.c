#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Handle สำหรับ task ทั้งสอง
TaskHandle_t MyFirstTaskHandle = NULL;
TaskHandle_t MySecondTaskHandle = NULL;
TaskHandle_t ISR = NULL;

#define LED_PIN 27
#define PUSH_BUTTON_PIN 33

// Task แรก
void interrupt_task(void *arg) {
    bool led_status = false;
    while (1) {
        vTaskSuspend(NULL);
        led_status = !led_status;
        gpio_set_level(LED_PIN, led_status);
        printf("Button pressed!\n");
    }
}

void IRAM_ATTR button_isr_handler(void *arg) {
    xTaskResumeFromISR(ISR);
}

void My_First_Task(void *arg) {
    uint16_t i = 0;
    while (1) {
        printf("Hello My First Task %d\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;

        // Suspend task ที่สองเมื่อ i เท่ากับ 5
        if (i == 5) {
            vTaskSuspend(MySecondTaskHandle);
            printf("Second Task suspended\n");
        }

        // Resume task ที่สองเมื่อ i เท่ากับ 10
        if (i == 10) {
            vTaskResume(MySecondTaskHandle);
            printf("Second Task resumed\n");
        }

        // ลบ task ที่สองเมื่อ i เท่ากับ 15
        if (i == 15) {
            vTaskDelete(MySecondTaskHandle);
            printf("Second Task deleted\n");
        }

        // Suspend ตัวเองเมื่อ i เท่ากับ 20
        if (i == 20) {
            printf("MyFirstTaskHandle will suspend itself\n");
            vTaskSuspend(NULL); // NULL จะหมายถึง task ปัจจุบัน
        }
    }
}

// Task ที่สอง
void My_Second_Task(void *arg) {
    uint16_t i = 0;
    while (1) {
        printf("Hello My Second Task %d\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
    }
}

// ฟังก์ชันหลักของโปรแกรม
void app_main(void) {
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    esp_rom_gpio_pad_select_gpio(LED_PIN);

    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(PUSH_BUTTON_PIN, button_isr_handler, NULL);

    xTaskCreate(interrupt_task, "interrupt_task", 4096, NULL, 10, &ISR);

}