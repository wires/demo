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
   1.0f, -1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
};

static float vsUnitUvs[] = {
   1.0f, -1.0f,
   1.0f,  1.0f,
  -1.0f, -1.0f,
  -1.0f,  1.0f,
};

static float vsSquare[] = {
   1.0f, -1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
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

  void setupDraw() const {
    glUseProgram(shaderProgram);
  }
};

static Program pGrid;
static Program pSun;
static Program pScreen;
static VertexBuffer vbQuad;
static VertexBuffer vbSquare;
static VertexBuffer vbGrid;

static unsigned int depthbuffer;
static unsigned int framebuffer;
static unsigned int renderTexture;

static const char* vertexShaderRoad = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUv;
layout (location = 0) uniform float iTime;
layout (location = 1) uniform vec2 iOff;

out vec2 uv;


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

float mountainAndRoad(vec3 p) {
  float n = (noise(p - vec3(iTime * 0.5, -iTime * 0.7, 0.0f)) * 3.0f - 1.0f) * cos(p.x);
  float h = 0.6f;
  float t = min(1.0f, pow(p.x, 4));
  return t * n + (1.0f - t) * h;
}

void main()
{
  float a = -1.7;
  mat4 rot = mat4(1.0, 0.0,        0.0, 0.0,
                  0.0, cos(a), -sin(a), 0.0,
                  0.0, sin(a),  cos(a), 0.0,
                  0.0, 0.0,        0.0, 1.0);
  vec3 pos = aPos;
  pos.xy += iOff;
  pos.y -= fract(iTime);
  pos.z += mountainAndRoad(pos);
  vec4 fpos = rot * vec4(pos, 1.0);
  gl_Position = vec4(fpos.xy, 1 - fpos.z, fpos.z);
  uv = pos.xy;
}
)";

static const char* vertexShaderUv = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUv;
layout (location = 0) uniform float iTime;

out vec2 uv;


void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
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
  if (fract(uv.y * (15.0f + 7.5f * sin(uv.y))) < 0.5f) discard;
  if (uv.y < -0.5f) discard;
  FragColor = vec4(1.0f, 0.6f + 0.3f * uv.y, 0.1f, 1.0f);
}
)";

static const char* fragmentShaderQuad = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
in vec2 uv;
out vec4 FragColor;

layout (location = 0) uniform float iTime;

void main() {
    vec3 color = vec3(0.8f, 0.8f, 0.8f);
    float d = 1.0f - uv.y * 0.12;
    FragColor = vec4(color.rgb * d, 1.0f);
}
)";

static const char* fragmentShaderScreen = R"(
#version 420
#extension GL_ARB_explicit_uniform_location : enable
in vec2 uv;
out vec4 FragColor;

layout (location = 0) uniform float iTime;
layout (location = 1) uniform sampler2D screenTexture;

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
  float w = sin(uv.y * 3.0 + iTime * 2) * 0.9 + 0.8;
  p = w * p + (1.0 - w);
  float k = 0.8 + 0.2 * fract(uv.y * 64);
  return p * k;
}

void main() {
  float dx = 0.003;
  float cr = texture(screenTexture, uv * 0.5 + vec2(0.5) + vec2(dx, 0.0)).r;
  float cg = texture(screenTexture, uv * 0.5 + vec2(0.5)).g;
  float cb = texture(screenTexture, uv * 0.5 + vec2(0.5) + vec2(-dx, 0.0)).b;
  vec3 color = vec3(cr, cg, cb);
  color += vec3(0.1, 0.0, 0.3);
  float v = vhs();
  color *= vec3(0.97, 0.9, 1.0);
  color += vec3(0.35f, 0.05f, 0.1f) * v;
  FragColor = vec4(color, 1.0);
}
)";

