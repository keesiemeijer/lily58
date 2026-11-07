#include <zmk_led_feedback.h>

// The hooks below shift the LED count down by 1 for all layers except layer 0.
// Layer 0 gets its own unique color.

// Layer 0 shows 1 LED with its unique color.
// Layer 1 shows 1 LED with normal palette color.
// Layer 2 shows 2 LEDs with normal palette color.
// Etc...

int zmk_led_feedback_layer_led_count(int layer, int count) {
    if (layer == 0) {
        return 1;
    }

    // The LED count for layers above 0 cycles from 1 to the strip length.
    return ((layer - 1) % zmk_led_feedback_strip_length()) + 1;
}

// Remove the entire next function if you're using per-layer colors
// Just add the unique color for layer 0 at the start of
// the CONFIG_ZMK_LED_FEEDBACK_LAYER_COLORS_PER_LAYER option string.

// Update the color for the default color (groups)
uint32_t zmk_led_feedback_layer_color(int layer, int led_count, uint32_t color) {
    if (layer == 0) {
        // Layer 0 has its own unique color.
        return 0xF58E27;
    }

    // Because the LED count is shifted down by 1 for layers above 0,
    // we also have the palette color index shifted down by 1 for layers above 0.
    return zmk_led_feedback_palette_color((layer - 1) / zmk_led_feedback_strip_length());
}