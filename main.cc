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

static float vsUnitUvs[] = {
   1.0f, -1.0f,
  -1.0f, -1.0f,
   1.0f,  1.0f,
  -1.0f,  1.0f,
};

static float vsSquare[] = {
   1.0f, -1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f,
   1.0f,  1.0f, 0.0f,
  -1.0f,  1.0f, 0.0f,
};

struct VertexBuffer {
  unsigned int vboPos;
  unsigned int vboUv;
  unsigned int vao;
  size_t posSize;
  size_t uvSize;
  GLenum mode;

  VertexBuffer() {}

  VertexBuffer(float* posBegin, float* posEnd, float* uvBegin, float* uvEnd, GLenum mode_) {
    mode = mode_;
    posSize = posEnd - posBegin;
    uvSize = uvEnd - uvBegin;
    glGenBuffers(1, &vboPos);
    glGenBuffers(1, &vboUv);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, posSize * sizeof(float), posBegin, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vboUv);
    glBufferData(GL_ARRAY_BUFFER, uvSize * sizeof(float), uvBegin, GL_STATIC_DRAW);

    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glVertexAttribPointer(
      0, // Attribute index
      3, // Elements per vertex
      GL_FLOAT,
      GL_FALSE, // Normalized
      0, // Stride: tightly packed
      nullptr
    );

    glBindBuffer(GL_ARRAY_BUFFER, vboUv);
    glVertexAttribPointer(
      1, // Attribute index
      2, // Elements per vertex
      GL_FLOAT,
      GL_FALSE, // Normalized
      0, // Stride: tightly packed
      nullptr
    );
  }

  void draw() const {
    glBindVertexArray(vao);
    glDrawArrays(mode, 0, posSize / 3);
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


static Program pQuad;
static Program pSun;
static VertexBuffer vbTriangle;
static VertexBuffer vbQuad;
static VertexBuffer vbSquare;

static const char* vertexShaderUv = R"(
#version 420
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUv;

out vec2 uv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    uv = aUv;
}
)";

static const char* fragmentShaderSun = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
in vec2 uv;
out vec4 FragColor;

layout (location = 0) uniform float iTime;

void main() {
  float r = dot(uv, uv);
  if (r > 1.0f) discard;
  FragColor = vec4(1.0f, 0.6f + 0.3f * uv.y, 0.1f, 1.0f);
}
)";

static const char* fragmentShaderQuad = R"(
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
  float p = uintToFloat(h);
  float w = sin(uv.y * 3.0 + iTime * 2) * 0.2 + 0.8;
  p = w * p + (1.0 - w);
  float k = 0.8 + 0.2 * fract(uv.y * 144);
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
  pQuad = Program(vertexShaderUv, fragmentShaderQuad);
  pSun = Program(vertexShaderUv, fragmentShaderSun);

  // Make the square really a square, correct for aspect ratio.
  const float aspectRatio = 9.0f / 16.0;
  vsSquare[0 * 3] *= aspectRatio;
  vsSquare[1 * 3] *= aspectRatio;
  vsSquare[2 * 3] *= aspectRatio;
  vsSquare[3 * 3] *= aspectRatio;
  // Make the square a bit smaller.
  for (float& x : vsSquare) x *= 0.7;

  vbTriangle = VertexBuffer(
    std::begin(vertices), std::end(vertices),
    std::begin(vertices), std::end(vertices),
    GL_TRIANGLE_STRIP
  );
  vbQuad = VertexBuffer(
    std::begin(vsQuad), std::end(vsQuad),
    std::begin(vsUnitUvs), std::end(vsUnitUvs),
    GL_TRIANGLE_STRIP
  );
  vbSquare = VertexBuffer(
    std::begin(vsSquare), std::end(vsSquare),
    std::begin(vsUnitUvs), std::end(vsUnitUvs),
    GL_TRIANGLE_STRIP
  );
}

float iTime = 0.0;

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  pQuad.setupDraw(iTime);
  vbQuad.draw();

  pSun.setupDraw(0.0f);
  vbSquare.draw();
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