void setup(int width, int height) {
  pGrid = Program(vertexShaderRoad, fragmentShaderQuad);
  pSun = Program(vertexShaderUv, fragmentShaderSun);
  pScreen = Program(vertexShaderUv, fragmentShaderScreen);

  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &renderTexture);
  glBindTexture(GL_TEXTURE_2D, renderTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);
  GLenum drawbuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawbuffers);

  // Make the square really a square, correct for aspect ratio.
  const float aspectRatio = 9.0f / 16.0;
  vsSquare[0 * 3] *= aspectRatio;
  vsSquare[1 * 3] *= aspectRatio;
  vsSquare[2 * 3] *= aspectRatio;
  vsSquare[3 * 3] *= aspectRatio;
  // Make the square a bit smaller.
  for (float& x : vsSquare) x *= 0.7;

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

  int gx = 12;
  int gy = 12;

  float* vsGrid = new float[gx * gy * 6 * 3];
  float* vsGridUvs = new float[gx * gy * 6 * 2];

  for (int y = 0; y < gy; y++) {
    float py0 = y * (1.0 / gy) * 2.0 - 1.0;
    float py1 = (y + 1) * (1.0 / gy) * 2.0 - 1.0;
    for (int x = 0; x < gx; x++) {
      float* grid = vsGrid + ((y * gx + x) * 6 * 3);
      float px0 = x * (1.0 / gx) * 2.0 - 1.0;
      float px1 = (x + 1) * (1.0 / gx) * 2.0 - 1.0;
      grid[0]  =  px1; grid[1]  = py0; grid[2]  = 1.0f;
      grid[3]  =  px1; grid[4]  = py1; grid[5]  = 1.0f;
      grid[6]  =  px0; grid[7]  = py0; grid[8]  = 1.0f;
      grid[9]  =  px1; grid[10] = py1; grid[11] = 1.0f;
      grid[12] =  px0; grid[13] = py0; grid[14] = 1.0f;
      grid[15] =  px0; grid[16] = py1; grid[17] = 1.0f;
    }
  }
  for (int y = 0; y < gy; y++) {
    for (int x = 0; x < gx; x++) {
      int k = (y * gx + x) * 6;
      for (int z = 0; z < 6; z++) {
        vsGridUvs[(k + z) * 2 + 0] = vsGrid[(k + z) * 3 + 0];
        vsGridUvs[(k + z) * 2 + 1] = vsGrid[(k + z) * 3 + 1];
      }
    }
  }

  vbGrid = VertexBuffer(
    vsGrid, vsGrid + (gx * gy * 6 * 3),
    vsGridUvs, vsGridUvs + (gx * gy * 6 * 2),
    GL_TRIANGLES
  );
}

float iTime = 0.0;

void render(int width, int height) {
  // Render to texture.
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  pSun.setupDraw();
  vbSquare.draw();

  pGrid.setupDraw();
  glUniform1f(0, iTime);
//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  for (int m = -4; m <= 4; m += 2) {
    for (int k = 0; k < 22; k += 2) {
      glUniform2f(1, (float)m, (float)k); // iOff
      vbGrid.draw();
    }
  }
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_BLEND);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, width, height);

  pScreen.setupDraw();
  glUniform1f(0, iTime);
  vbQuad.draw();
}

void reportError(GLenum, GLenum, GLuint, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf(stderr, "%s\n", message);

  if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) abort();
}

GLFWwindow* window;
int width, height;

void loop() {
  //while (!glfwWindowShouldClose(window)) {
    render(width, height);
    iTime += 0.01;
    glfwSwapBuffers(window);
    glfwPollEvents();
  //}
}

#include <emscripten.h>


int main(int argc, char** argv) {

  if (!glfwInit()) return -2;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1280, 720, "Demo", nullptr, nullptr);

  if (!window) return -1;

  glfwMakeContextCurrent(window);

	//glDebugMessageCallback(reportError, nullptr);
  //glEnable(GL_DEBUG_OUTPUT);

  // Set viewport size to window size * scaling, to fix HiDPI. The framebuffer
  // size should be the right size, adjusted for scaling, but unfortunately it
  // is not on my system, the size returned by glfwGetFramebufferSize and
  // glfwGetWindowSize are the same, even on a HiDPI display.

  // HACK: Note that GetFramebufferSize should return the device pixels, not
  // screen coordinates, so this *should* work for HiDPI, but it does not. See
  // also https://github.com/glfw/glfw/issues/1168. So we just scale if provided
  // as a command line flag.
  glfwGetFramebufferSize(window, &width, &height);
  if (argc == 2 && !strcmp(argv[1], "--hidpi")) {
    width *= 2;
    height *= 2;
  }

  emscripten_set_main_loop (loop, 0, true);
  

  // Enable vsync. I think.
  glfwSwapInterval(1);

  setup(width, height);

  return 0;
}




