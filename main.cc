// Attempt at demosceney things
// Copyright 2018 Arian van Putten, Radek Slupik, Ruud van Asseldonk

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3. A copy
// of the License is available in the root of the repository.

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GLFW/glfw3.h>

static float vertices[] = {
  -0.5f, -0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
   0.0f,  0.5f, 0.0f
};

static unsigned int vbo;
static unsigned int vao;
static unsigned int vertexShader;
static unsigned int fragmentShader;
static unsigned int shaderProgram;

static const char* vertexShaderSource = R"(
#version 440
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 440
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 1.0f, 0.1f, 1.0f);
}
)";

void setup() {
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao);

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(shaderProgram);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void reportError(GLenum, GLenum, GLuint, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "%s\n", message);

  if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) abort();
}

int main(int argc, char** argv) {
  GLFWwindow* window;

  if (!glfwInit()) return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1280, 720, "Demo", nullptr, nullptr);

  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glDebugMessageCallback(reportError, nullptr);
  glEnable(GL_DEBUG_OUTPUT);

  // Set viewport size to window size * scaling, to fix HiDPI. The framebuffer
  // size should be the right size, adjusted for scaling, but unfortunately it
  // is not on my system, the size returned by glfwGetFramebufferSize and
  // glfwGetWindowSize are the same, even on a HiDPI display.

  // HACK: Note that GetFramebufferSize should return the device pixels, not
  // screen coordinates, so this *should* work for HiDPI, but it does not. See
  // also https://github.com/glfw/glfw/issues/1168. So we just scale if provided
  // as a command line flag.
  int width, height;
  int scale = 1;
  if (argc == 2 && !strcmp(argv[1], "--hidpi")) scale = 2;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width * scale, height * scale);

  setup();

  while (!glfwWindowShouldClose(window)) {
    render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
