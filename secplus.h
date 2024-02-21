#ifndef SECPLUS_HEADER
#define SECPLUS_HEADER

#define TAG "secplus"

#include <stdint.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/popup.h>

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Popup* popup;

    char text[64];
    uint32_t codes[2];
} SecPlusApp;

#endif