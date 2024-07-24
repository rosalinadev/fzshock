#include <furi.h>
#include <gui/view_dispatcher.h>

#include "knob.h"
#include "subghz.h"

#define TAG "fzshock"

/////////////////////////////////////////////////////////////////
// Routine for logging messages with a delay.
/////////////////////////////////////////////////////////////////

void message(char* message) {
    FURI_LOG_I(TAG, message);
    furi_delay_ms(10);
}

/////////////////////////////////////////////////////////////////
// This is our application.
/////////////////////////////////////////////////////////////////

typedef struct App {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Knob* knob;
    float volume;
    float frequency;
} App;

typedef enum {
    AppSceneSetVolume,
    AppSceneSetFrequency,
    AppSceneNum,
} appScene;

typedef enum {
    AppViewSetCounter = 200,
} appViews;

typedef enum {
    AppKnobEventDone,
} AppKnobCustomEvents;

void app_knob_callback(void* context, uint32_t index) {
    furi_assert(context);
    App* app = context;
    UNUSED(index);
    message("app_knob_callback");

    send_payload(knob_get_counter(app->knob));
}

bool app_scene_start_custom_callback(void* context, uint32_t custom_event) {
    message("app_scene_start_custom_callback");
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

bool app_back_event_callback(void* context) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

void app_scene_set_volume_on_enter(void* context) {
    message("app_scene_set_volume_on_enter");
    App* app = context;

    Knob* knob = app->knob;
    knob_set_callback(knob, app_knob_callback, app);
    knob_set_counter(knob, (uint32_t)app->volume);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewSetCounter);
}

bool app_scene_set_volume_on_event(void* context, SceneManagerEvent event) {
    message("app_scene_set_volume_on_event");
    App* app = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeBack) {
        // Back button pressed
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == AppKnobEventDone) {
            FURI_LOG_I(TAG, "Custom Event!");
            uint32_t counter = knob_get_counter(app->knob);
            FURI_LOG_I(TAG, "The counter is %ld.", counter);
            app->volume = counter;

            scene_manager_next_scene(app->scene_manager, AppSceneSetFrequency);
        }
    }
    return consumed;
}

void app_scene_set_volume_on_exit(void* context) {
    message("app_scene_set_volume_on_exit");
    UNUSED(context);
}

void app_scene_set_frequency_on_enter(void* context) {
    message("app_scene_set_frequency_on_enter");
    App* app = context;

    Knob* knob = app->knob;
    knob_set_callback(knob, app_knob_callback, app);
    knob_set_counter(knob, (uint32_t)app->frequency);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppViewSetCounter);
}

bool app_scene_set_frequency_on_event(void* context, SceneManagerEvent event) {
    message("app_scene_set_frequency_on_event");
    App* app = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeBack) {
        // Back button pressed
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == AppKnobEventDone) {
            FURI_LOG_I(TAG, "Custom Event!");
            uint32_t counter = knob_get_counter(app->knob);
            FURI_LOG_I(TAG, "The counter is %ld.", counter);
            app->frequency = counter;
            scene_manager_next_scene(app->scene_manager, AppSceneSetVolume);
        }
    }
    return consumed;
}

void app_scene_set_frequency_on_exit(void* context) {
    message("app_scene_set_frequency_on_exit");
    UNUSED(context);
}

void (*const app_on_enter_handlers[])(void*) = {
    app_scene_set_volume_on_enter,
    app_scene_set_frequency_on_enter,
};

bool (*const app_on_event_handlers[])(void* context, SceneManagerEvent event) = {
    app_scene_set_volume_on_event,
    app_scene_set_frequency_on_event,
};

void (*const app_on_exit_handlers[])(void* context) = {
    app_scene_set_volume_on_exit,
    app_scene_set_frequency_on_exit,
};

const SceneManagerHandlers app_scene_handlers = {
    .on_enter_handlers = app_on_enter_handlers,
    .on_event_handlers = app_on_event_handlers,
    .on_exit_handlers = app_on_exit_handlers,
    .scene_num = AppSceneNum,
};

App* fzshock_app_alloc() {
    message("fzshock_app_alloc");
    App* app = malloc(sizeof(App));
    app->frequency = 440;
    app->volume = 10;
    app->scene_manager = scene_manager_alloc(&app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, app_scene_start_custom_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, app_back_event_callback);
    app->knob = knob_alloc();
    view_dispatcher_add_view(app->view_dispatcher, AppViewSetCounter, knob_get_view(app->knob));

    return app;
}

void fzshock_app_free(App* app) {
    message("knob_app_free");
    view_dispatcher_remove_view(app->view_dispatcher, AppViewSetCounter);
    knob_free(app->knob);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    free(app);
}

int32_t fzshock_app(void* p) {
    UNUSED(p);
    message("knob_demo_app");

    App* app = fzshock_app_alloc();
    message("knob_app_alloc");

    Gui* gui = furi_record_open(RECORD_GUI);

    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    message("view_dispatcher_attach_to_gui");

    scene_manager_next_scene(app->scene_manager, AppSceneSetVolume);
    message("scene_manager_next_scene");

    view_dispatcher_run(app->view_dispatcher);
    message("view_dispatcher_run");

    // Free resources
    message("Freeing resources...");
    fzshock_app_free(app);
    furi_record_close(RECORD_GUI);

    return 0;
}
