//========================================================================
// Multisample anti-aliasing test
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
// This test renders two high contrast, slowly rotating quads, one aliased
// and one (hopefully) anti-aliased, thus allowing for visual verification
// of whether MSAA is indeed enabled
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
        case GLFW_KEY_SPACE:
            glfwSetTime(0.0);
            break;
    }
}

static void usage(void)
{
    printf("Usage: msaa [-h] [-s SAMPLES]\n");
}

int main(int argc, char** argv)
{
    int ch, samples = 4;
    GLFWwindow* window;
    NVGcontext* nvg;

    while ((ch = getopt(argc, argv, "hs:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                samples = atoi(optarg);
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (samples)
        printf("Requesting MSAA with %i samples\n", samples);
    else
        printf("Requesting that MSAA not be available\n");

    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(800, 400, "Aliasing Detector", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    nvg = nvgCreateGL2(0);
    if (!nvg)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glGetIntegerv(GL_SAMPLES, &samples);
    if (samples)
        printf("Context reports MSAA is available with %i samples\n", samples);
    else
        printf("Context reports MSAA is unavailable\n");

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        float side;
        const double time = glfwGetTime();

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        side = fminf(width / 2.f, height) * 0.65;

        glDisable(GL_MULTISAMPLE);

        nvgBeginFrame(nvg, width, height, 1.f);
        nvgBeginPath(nvg);
        nvgTranslate(nvg, width * 0.25f, height / 2.f);
        nvgRotate(nvg, (float) (time * M_PI) / 180.f);
        nvgRect(nvg, -side / 2.f, -side / 2.f, side, side);
        nvgFillColor(nvg, nvgRGBA(255, 255, 255, 255));
        nvgFill(nvg);
        nvgEndFrame(nvg);

        glEnable(GL_MULTISAMPLE);

        nvgBeginFrame(nvg, width, height, 1.f);
        nvgBeginPath(nvg);
        nvgTranslate(nvg, width * 0.75f, height / 2.f);
        nvgRotate(nvg, (float) (time * M_PI) / 180.f);
        nvgRect(nvg, -side / 2.f, -side / 2.f, side, side);
        nvgFillColor(nvg, nvgRGBA(255, 255, 255, 255));
        nvgFill(nvg);
        nvgEndFrame(nvg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

