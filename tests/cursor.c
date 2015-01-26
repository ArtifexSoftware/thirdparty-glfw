//========================================================================
// Cursor image and cursor mode test
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
// This test provides an interface to the cursor image and cursor mode
// parts of the API.
//
// Custom cursor image generation by urraka.
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CURSOR_FRAME_COUNT 60

static double cursor_x;
static double cursor_y;
static int swap_interval = 1;
static GLboolean wait_events = GL_FALSE;
static GLboolean animate_cursor = GL_FALSE;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static float star(int x, int y, float t)
{
    const float c = 64 / 2.f;

    const float i = (0.25f * (float) sin(2.f * M_PI * t) + 0.75f);
    const float k = 64 * 0.046875f * i;

    const float dist = (float) sqrt((x - c) * (x - c) + (y - c) * (y - c));

    const float salpha = 1.f - dist / c;
    const float xalpha = (float) x == c ? c : k / (float) fabs(x - c);
    const float yalpha = (float) y == c ? c : k / (float) fabs(y - c);

    return fmax(0.f, fmin(1.f, i * salpha * 0.2f + salpha * xalpha * yalpha));
}

static GLFWcursor* create_cursor_frame(float t)
{
    int i = 0, x, y;
    unsigned char buffer[64 * 64 * 4];
    const GLFWimage image = { 64, 64, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = (unsigned char) (255 * star(x, y, t));
        }
    }

    return glfwCreateCursor(&image, image.width / 2, image.height / 2);
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    printf("%0.3f: Cursor position: %f %f (%+f %+f)\n",
           glfwGetTime(),
           x, y, x - cursor_x, y - cursor_y);

    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_A:
        {
            animate_cursor = !animate_cursor;
            if (!animate_cursor)
                glfwSetCursor(window, NULL);

            break;
        }

        case GLFW_KEY_ESCAPE:
        {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            }

            /* FALLTHROUGH */
        }

        case GLFW_KEY_N:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            printf("(( cursor is normal ))\n");
            break;

        case GLFW_KEY_D:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            printf("(( cursor is disabled ))\n");
            break;

        case GLFW_KEY_H:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            printf("(( cursor is hidden ))\n");
            break;

        case GLFW_KEY_SPACE:
            swap_interval = 1 - swap_interval;
            printf("(( swap interval: %i ))\n", swap_interval);
            glfwSwapInterval(swap_interval);
            break;

        case GLFW_KEY_W:
            wait_events = !wait_events;
            printf("(( %sing for events ))\n", wait_events ? "poll" : "wait");
            break;
    }
}

int main(void)
{
    int i;
    GLFWwindow* window;
    GLFWcursor* cursors[CURSOR_FRAME_COUNT];
    NVGcontext* nvg;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
    {
        cursors[i] = create_cursor_frame(i / (float) CURSOR_FRAME_COUNT);
        if (!cursors[i])
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 480, "Cursor Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(swap_interval);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwGetCursorPos(window, &cursor_x, &cursor_y);
    printf("Cursor position: %f %f\n", cursor_x, cursor_y);

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    glClearColor(0.f, 0.f, 0.f, 0.f);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        nvgBeginFrame(nvg, width, height, 1.f);

        nvgStrokeColor(nvg, nvgRGB(255, 255, 255));
        nvgBeginPath(nvg);
        nvgMoveTo(nvg, 0.f, cursor_y);
        nvgLineTo(nvg, width, cursor_y);
        nvgMoveTo(nvg, cursor_x, 0.f);
        nvgLineTo(nvg, cursor_x, height);
        nvgStroke(nvg);

        nvgEndFrame(nvg);
        glfwSwapBuffers(window);

        if (animate_cursor)
        {
            const int i = (int) (glfwGetTime() * 30.0) % CURSOR_FRAME_COUNT;
            glfwSetCursor(window, cursors[i]);
        }

        if (wait_events)
            glfwWaitEvents();
        else
            glfwPollEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    nvgDeleteGL2(nvg);
    glfwDestroyWindow(window);

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
        glfwDestroyCursor(cursors[i]);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

