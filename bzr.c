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

typedef unsigned char byte;

const int WIDTH = 512;
const int HEIGHT = 512;
const int DISHSIZE = WIDTH * HEIGHT;

static byte cellA[DISHSIZE];
static byte cellB[DISHSIZE];
static byte *cell = (byte *)cellA;
static unsigned int rgb[DISHSIZE];
static unsigned int fullspectrum_rgb[256];
static unsigned int rgb_rgb[256];

unsigned int rgb2int(float r, float g, float b)
{
  return 0xff000000U | ((unsigned int)(r * 256) << 16) | ((unsigned int)(g * 256) << 8) | (unsigned int)(b * 256) << 0;
}

unsigned int hsv2rgb(float h, float s, float v)
{
  while (h >= 360.f)
    h -= 360.f;
  h /= 60;
  const int i = (int)h;
  const float f = h - i;
  const float p = v * (1.f - s);
  if ((i & 1) == 1)
  {
    const float q = v * (1.f - (s * f));
    switch (i)
    {
    case 1:
      return rgb2int(q, v, p);
    case 3:
      return rgb2int(p, q, v);
    case 5:
      return rgb2int(v, p, q);
    }
  }
  else
  {
    const float t = v * (1.f - (s * (1.f - f)));
    switch (i)
    {
    case 0:
      return rgb2int(v, t, p);
    case 2:
      return rgb2int(p, v, t);
    case 4:
      return rgb2int(t, p, v);
    }
  }
  return 0x00000000;
}

void convertToRGB(unsigned int n)
{
  byte *const current = (cell == cellA)
                            ? (byte *)&cellA
                            : (byte *)&cellB;
  for (int i = 0; i < WIDTH * HEIGHT; ++i)
  {
    rgb[i] = rgb_rgb[256 * (unsigned int)current[i] / n];
  }
}

void BZR_pour(unsigned int seed)
{
  unsigned int r = seed;
  const unsigned int a = 13U;
  const unsigned int b = 2531011U;
  cell = cellA;
  for (int i = 0; i < WIDTH * HEIGHT; ++i)
  {
    cell[i] = r % 0xff;
    r = (a * r + b) & 0xffffffffU;
  }
}

byte *BZR_rgb_ref()
{
  return (byte *)rgb;
}

void BZR_init()
{
  // setup color tables
  for (int i = 0; i < 256; ++i)
  {
    {
      const float hue = 360.f * (float)i / 256.f;
      fullspectrum_rgb[i] = hsv2rgb(hue, 1.f, 1.f);
    }
    {
      if (i < 64)
      {
        rgb_rgb[i] = rgb2int(0xff, 0, 0);
      }
      else if (i < 128)
      {
        rgb_rgb[i] = rgb2int(0xff, 0xff, 0);
      }
      else if (i < 192)
      {
        rgb_rgb[i] = rgb2int(0, 0xff, 0xff);
      }
      else
      {
        rgb_rgb[i] = rgb2int(0, 0, 0xff);
      }
    }
  }
}

// One iteration of the Belousov-Zhabotinsky reaction:
// k1: influence of excited cells in neighborhood
// k2: influence of activated cells in neighborhood
// g: speed at which waves travel
// n: max cell value
void BZR_iterate(int k1, int k2, int g, int n)
{
  byte *const next = (cell == cellA)
                         ? (byte *)&cellB
                         : (byte *)&cellA;
  for (int y = 0; y < HEIGHT; ++y)
  {
    for (int x = 0; x < WIDTH; ++x)
    {
      const int c = cell[x + y * WIDTH];
      int S = 0;
      int excited = 0;
      int active = 0;
      for (int yy = y - 1; yy <= y + 1; ++yy)
      {
        for (int xx = x - 1; xx <= x + 1; ++xx)
        {
          int v = cell[(xx + WIDTH) % WIDTH + ((yy + HEIGHT) % HEIGHT) * WIDTH];
          S += v;
          if (v == n)
          {
            ++active;
          }
          else if (v != 0)
          {
            ++excited;
          }
        }
      }
      int c_new;
      if (c == 0)
      {
        c_new = excited / k1 + active / k2;
      }
      else if (c == n)
      {
        c_new = 0;
      }
      else
      {
        c_new = S / (excited + active + 1) + g;
      }
      if (c_new > n)
      {
        c_new = n;
      }
      next[x + y * WIDTH] = c_new;
    }
  }
  convertToRGB(n);
  cell = next;
}
