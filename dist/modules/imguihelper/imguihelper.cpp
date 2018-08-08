// Adapted from https://github.com/nx-python/imgui-switch/blob/master/src/main.cpp
#include <switch.h>

#include "Python.h"

#include "imgui/imgui.h"
#include "imgui/imgui_sw.hpp"

#include <vector>

u32 * framebuf;
int width, height;
std::vector<uint32_t> * pixel_buffer;
bool initialized = false;

PyDoc_STRVAR(module_doc, "Helper functions for pyimgui on switch\n");


PyDoc_STRVAR(imguihelper_initialize_doc, ""); // TODO

static PyObject *
imguihelper_initialize(PyObject *self, PyObject *args)
{
    gfxSetMode(GfxMode_LinearDouble);

    if (initialized) Py_RETURN_NONE;


    framebuf = (u32*) gfxGetFramebuffer((u32*)&width, (u32*)&height);
    pixel_buffer = new std::vector<uint32_t>(width * height, 0);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = true;
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::GetStyle().ScaleAllSizes(2);
    ImGui::GetStyle().MouseCursorScale = 1;    

    imgui_sw::bind_imgui_painting();
    imgui_sw::make_style_fast();
    initialized = true;

    Py_RETURN_NONE;
}



PyDoc_STRVAR(imguihelper_handleinputs_doc, ""); // TODO

static PyObject *
imguihelper_handleinputs(PyObject *self, PyObject *args)
{
    ImGuiIO& io = ImGui::GetIO();
    hidScanInput();
    u32 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
    JoystickPosition pos;
    hidJoystickRead(&pos, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);

    memset(io.NavInputs, 0, sizeof(io.NavInputs));

    #define MAP_BUTTON(NAV, BUTTON)       { if (kHeld & BUTTON) io.NavInputs[NAV] = 1.0f; }
    MAP_BUTTON(ImGuiNavInput_Activate,    KEY_A);
    MAP_BUTTON(ImGuiNavInput_Cancel,      KEY_B);
    MAP_BUTTON(ImGuiNavInput_Menu,        KEY_Y);
    MAP_BUTTON(ImGuiNavInput_Input,       KEY_X);
    MAP_BUTTON(ImGuiNavInput_DpadLeft,    KEY_DLEFT);
    MAP_BUTTON(ImGuiNavInput_DpadRight,   KEY_DRIGHT);
    MAP_BUTTON(ImGuiNavInput_DpadUp,      KEY_DUP);
    MAP_BUTTON(ImGuiNavInput_DpadDown,    KEY_DDOWN);
    MAP_BUTTON(ImGuiNavInput_FocusPrev,   KEY_L);
    MAP_BUTTON(ImGuiNavInput_FocusNext,   KEY_R);
    MAP_BUTTON(ImGuiNavInput_TweakSlow,   KEY_L);
    MAP_BUTTON(ImGuiNavInput_TweakFast,   KEY_R);
    MAP_BUTTON(ImGuiNavInput_LStickLeft,  KEY_LSTICK_LEFT);
    MAP_BUTTON(ImGuiNavInput_LStickRight, KEY_LSTICK_RIGHT);
    MAP_BUTTON(ImGuiNavInput_LStickUp,    KEY_LSTICK_UP);
    MAP_BUTTON(ImGuiNavInput_LStickDown,  KEY_LSTICK_DOWN);
    #undef MAP_BUTTON
    
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;


    // io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX);
    io.MouseDown[0] = false;
    unsigned long touch_count = hidTouchCount();
    if (touch_count > 0) {
        touchPosition pos;
        hidTouchRead(&pos, 0);
        io.MousePos = ImVec2(pos.px, pos.py);
        io.MouseDown[0] = true;
    }

    Py_RETURN_NONE;
}



PyDoc_STRVAR(imguihelper_render_doc, ""); // TODO

static PyObject *
imguihelper_render(PyObject *self, PyObject *args)
{
    // fill gray (this could be any previous rendering)
    std::fill_n(pixel_buffer->data(), width * height, 0x19191919u);

    // overlay the GUI
    imgui_sw::paint_imgui(pixel_buffer->data(), width, height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            u32 pos = i * width + j;
            framebuf[pos] = pixel_buffer->data()[pos];
        }
    }

    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();

    Py_RETURN_NONE;
}



PyDoc_STRVAR(imguihelper_shutdown_doc, ""); // TODO

static PyObject *
imguihelper_shutdown(PyObject *self, PyObject *args)
{
    imgui_sw::unbind_imgui_painting();
    delete pixel_buffer;
    initialized = false;

    Py_RETURN_NONE;
}



static PyMethodDef module_methods[] = {
    {"initialize", imguihelper_initialize, METH_NOARGS, imguihelper_initialize_doc},
    {"handleinputs", imguihelper_handleinputs, METH_NOARGS, imguihelper_handleinputs_doc},
    {"render", imguihelper_render, METH_NOARGS, imguihelper_render_doc},
    {"shutdown", imguihelper_shutdown, METH_NOARGS, imguihelper_shutdown_doc},
    {NULL, NULL}
};

struct PyModuleDef _imguihelper_Module = {
    PyModuleDef_HEAD_INIT,
    "imguihelper",
    module_doc,
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL,
};

PyMODINIT_FUNC
PyInit_imguihelper(void)
{
    PyObject *m = PyModule_Create(&_imguihelper_Module);
    return m;
}