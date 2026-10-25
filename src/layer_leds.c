/*
 * Lights up (layer + 1) LEDs of a strip for a short time when the layer changes.
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#define STRIP_NODE DT_CHOSEN(zmk_layer_leds)
#define STRIP_LEN DT_PROP(STRIP_NODE, chain_length)

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixels[STRIP_LEN];

static void show_handler(struct k_work *work);
static void clear_handler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(show_work, show_handler);
static K_WORK_DELAYABLE_DEFINE(clear_work, clear_handler);

static void show_handler(struct k_work *work) {
    int lit = zmk_keymap_highest_layer_active() + 1;

    for (int i = 0; i < STRIP_LEN; i++) {
        pixels[i] = (struct led_rgb){
            .r = 0, .g = i < lit ? CONFIG_ZMK_LAYER_LEDS_BRIGHTNESS : 0, .b = 0};
    }

    led_strip_update_rgb(strip, pixels, STRIP_LEN);
    k_work_reschedule(&clear_work, K_MSEC(CONFIG_ZMK_LAYER_LEDS_DURATION_MS));
}

static void clear_handler(struct k_work *work) {
    memset(pixels, 0, sizeof(pixels));
    led_strip_update_rgb(strip, pixels, STRIP_LEN);
}

static int layer_leds_listener(const zmk_event_t *eh) {
    if (device_is_ready(strip)) {
        k_work_reschedule(&show_work, K_NO_WAIT);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_leds, layer_leds_listener);
ZMK_SUBSCRIPTION(layer_leds, zmk_layer_state_changed);
