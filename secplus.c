#include <stdio.h>
#include <furi.h>
#include <furi_hal_power.h>
#include <secplus_icons.h>
#include "secplus.h"
#include "storage.h"
#include "transmission.h"

SecPlusApp* app;

typedef enum { SecPlusScene_Main } SecPlusScene;

typedef enum { SecPlusView_Main } SecPlusView;

typedef enum { SecPlusEvent_Transmit } SecPlusEvent;

void secplus_app_menu_callback_main(void* context, uint32_t index) {
    UNUSED(index);
    SecPlusApp* app = context;
    scene_manager_handle_custom_event(app->scene_manager, SecPlusEvent_Transmit);
}

void secplus_app_scene_on_enter_main(void* context) {
    SecPlusApp* app = context;
    popup_reset(app->popup);
    popup_set_context(app->popup, app);
    popup_set_icon(app->popup, 32, 4, &I_key_64_32);
    snprintf(app->text, 64, "Fixed code: %lu\nRolling code: %lu", app->codes[0], app->codes[1]);
    popup_set_text(app->popup, app->text, 4, 42, AlignLeft, AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, SecPlusView_Main);
}

bool secplus_app_scene_on_event_main(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void secplus_app_scene_on_exit_main(void* context) {
    SecPlusApp* app = context;
    popup_reset(app->popup);
}

void (*const secplus_app_scene_on_enter_handlers[])(void*) = {secplus_app_scene_on_enter_main};

bool (*const secplus_app_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    secplus_app_scene_on_event_main};

void (*const secplus_app_scene_on_exit_handlers[])(void*) = {secplus_app_scene_on_exit_main};

const SceneManagerHandlers secplus_app_scene_event_handlers = {
    .on_enter_handlers = secplus_app_scene_on_enter_handlers,
    .on_event_handlers = secplus_app_scene_on_event_handlers,
    .on_exit_handlers = secplus_app_scene_on_exit_handlers,
    .scene_num = 1};

bool secplus_app_scene_manager_custom_event_callback(void* context, uint32_t custom_event) {
    furi_assert(context);
    SecPlusApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

bool secplus_app_scene_manager_navigation_event_callback(void* context) {
    furi_assert(context);
    SecPlusApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

bool secplus_app_view_input_event_callback(InputEvent* event, void* context) {
    // TODO: Ugly! Remove the global app.
    if(event->type == InputTypePress && event->key == InputKeyOk) {
        secplus_app_transmit(app, app->codes[0], app->codes[1]);
        app->codes[1] += 3;
        FURI_LOG_I(TAG, "New codes, fixed=%lu, rolling=%lu", app->codes[0], app->codes[1]);
        secplus_save_codes(app);
        snprintf(
            app->text, 64, "Fixed code: %lu\nRolling code: %lu", app->codes[0], app->codes[1]);
        popup_set_text((Popup*)context, app->text, 4, 42, AlignLeft, AlignTop);
        return true;
    }
    return false;
}

SecPlusApp* secplus_app_init() {
    FURI_LOG_I(TAG, "Initializing app...");
    furi_hal_power_suppress_charge_enter();

    // Init GUI.
    SecPlusApp* app = malloc(sizeof(SecPlusApp));
    app->scene_manager = scene_manager_alloc(&secplus_app_scene_event_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    app->popup = popup_alloc();
    view_set_input_callback(popup_get_view(app->popup), secplus_app_view_input_event_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, secplus_app_scene_manager_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, secplus_app_scene_manager_navigation_event_callback);
    view_dispatcher_add_view(app->view_dispatcher, SecPlusView_Main, popup_get_view(app->popup));

    // Init data.
    if(!secplus_init_storage() || !secplus_read_codes(app)) {
        return NULL;
    }

    return app;
}

void secplus_app_free(SecPlusApp* app) {
    FURI_LOG_I(TAG, "Exiting app...");
    furi_assert(app);
    furi_hal_power_suppress_charge_exit();

    scene_manager_free(app->scene_manager);
    view_dispatcher_remove_view(app->view_dispatcher, SecPlusView_Main);
    view_dispatcher_free(app->view_dispatcher);
    popup_free(app->popup);
    free(app);
}

void secplus_app_set_log_level() {
#ifdef FURI_DEBUG
    furi_log_set_level(FuriLogLevelTrace);
#else
    furi_log_set_level(FuriLogLevelInfo);
#endif
}

int32_t secplus_app(void* p) {
    UNUSED(p);

    app = secplus_app_init();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, SecPlusScene_Main);
    view_dispatcher_run(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    secplus_app_free(app);
    return 0;
}
