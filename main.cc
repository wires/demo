// Attempt at demosceney things
// Copyright 2018 Arian van Putten, Radek Slupik, Ruud van Asseldonk

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3. A copy
// of the License is available in the root of the repository.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GLFW/glfw3.h>

static float vertices[] = {
  -0.5f, -0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
   0.0f,  0.5f, 0.0f
};

static float vsQuad[] = {
   1.0f, -1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f,
   1.0f,  1.0f, 0.0f,
  -1.0f,  1.0f, 0.0f,
};

struct VertexBuffer {
  unsigned int vbo;
  unsigned int vao;
  size_t size;
  GLenum mode;

  VertexBuffer() {}

  VertexBuffer(float* begin, float* end, GLenum mode_) {
    mode = mode_;
    size = end - begin;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, begin, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    printf("vbo: %i vao: %i size: %zu\n", vbo, vao, size);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), begin, GL_STATIC_DRAW);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(
      0, // Attribute index
      3, // Elements per vertex
      GL_FLOAT,
      GL_FALSE, // Normalized
      0, // Stride: tightly packed
      nullptr
    );
  }

  void draw() const {
    glBindVertexArray(vao);
    glDrawArrays(mode, 0, size);
  }
};


struct Program {
  unsigned int vertexShader;
  unsigned int fragmentShader;
  unsigned int shaderProgram;
 
  Program() {}
  Program(const char* vertexShaderSource, const char* fragmentShaderSource) {
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
  }

  void setupDraw(float iTime) const {
    glUseProgram(shaderProgram);
    glUniform1f(0, iTime);
  }
};


static Program quadProgram;
static VertexBuffer vbTriangle;
static VertexBuffer vbQuad;

static const char* vertexShaderSource = R"(
#version 420
layout (location = 0) in vec3 aPos;
out vec2 uv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    uv = aPos.xy;
}
)";

static const char* fragmentShaderSource = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
in vec2 uv;
out vec4 FragColor;

layout (location = 0) uniform float iTime;

uint hash(uint x) {
  x = x ^ uint(61) ^ (x >> 16);
  x *= uint(9);
  x = x ^ (x >> 4);
  x *= 0x27d4eb2d;
  x = x ^ (x >> 15);
  return x;
}

uint merge(uint x, uint y) {
  return hash(x ^ (y * 65537));
}

float uintToFloat(uint seed) {
  return seed * (1.0f / 4294967296.0f);
}

float vhs() {
  uint h = 17;
  h = merge(h, floatBitsToUint(iTime));
  h = merge(h, floatBitsToUint(uv.x));
  h = merge(h, floatBitsToUint(uv.y));
  highp float p = uintToFloat(h);
  float w = sin(uv.y * 3.0 + iTime * 2) * 0.2 + 0.8;
  p = w * p + (1.0 - w);
  float k = 0.8 + 0.2 * fract(uv.y * 72);
  return p * k;
}

void main() {
    vec3 color = vec3(0.8f, 0.8f, 0.8f);
    color *= vhs();
    color *= vec3(0.97, 0.9, 1.0);
    FragColor = vec4(color.rgb, 1.0f);
}
)";



void setup() {
  quadProgram = Program(vertexShaderSource, fragmentShaderSource);
  vbTriangle = VertexBuffer(std::begin(vertices), std::end(vertices), GL_TRIANGLE_STRIP);
  vbQuad = VertexBuffer(std::begin(vsQuad), std::end(vsQuad), GL_TRIANGLE_STRIP);
}

float iTime = 0.0;

void render() {
  glClear(GL_COLOR_BUFFER_BIT);
  quadProgram.setupDraw(iTime);
  vbTriangle.draw();
  vbQuad.draw();
  iTime += 0.01;
}

void reportError(GLenum, GLenum, GLuint, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "%s\n", message);

  if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) abort();
}

int main(int argc, char** argv) {
  GLFWwindow* window;

  if (!glfwInit()) return -2;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1280, 720, "Demo", nullptr, nullptr);

  if (!window) return -1;

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

  // Enable vsync. I think.
  glfwSwapInterval(1);

  setup();

  while (!glfwWindowShouldClose(window)) {
    render();
    iTime += 0.01;
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
