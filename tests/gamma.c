//========================================================================
// Gamma correction test program
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
// This program is used to test the gamma correction functionality for
// both full screen and windowed mode windows
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#define STEP_SIZE 0.1f

static GLfloat gamma_value = 1.0f;

static void usage(void)
{
    printf("Usage: gamma [-h] [-f]\n");
}

static void set_gamma(GLFWwindow* window, float value)
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(window);
    if (!monitor)
        monitor = glfwGetPrimaryMonitor();

    gamma_value = value;
    printf("Gamma: %f\n", gamma_value);
    glfwSetGamma(monitor, gamma_value);
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
        case GLFW_KEY_ESCAPE:
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }

        case GLFW_KEY_KP_ADD:
        case GLFW_KEY_Q:
        {
            set_gamma(window, gamma_value + STEP_SIZE);
            break;
        }

        case GLFW_KEY_KP_SUBTRACT:
        case GLFW_KEY_W:
        {
            if (gamma_value - STEP_SIZE > 0.f)
                set_gamma(window, gamma_value - STEP_SIZE);

            break;
        }
    }
}

int main(int argc, char** argv)
{
    int width, height, ch;
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;
    NVGcontext* nvg;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                monitor = glfwGetPrimaryMonitor();
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

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

    window = glfwCreateWindow(width, height, "Gamma Test", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    set_gamma(window, 1.f);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        int i, width, height;
        NVGpaint paint;
        const int slices = 6;
        const float border = 40.f;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        nvgBeginFrame(nvg, width, height, 1.f);

        for (i = 0;  i < slices;  i++)
        {
            const float step = (width - border * 2.f) / slices;
            const float x = border + i * step;
            paint = nvgLinearGradient(nvg,
                                      x, 0.f, x + step, 0.f,
                                      nvgHSL(i / (float) slices, 1.f, 0.5f),
                                      nvgHSL((i + 1) / (float) slices, 1.f, 0.5f));

            nvgBeginPath(nvg);
            nvgRect(nvg, x, border, step, (height - border * 3.f) / 2.f);
            nvgFillPaint(nvg, paint);
            nvgFill(nvg);
        }

        paint = nvgLinearGradient(nvg,
                                  border, 0.f, width - border, 0.f,
                                  nvgRGB(0, 0, 0), nvgRGB(255, 255, 255));

        nvgBeginPath(nvg);
        nvgRect(nvg,
                border,
                height / 2.f + border / 2.f,
                width - border * 2.f,
                (height - border * 3.f) / 2.f);
        nvgFillPaint(nvg, paint);
        nvgFill(nvg);

        nvgEndFrame(nvg);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    nvgDeleteGL2(nvg);
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

