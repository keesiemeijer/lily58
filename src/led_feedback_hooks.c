#include <zmk_led_feedback.h>

// Layer 2 shows 3 LEDs, all other layers keep the module's count.

// int zmk_led_feedback_layer_led_count(int layer, int count)
// {
//     return layer == 2 ? 3 : count;
// }

// Layer 2 shows its LEDs in red.

// uint32_t zmk_led_feedback_layer_color(int layer, int led_count, uint32_t color)
// {
//     return layer == 2 ? 0xFF0000 : color;
// }
