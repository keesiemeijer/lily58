#include <zmk_led_feedback.h>

// The hooks below change the default behaviour and shift the
// LED count 1 down for all layers except layer 0. (e.g., layer 2 shows 2 leds)
// Layer 0 also gets its own unique color.

// layer | LEDs | color
// ------+------+-----------------------
//   0   |  1   | 0xF58E27 (own color)
//   1-4 | 1-4  | per layer palette color of that layer (the palette repeats)

int zmk_led_feedback_layer_led_count(int layer, int count)
{
    if (layer == 0)
    {
        return 1;
    }

    // The LED count for layers above 0 cycles from 1 to the strip length.
    return ((layer - 1) % zmk_led_feedback_strip_length()) + 1;
}

uint32_t zmk_led_feedback_layer_color(int layer, int led_count, uint32_t color)
{
    if (layer == 0)
    {
        return 0xF58E27;
    }

    // Keep the color the module picked (CONFIG_ZMK_LED_FEEDBACK_LAYER_COLORS_PER_LAYER).
    return color;
}
