//========================================================================
// Mouse cursor accuracy test
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
// This test came about as the result of bug #1867804
//
// No sign of said bug has so far been detected
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <stdio.h>
#include <stdlib.h>

static double cursor_x = 0.0, cursor_y = 0.0;
static int swap_interval = 1;

static void set_swap_interval(GLFWwindow* window, int interval)
{
    char title[256];

    swap_interval = interval;
    glfwSwapInterval(swap_interval);

    sprintf(title, "Cursor Inaccuracy Detector (interval %i)", swap_interval);

    glfwSetWindowTitle(window, title);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        set_swap_interval(window, 1 - swap_interval);
}

int main(void)
{
    GLFWwindow* window;
    NVGcontext* nvg;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 480, "", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    set_swap_interval(window, swap_interval);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        nvgBeginFrame(nvg, width, height, 1.f);

        nvgBeginPath(nvg);
        nvgMoveTo(nvg, 0.f, cursor_y);
        nvgLineTo(nvg, width, cursor_y);
        nvgMoveTo(nvg, cursor_x, 0.f);
        nvgLineTo(nvg, cursor_x, height);
        nvgStrokeColor(nvg, nvgRGB(255, 255, 255));
        nvgStroke(nvg);

        nvgEndFrame(nvg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

