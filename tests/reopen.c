//========================================================================
// Window re-opener (open/close stress test)
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
// This test came about as the result of bug #1262773
//
// It closes and re-opens the GLFW window every five seconds, alternating
// between windowed and full screen mode
//
// It also times and logs opening and closing actions and attempts to separate
// user initiated window closing from its own
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_close_callback(GLFWwindow* window)
{
    printf("Close callback triggered\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
    }
}

static GLFWwindow* open_window(int width, int height, GLFWmonitor* monitor)
{
    double base;
    GLFWwindow* window;

    base = glfwGetTime();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(width, height, "Window Re-opener", monitor, NULL);
    if (!window)
        return NULL;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);

    if (monitor)
    {
        printf("Opening full screen window on monitor %s took %0.3f seconds\n",
               glfwGetMonitorName(monitor),
               glfwGetTime() - base);
    }
    else
    {
        printf("Opening regular window took %0.3f seconds\n",
               glfwGetTime() - base);
    }

    return window;
}

static void close_window(GLFWwindow* window)
{
    double base = glfwGetTime();
    glfwDestroyWindow(window);
    printf("Closing window took %0.3f seconds\n", glfwGetTime() - base);
}

int main(int argc, char** argv)
{
    int count = 0;
    GLFWwindow* window;
    NVGcontext* nvg;

    srand((unsigned int) time(NULL));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    for (;;)
    {
        GLFWmonitor* monitor = NULL;

        if (count % 2 == 0)
        {
            int monitorCount;
            GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
            monitor = monitors[rand() % monitorCount];
        }

        window = open_window(640, 480, monitor);
        if (!window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

        nvg = nvgCreateGL2(0);
        if (!nvg)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetTime(0.0);

        while (glfwGetTime() < 5.0)
        {
            float side;
            int width, height;

            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClearColor(1.0, 1.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            side = fminf(width, height) * 0.65;

            nvgBeginFrame(nvg, width, height, 1.f);

            nvgFillColor(nvg, nvgRGBA(0, 0, 0, 255));
            nvgBeginPath(nvg);
            nvgRect(nvg, 0.f, 40, width, height - 80);
            nvgFill(nvg);
            nvgBeginPath(nvg);
            nvgRect(nvg, 40, 0.f, width - 80, height);
            nvgFill(nvg);
            nvgBeginPath(nvg);
            nvgRect(nvg, 5, 5, width - 10, height - 10);
            nvgFill(nvg);

            nvgFillColor(nvg, nvgRGBA(255, 255, 255, 255));
            nvgBeginPath(nvg);
            nvgTranslate(nvg, width / 2.f, height / 2.f);
            nvgRotate(nvg, (float) glfwGetTime());
            nvgRect(nvg, -side / 2.f, -side / 2.f, side, side);
            nvgFill(nvg);

            nvgEndFrame(nvg);

            glfwSwapBuffers(window);
            glfwPollEvents();

            if (glfwWindowShouldClose(window))
            {
                close_window(window);
                printf("User closed window\n");

                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        printf("Closing window\n");
        close_window(window);

        count++;
    }

    glfwTerminate();
}

