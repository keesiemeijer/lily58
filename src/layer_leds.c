/*
 * Layer 0: all LEDs orange. Layer N: N green LEDs. Shown for a short time when the layer changes.
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
    int layer = zmk_keymap_highest_layer_active();
    struct led_rgb color;
    int lit;

    if (layer == 0) {
        // Orange on all LEDs
        color = (struct led_rgb){.r = CONFIG_ZMK_LAYER_LEDS_BRIGHTNESS,
                                 .g = CONFIG_ZMK_LAYER_LEDS_BRIGHTNESS * 2 / 5,
                                 .b = 0};
        lit = STRIP_LEN;
    } else {
        color = (struct led_rgb){.r = 0, .g = CONFIG_ZMK_LAYER_LEDS_BRIGHTNESS, .b = 0};
        lit = layer;
    }

    for (int i = 0; i < STRIP_LEN; i++) {
        pixels[i] = i < lit ? color : (struct led_rgb){0};
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
