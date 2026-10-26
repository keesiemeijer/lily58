#include <zmk_led_feedback.h>

// Layer 2 shows 3 LEDs, all other layers keep the module's count.
int zmk_led_feedback_layer_led_count(int layer, int count)
{
    return layer == 2 ? 3 : count;
}
