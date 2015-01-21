//========================================================================
// Context sharing test program
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
// This program is used to test sharing of objects between contexts
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

#define WIDTH  400
#define HEIGHT 400
#define OFFSET 50

static GLFWwindow* windows[2];

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static GLFWwindow* open_window(const char* title, GLFWwindow* share, int posX, int posY)
{
    GLFWwindow* window;

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, title, NULL, share);
    if (!window)
        return NULL;

    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, posX, posY);
    glfwShowWindow(window);

    glfwSetKeyCallback(window, key_callback);

    return window;
}

static int create_image(NVGcontext* nvg)
{
    int i;
    unsigned char pixels[256 * 256 * 4];

    for (i = 0;  i < sizeof(pixels);  i++)
        pixels[i] = rand() % 256;

    return nvgCreateImageRGBA(nvg, 256, 256, 0, pixels);
}

static void draw_quad(NVGcontext* nvg, int image, NVGcolor color)
{
    int width, height;

    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    nvgBeginFrame(nvg, width, height, 1.f);

    nvgBeginPath(nvg);
    nvgRect(nvg, 0.f, 0.f, width, height);
    nvgFillPaint(nvg, nvgImagePattern(nvg, 0.f, 0.f, width, height, 0.f, image, 1.f));
    nvgFill(nvg);
    nvgFillColor(nvg, nvgTransRGBA(color, 127));
    nvgFill(nvg);

    nvgEndFrame(nvg);
}

int main(int argc, char** argv)
{
    int x, y, width;
    NVGcontext* nvg;
    int image;

    srand((unsigned int) time(NULL));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    windows[0] = open_window("First", NULL, OFFSET, OFFSET);
    if (!windows[0])
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(windows[0]);
    glfwSwapInterval(1);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // This is the one and only time we create an image
    // It is created inside the first context, created above
    // It will then be shared with the second context, created below
    image = create_image(nvg);

    glfwGetWindowPos(windows[0], &x, &y);
    glfwGetWindowSize(windows[0], &width, NULL);

    // Put the second window to the right of the first one
    windows[1] = open_window("Second", windows[0], x + width + OFFSET, y);
    if (!windows[1])
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(windows[0]) &&
           !glfwWindowShouldClose(windows[1]))
    {
        glfwMakeContextCurrent(windows[0]);
        draw_quad(nvg, image, nvgRGB(200, 0, 200));

        glfwMakeContextCurrent(windows[1]);
        draw_quad(nvg, image, nvgRGB(200, 200, 0));

        glfwSwapBuffers(windows[0]);
        glfwSwapBuffers(windows[1]);

        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

