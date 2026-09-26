#include <assert.h>
#include <string.h>

#include "blu2usb/ux_model/ux_model.h"

typedef struct {
    blu2usb_screen_id_t id;
    const char *title;
} screen_expectation_t;

static const screen_expectation_t expected[] = {
    {BLU2USB_SCREEN_HOME, "UNKNOWN MOUSE"},
    {BLU2USB_SCREEN_HELP_HOME_CONNECTED, "REMOVE CONNECTED HELP"},
    {BLU2USB_SCREEN_HOME_SEARCHING, "SEARCHING SAVED MOUSE"},
    {BLU2USB_SCREEN_HOME_SEARCHING_HELP, "HOME SEARCHING HELP"},
    {BLU2USB_SCREEN_HOME_RETRY, "DEVICE NOT FOUND"},
    {BLU2USB_SCREEN_HOME_RETRY_HELP, "HOME RETRY HELP"},
    {BLU2USB_SCREEN_MOUSE_OPTIONS, "REMAPPING OPTIONS"},
    {BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS, "REMAPPER OPTIONS HELP"},
    {BLU2USB_SCREEN_PAIR_MOUSE, "PAIR NEW MOUSE"},
    {BLU2USB_SCREEN_HELP_PAIR_NEW, "PAIR NEW DEVICE HELP"},
    {BLU2USB_SCREEN_RETRY_PAIR_NEW, "NEW MOUSE NOT FOUND"},
    {BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW, "MOUSE NOT FOUND HELP"},
    {BLU2USB_SCREEN_MOUSE_SAVED, "FIRST MOUSE CONNECTED"},
    {BLU2USB_SCREEN_APPLY_PASSTHROUGH, "APPLY PASSTHROUGH"},
    {BLU2USB_SCREEN_PASSTHROUGH_APPLIED, "PASSTHROUGH ACTIVE"},
    {BLU2USB_SCREEN_APPLY_DEFAULT, "APPLY STANDARD REMAP"},
    {BLU2USB_SCREEN_DEFAULT_APPLIED, "STANDARD REMAP ACTIVE"},
    {BLU2USB_SCREEN_APPLY_ESCAPE, "APPLY ESCAPE REMAP"},
    {BLU2USB_SCREEN_ESCAPE_APPLIED, "ESCAPE APPLIED ACTIVE"},
    {BLU2USB_SCREEN_EDIT_CUSTOM, "EDIT CUSTOM REMAP"},
    {BLU2USB_SCREEN_LEFT_WILL_BECOME, "LEFT WILL BECOME"},
    {BLU2USB_SCREEN_RIGHT_WILL_BECOME, "RIGHT WILL BECOME"},
    {BLU2USB_SCREEN_MIDDLE_WILL_BECOME, "MIDDLE WILL BECOME"},
    {BLU2USB_SCREEN_FORWARD_WILL_BECOME, "FORWARD WILL BECOME"},
    {BLU2USB_SCREEN_BACKWARD_WILL_BECOME, "BACKWARD WILL BECOME"},
    {BLU2USB_SCREEN_SAVED_DEVICES, "0 OF 0"},
    {BLU2USB_SCREEN_REMOVE_THIS, "REMOVE THIS MOUSE"},
    {BLU2USB_SCREEN_HELP_REMOVE_THIS, "REMOVE MOUSE HELP"},
    {BLU2USB_SCREEN_SEARCHING_FIRST, "SEARCHING FIRST MOUSE"},
};

_Static_assert(BLU2USB_SCREEN_COUNT == 29,
               "HOPE-31 final inventory must contain exactly 29 screens");
_Static_assert(sizeof(expected) / sizeof(expected[0]) == 29,
               "HOPE-31 expected-title inventory must contain 29 screens");

static void test_exact_inventory(void)
{
    bool seen[BLU2USB_SCREEN_COUNT] = {false};

    for (unsigned i = 0u; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        assert((unsigned)expected[i].id < BLU2USB_SCREEN_COUNT);
        assert(!seen[expected[i].id]);
        seen[expected[i].id] = true;

        const blu2usb_screen_template_t *screen =
            blu2usb_ux_screen_template(expected[i].id);
        assert(screen != NULL);
        assert(screen->rows[0] != NULL);
        assert(strcmp(screen->rows[0], expected[i].title) == 0);
    }

    for (unsigned id = 0u; id < BLU2USB_SCREEN_COUNT; ++id)
        assert(seen[id]);
}

static void test_final_home_counts(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);

    ux.screen = BLU2USB_SCREEN_HOME;
    assert(blu2usb_ux_option_count(&ux) == 3u);

    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;
    assert(blu2usb_ux_option_count(&ux) == 2u);

    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    assert(blu2usb_ux_option_count(&ux) == 2u);
}

static void test_searching_first_is_automatic_identity(void)
{
    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_SEARCHING_FIRST);
    assert(screen != NULL);
    assert(strcmp(screen->rows[0], "SEARCHING FIRST MOUSE") == 0);
    assert(strcmp(screen->rows[1], "PRESS TO LEARN KEYS") == 0);
}

int main(void)
{
    test_exact_inventory();
    test_final_home_counts();
    test_searching_first_is_automatic_identity();
    return 0;
}
