//========================================================================
// Vsync enabling test
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test renders a high contrast, horizontally moving bar, allowing for
// visual verification of whether the set swap interval is indeed obeyed
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "getopt.h"

static GLboolean swap_tear;
static int swap_interval;
static double frame_rate;

static void usage(void)
{
    printf("Usage: iconify [-h] [-f]\n");
    printf("Options:\n");
    printf("  -f create full screen window\n");
    printf("  -h show this help\n");
}

static void update_window_title(GLFWwindow* window)
{
    char title[256];

    sprintf(title, "Tearing detector (interval %i%s, %0.1f Hz)",
            swap_interval,
            (swap_tear && swap_interval < 0) ? " (swap tear)" : "",
            frame_rate);

    glfwSetWindowTitle(window, title);
}

static void set_swap_interval(GLFWwindow* window, int interval)
{
    swap_interval = interval;
    glfwSwapInterval(swap_interval);
    update_window_title(window);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_UP:
        {
            if (swap_interval + 1 > swap_interval)
                set_swap_interval(window, swap_interval + 1);
            break;
        }

        case GLFW_KEY_DOWN:
        {
            if (swap_tear)
            {
                if (swap_interval - 1 < swap_interval)
                    set_swap_interval(window, swap_interval - 1);
            }
            else
            {
                if (swap_interval - 1 >= 0)
                    set_swap_interval(window, swap_interval - 1);
            }
            break;
        }

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, 1);
            break;
    }
}

int main(int argc, char** argv)
{
    int ch, width, height;
    unsigned long frame_count = 0;
    double last_time, current_time;
    GLboolean fullscreen = GL_FALSE;
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;
    NVGcontext* nvg;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = GL_TRUE;
                break;
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (fullscreen)
    {
        const GLFWvidmode* mode;

        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 640;
        height = 480;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(width, height, "", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    set_swap_interval(window, 0);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    last_time = glfwGetTime();
    frame_rate = 0.0;
    swap_tear = (glfwExtensionSupported("WGL_EXT_swap_control_tear") ||
                 glfwExtensionSupported("GLX_EXT_swap_control_tear"));

    while (!glfwWindowShouldClose(window))
    {
        float position;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        nvgBeginFrame(nvg, width, height, 1.f);

        position = (float) ((cos(glfwGetTime() * 4.0) + 1.0) * 0.75 / 2.0);
        nvgBeginPath(nvg);
        nvgRect(nvg, width * position, 0.f, width * 0.25f, height);
        nvgFillColor(nvg, nvgRGBA(255, 255, 255, 255));
        nvgFill(nvg);

        nvgEndFrame(nvg);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;

        current_time = glfwGetTime();
        if (current_time - last_time > 1.0)
        {
            frame_rate = frame_count / (current_time - last_time);
            frame_count = 0;
            last_time = current_time;
            update_window_title(window);
        }
    }

    nvgDeleteGL2(nvg);
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

