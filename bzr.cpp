// Copyright (c) 2019 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#include "rng.hpp"

typedef unsigned int uint;

// WIDTH and HEIGHT determine the size of the "dish" in pixels.
const int WIDTH = 512;
const int HEIGHT = 512;
const int DISHSIZE = WIDTH * HEIGHT;

// The BZR algorithm as described by Tomeu et al uses the values
// of the surrounding cells to calculate the value of the currently
// processed cell. The diagonal values are divided by the square root
// of 2 to better resemble the actual reaction.
const float SQRT2INV = 1.f / 1.41421356237f;
const float WEIGHT[9] = {
    SQRT2INV, 1, SQRT2INV,
    1, 1, 1,
    SQRT2INV, 1, SQRT2INV};
// WEIGHTSUM contains the sum of the factors listed in WEIGHT.
const float WEIGHTSUM = 7.8284271247523802f;

// a, b and c contain the concentration of each reagent in each cell.
// The last index determines whether the value belongs to the current
// or the next iteration (see t0 and t1).
float a[WIDTH][HEIGHT][2];
float b[WIDTH][HEIGHT][2];
float c[WIDTH][HEIGHT][2];

// t0 and t1 contain the index for the current and the next "dish".
int t0 = 0;
int t1 = t0 ^ 1;

// rgb contains the RGBA data of the generated image. JavaScript code
// can access the data contained herein to draw the image on a canvas.
uint rgb[DISHSIZE];

extern "C"
{
  // The following functions will be exported so that they can be
  // used in JavaScript code.
  void BZR_init();
  void BZR_pour(uint seed);
  void *BZR_rgb_ref();
  void BZR_iterate(float alpha, float beta, float gamma);
  void BZR_convertToRGB();
}

// rgb2int() converts the given red, green and blue value (in the
// interval [0..1]) to a 32 bit unsigned integer value).
// Alpha always is 0xff.
uint rgb2int(float r, float g, float b)
{
  return 0xff000000U | (uint(r * 0xff) << 16) | (uint(g * 0xff) << 8) | uint(b * 0xff);
}

// clamp() forces the given value into the interval [0..1].
float clamp(float x)
{
  return (x < 0) ? 0 : (x > 1) ? 1 : x;
}

// BZR_convertToRGB() converts the concentration values to
// RGBA image data.
void BZR_convertToRGB()
{
  for (int x = 0; x < WIDTH; ++x)
  {
    for (int y = 0; y < HEIGHT; ++y)
    {
      const float u = a[x][y][t0];
      const float v = b[x][y][t0];
      const float w = c[x][y][t0];
      rgb[x + y * WIDTH] = rgb2int(u, v, w);
    }
  }
}

// BZR_pour() fills the "dish" with random values.
void BZR_pour(uint seed)
{
  RNG rng(seed);
  for (int x = 0; x < WIDTH; ++x)
  {
    for (int y = 0; y < HEIGHT; ++y)
    {
      a[x][y][0] = rng.getFloat();
      b[x][y][0] = rng.getFloat();
      c[x][y][0] = rng.getFloat();
    }
  }
  t0 = 0;
  t1 = 1;
}

// BZR_rgb_ref() returns a pointer to the RGBA data of the generated
// image. In JavaScript code this pointer can be used to populate a
// Uint8Array.
void *BZR_rgb_ref()
{
  return (void *)rgb;
}

// BZR_iterate() implements one iteration of the
// Belousov-Zhabotinsky reaction as described by Tomeu et al.
void BZR_iterate(float alpha, float beta, float gamma)
{
  for (int x = 0; x < WIDTH; x++)
  {
    for (int y = 0; y < HEIGHT; y++)
    {
      float sa = 0;
      float sb = 0;
      float sc = 0;
      int weightIdx = 0;
      for (int i = x - 1; i <= x + 1; ++i)
      {
        for (int j = y - 1; j <= y + 1; ++j)
        {
          const int ii = (i + WIDTH) % WIDTH;
          const int jj = (j + HEIGHT) % HEIGHT;
          const float weight = WEIGHT[weightIdx];
          sa += weight * a[ii][jj][t0];
          sb += weight * b[ii][jj][t0];
          sc += weight * c[ii][jj][t0];
          ++weightIdx;
        }
      }
      sa /= WEIGHTSUM;
      sb /= WEIGHTSUM;
      sc /= WEIGHTSUM;
      a[x][y][t1] = clamp(sa + sa * (alpha * sb - gamma * sc));
      b[x][y][t1] = clamp(sb + sb * (beta * sc - alpha * sa));
      c[x][y][t1] = clamp(sc + sc * (gamma * sa - beta * sb));
    }
  }
  t0 ^= 1;
  t1 ^= 1;
}
