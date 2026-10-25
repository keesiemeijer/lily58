/*
 * Layer 0: all LEDs orange. Layer N: N green LEDs. Shown for a short time when the layer changes.
 * Blinking red LEDs while a half is disconnected: 1 = left, 2 = right, 3 = both.
 */

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/drivers/led_strip.h>

#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#define STRIP_NODE DT_CHOSEN(zmk_layer_leds)
#define STRIP_LEN DT_PROP(STRIP_NODE, chain_length)
#define BLINK_INTERVAL_MS 500

#define LEFT_SLOT CONFIG_ZMK_LAYER_LEDS_LEFT_SLOT
#define RIGHT_SLOT (1 - LEFT_SLOT)

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixels[STRIP_LEN];

static atomic_t slot_connected[2];
static bool layer_active;
static bool blink_on;

static void show_handler(struct k_work *work);
static void clear_handler(struct k_work *work);
static void blink_handler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(show_work, show_handler);
static K_WORK_DELAYABLE_DEFINE(clear_work, clear_handler);
static K_WORK_DELAYABLE_DEFINE(blink_work, blink_handler);

static void update_strip(int lit, struct led_rgb color) {
    for (int i = 0; i < STRIP_LEN; i++) {
        pixels[i] = i < lit ? color : (struct led_rgb){0};
    }

    led_strip_update_rgb(strip, pixels, STRIP_LEN);
}

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

    layer_active = true;
    update_strip(lit, color);
    k_work_reschedule(&clear_work, K_MSEC(CONFIG_ZMK_LAYER_LEDS_DURATION_MS));
}

static void clear_handler(struct k_work *work) {
    layer_active = false;
    update_strip(0, (struct led_rgb){0});
    k_work_reschedule(&blink_work, K_MSEC(BLINK_INTERVAL_MS));
}

static void blink_handler(struct k_work *work) {
    if (!device_is_ready(strip)) {
        k_work_reschedule(&blink_work, K_MSEC(BLINK_INTERVAL_MS));
        return;
    }

    bool left_missing = !atomic_get(&slot_connected[LEFT_SLOT]);
    bool right_missing = !atomic_get(&slot_connected[RIGHT_SLOT]);
    int missing = left_missing ? (right_missing ? 3 : 1) : (right_missing ? 2 : 0);

    // The layer indication has priority, it restarts the blinking when it is done.
    if (layer_active) {
        return;
    }

    blink_on = missing > 0 && !blink_on;
    update_strip(blink_on ? missing : 0,
                 (struct led_rgb){.r = CONFIG_ZMK_LAYER_LEDS_BRIGHTNESS, .g = 0, .b = 0});
    k_work_reschedule(&blink_work, K_MSEC(BLINK_INTERVAL_MS));
}

static void set_connected(struct bt_conn *conn, bool connected) {
    struct bt_conn_info info;

    if (bt_conn_get_info(conn, &info) < 0 || info.role != BT_CONN_ROLE_CENTRAL) {
        return;
    }

    int slot = zmk_ble_put_peripheral_addr(bt_conn_get_dst(conn));
    if (slot >= 0 && slot < 2) {
        atomic_set(&slot_connected[slot], connected);
    }
}

static void layer_leds_connected(struct bt_conn *conn, uint8_t err) {
    if (!err) {
        set_connected(conn, true);
    }
}

static void layer_leds_disconnected(struct bt_conn *conn, uint8_t reason) {
    set_connected(conn, false);
}

BT_CONN_CB_DEFINE(layer_leds_conn_cb) = {
    .connected = layer_leds_connected,
    .disconnected = layer_leds_disconnected,
};

static int layer_leds_listener(const zmk_event_t *eh) {
    if (device_is_ready(strip)) {
        k_work_reschedule(&show_work, K_NO_WAIT);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_leds, layer_leds_listener);
ZMK_SUBSCRIPTION(layer_leds, zmk_layer_state_changed);

static int layer_leds_init(void) {
    k_work_reschedule(&blink_work, K_MSEC(BLINK_INTERVAL_MS));
    return 0;
}

SYS_INIT(layer_leds_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
