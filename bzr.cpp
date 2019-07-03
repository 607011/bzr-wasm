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

typedef unsigned char byte;
typedef unsigned int uint;

const int WIDTH = 512;
const int HEIGHT = 512;
const int DISHSIZE = WIDTH * HEIGHT;

static float a[WIDTH][HEIGHT][2];
static float b[WIDTH][HEIGHT][2];
static float c[WIDTH][HEIGHT][2];
static int p = 0;
static int q = 1;
static uint rgb[DISHSIZE];

extern "C"
{
  void BZR_init();
  void BZR_pour(uint seed);
  byte *BZR_rgb_ref();
  void BZR_iterate(float alpha, float beta, float gamma);
  void BZR_convertToRGB();
}

uint rgb2int(float r, float g, float b)
{
  return 0xff000000U | (uint(r * 256) << 16) | (uint(g * 256) << 8) | uint(b * 256) << 0;
}

void BZR_convertToRGB()
{
  for (int x = 0; x < WIDTH; ++x)
  {
    for (int y = 0; y < HEIGHT; ++y)
    {
      rgb[x + y * WIDTH] = rgb2int(a[x][y][p], b[x][y][p], c[x][y][p]);
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
  p = 0;
  q = 1;
}

byte *BZR_rgb_ref()
{
  return (byte *)rgb;
}

float constrain(float x)
{
  return (x < 0) ? 0 : (x > 1) ? 1 : x;
}

// One iteration of the Belousov-Zhabotinsky reaction:
// k1: influence of excited cells in neighborhood
// k2: influence of activated cells in neighborhood
// g: speed at which waves travel
// n: max cell value
void BZR_iterate(float alpha, float beta, float gamma)
{
  for (int x = 0; x < WIDTH; x++)
  {
    for (int y = 0; y < HEIGHT; y++)
    {
      float sa = 0;
      float sb = 0;
      float sc = 0;
      for (int i = x - 1; i <= x + 1; ++i)
      {
        for (int j = y - 1; j <= y + 1; ++j)
        {
          sa += a[(i + WIDTH) % WIDTH]
                 [(j + HEIGHT) % HEIGHT][p];
          sb += b[(i + WIDTH) % WIDTH]
                 [(j + HEIGHT) % HEIGHT][p];
          sc += c[(i + WIDTH) % WIDTH]
                 [(j + HEIGHT) % HEIGHT][p];
        }
      }
      sa /= 9;
      sb /= 9;
      sc /= 9;
      a[x][y][q] = constrain(sa + sa * (alpha * sb - gamma * sc));
      b[x][y][q] = constrain(sb + sb * (beta * sc - alpha * sa));
      c[x][y][q] = constrain(sc + sc * (gamma * sa - beta * sb));
    }
  }
  if (p == 0)
  {
    p = 1;
    q = 0;
  }
  else
  {
    p = 0;
    q = 1;
  }
}
