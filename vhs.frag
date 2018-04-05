float rand(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(float prev, in vec2 fragCoord) {
  float noiseS = sin(8.4 * fragCoord.y) / 2.0;
  return prev * 0.9 + 0.1 * noiseS;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec4 baseBase = vec4(0.2, 0.3, 0.8, 1.0);

  vec2 base2 = round(0.5 + 0.5 * sin(6.2 * fragCoord));
  float baseColor = base2.x + base2.y;
  vec4 base4 = vec4(vec3(baseColor), 1.0);
  vec4 base = base4 * vec4(0.50, 0.49, 0.20, 1.0);

  float y = 0.1 * fragCoord.y + 41.0 * iTime;
  float n = pow(sin(y * 0.1) * 0.6 + 0.6, 5.0);
  vec4 stripes = vec4(0.9 * n, 0.3 * n, 0.97 * n, 1.0);

  float noize = pow(rand(iTime / 5000.0 + fragCoord), 6.0);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);
  noize = noise(noize, fragCoord);

  float noize2 = noise(noize, fragCoord);

  fragColor = 0.5 * baseBase + 0.23 * base + (0.9 * noize2 * stripes) + 0.3 * noize;
}

