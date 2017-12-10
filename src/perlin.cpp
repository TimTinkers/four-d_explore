#include "perlin.h"

#include <math.h>

float Perlin::noise(int x, int y, int z, int w) {
  int v = x + y * 57 + z * 9371 + w * 10903;
  v = (v << 13) ^ v;
  return (1.0f -
          ((v * (v * v* 15731 + 789221) + 1376312589) & 0x7fffffff) /
              1073741824.0);
}

// If we want noise at arbitrary position, this must be changed to interpolate
// between values.
float Perlin::perlin(int x, int y, int z, int w) { return noise(x, y, z, w); }

float Perlin::octave(int x, int y, int z, int w) {
  float total = 0.0f;
  float persistance = 0.5f;
  for (int i = 0; i < 8; ++i) {
    int freq = (int)pow(2.0f, i);
    float amplitude = pow(persistance, i);
    total += perlin(x * freq, y * freq, z * freq, w * freq) * amplitude;
  }
  return total;
}
