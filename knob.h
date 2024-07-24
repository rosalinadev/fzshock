#include <furi.h>
#include <gui/view_dispatcher.h>

typedef enum {
    KnobEventDone,
} KnobCustomEvents;

typedef void (*KnobCallback)(void* context, uint32_t index);

typedef struct Knob {
    View* view;
} Knob;

typedef struct KnobModel {
    FuriString* buffer;
    int8_t strength;
    // uint16_t counter;
    // char* heading;
    KnobCallback callback;
    void* callback_context;
} KnobModel;

void knob_set_callback(Knob* knob, KnobCallback callback, void* callback_context);
Knob* knob_alloc();
void knob_free(Knob* knob);
View* knob_get_view(Knob* knob);
uint32_t knob_get_counter(Knob* knob);
void knob_set_counter(Knob* knob, uint32_t count);
