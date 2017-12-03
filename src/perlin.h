#ifndef PERLIN_H_
#define PERLIN_H_

class Perlin {
 public:
  static float perlin(int x, int y, int z, int w);
  static float octave(int x, int y, int z, int w);
 private:
  static float noise(int x, int y, int z, int w);
};

#endif  // PERLIN_H_
