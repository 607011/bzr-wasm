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

const int WIDTH = 512;
const int HEIGHT = 512;
const int DISHSIZE = WIDTH * HEIGHT;
const float SQRT2INV = 1.f / 1.41421356237f;
const float WEIGHT[9] = {
    SQRT2INV, 1, SQRT2INV,
    1, 1, 1,
    SQRT2INV, 1, SQRT2INV};

float a[WIDTH][HEIGHT][2];
float b[WIDTH][HEIGHT][2];
float c[WIDTH][HEIGHT][2];
int t0 = 0;
int t1 = t0 ^ 1;
uint rgb[DISHSIZE];

extern "C"
{
  void BZR_init();
  void BZR_pour(uint seed);
  void *BZR_rgb_ref();
  void BZR_iterate(float alpha, float beta, float gamma);
  void BZR_convertToRGB();
}

uint rgb2int(float r, float g, float b)
{
  return 0xff000000U | (uint(r * 256) << 16) | (uint(g * 256) << 8) | uint(b * 256);
}

float clamp(float x)
{
  return (x < 0) ? 0 : (x > 1) ? 1 : x;
}

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

void *BZR_rgb_ref()
{
  return (void *)rgb;
}

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
      float weightScale = 0;
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
          weightScale += weight;
        }
      }
      sa /= weightScale;
      sb /= weightScale;
      sc /= weightScale;
      a[x][y][t1] = clamp(sa + sa * (alpha * sb - gamma * sc));
      b[x][y][t1] = clamp(sb + sb * (beta * sc - alpha * sa));
      c[x][y][t1] = clamp(sc + sc * (gamma * sa - beta * sb));
    }
  }
  t0 ^= 1;
  t1 ^= 1;
}
