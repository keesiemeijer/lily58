#include <zmk_led_feedback.h>

// Layer 0 and layer 1 show 1 LED, layer N shows N LEDs. After all LEDs are used, the next layer
// starts again at 1 LED.
int zmk_led_feedback_layer_led_count(int layer, int count)
{
    // Layer 0 shows 1 LED, all other layers show a number of LEDs based on the strip length.
    if (layer == 0)
    {
        return 1;
    }

    // For all other layers, the number of LEDs is determined by the strip length.
    return ((layer - 1) % zmk_led_feedback_strip_length()) + 1;
}

// Layer 0 uses its own color. The color changes each time all LEDs have been used (layers 1 to
// the number of LEDs use the first color).
uint32_t zmk_led_feedback_layer_color(int layer, int led_count, uint32_t color)
{
    if (layer == 0)
    {
        return 0xF5F527;
    }

    return zmk_led_feedback_palette_color((layer - 1) / zmk_led_feedback_strip_length());
}
