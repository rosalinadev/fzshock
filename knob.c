#include <furi.h>
#include <gui/view_dispatcher.h>

#include "fzshock.h"
#include "knob.h"

// Set a callback to invoke when knob has an event.
// @knob is a pointer to our Knob instance.
// @callback is a function to invoke when we have custom events.
void knob_set_callback(Knob* knob, KnobCallback callback, void* callback_context) {
    with_view_model(
        knob->view,
        KnobModel * model,
        {
            model->callback_context = callback_context;
            model->callback = callback;
        },
        true);
}

// Invoked when input (button press) is detected.
// @input_even is the event the occured.
// @ctx is a pointer to our Knob instance.
static bool knob_input_callback(InputEvent* input_event, void* ctx) {
    message("knob_input_callback");
    Knob* knob = (Knob*)ctx;

    bool handled = false;

    if(input_event->type == InputTypePress && input_event->key == InputKeyUp) {
        with_view_model(
            knob->view,
            KnobModel * model,
            {   model->strength++;
                if (model->strength > 99)
                    model->strength = 0;
            },
            true);
        handled = true;
    } else if(input_event->type == InputTypePress && input_event->key == InputKeyDown) {
        with_view_model(
            knob->view,
            KnobModel * model,
            {   model->strength--;
                if (model->strength < 0)
                    model->strength = 99;
            },
            true); // Render new data.
        handled = true;
    } else if(input_event->type == InputTypePress && input_event->key == InputKeyOk) {
        with_view_model(
            knob->view,
            KnobModel * model,
            {
                if(model->callback) {
                    message("invoking callback");
                    model->callback(model->callback_context, KnobEventDone);
                } else {
                    message("no callback set; use knob_set_callback first.");
                }
            },
            false); // No new data.
        handled = true;
    }

    return handled;
}

// Invoked by the draw callback to render the knob.
// @canvas is the canvas to draw on.
// @ctx is our model.
static void knob_render_callback(Canvas* canvas, void* ctx) {
    message("knob_render_callback");
    KnobModel* model = ctx;

    furi_string_printf(model->buffer, "%02d", model->strength);

    canvas_set_font(canvas, FontBigNumbers);

    canvas_draw_str_aligned(
        canvas, 128 / 2, 64 / 2, AlignCenter, AlignCenter, furi_string_get_cstr(model->buffer));
}

// Allocates a Knob instance.
Knob* knob_alloc() {
    message("knob_alloc");
    Knob* knob = malloc(sizeof(Knob));
    knob->view = view_alloc();

    // context passed to input_callback.
    view_set_context(knob->view, knob);

    // context passed to render.
    view_allocate_model(knob->view, ViewModelTypeLockFree, sizeof(KnobModel));
    with_view_model(
        knob->view,
        KnobModel * model,
        {
            model->buffer = furi_string_alloc();
            model->strength = 0;
        },
        true);

    view_set_draw_callback(knob->view, knob_render_callback);
    view_set_input_callback(knob->view, knob_input_callback);
    return knob;
}

// Free a Knob instance.
// @knob pointer to a Knob instance.
void knob_free(Knob* knob) {
    message("knob_free");
    furi_assert(knob);
    with_view_model(
        knob->view, KnobModel * model, { furi_string_free(model->buffer); }, true);
    view_free(knob->view);
    free(knob);
}

// Gets the view associated with our Knob.
// @knob pointer to a Knob instance.
View* knob_get_view(Knob* knob) {
    message("knob_get_view");
    furi_assert(knob);
    return knob->view;
}

// Gets the current counter value for a given Knob instance.
// @knob pointer to a Knob instance.
uint32_t knob_get_counter(Knob* knob) {
    message("knob_get_counter");
    furi_assert(knob);

    uint32_t value = 0;
    with_view_model(
        knob->view, KnobModel * model, { value = model->strength; }, false);

    return value;
}

// Set the counter value for a given Knob instance.
// @knob pointer to a Knob instance.
void knob_set_counter(Knob* knob, uint32_t count) {
    with_view_model(
        knob->view, KnobModel * model, { model->strength = count; }, true);
}
