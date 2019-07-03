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

#ifndef __RNG_HPP__
#define __RNG_HPP__

class RNG
{
private:
  static const unsigned int UINT_RAND_MAX = 0xffffffffU;
  static const unsigned int a = 214013U;
  static const unsigned int b = 2531011U;
  static const unsigned int m = 0xffffffffU;
  unsigned int r;
  void proceed()
  {
    r = (a * r + b) % m;
  }

public:
  explicit RNG(unsigned int seed)
      : r(seed){};
  float getFloat()
  {
    proceed();
    return float(r) / UINT_RAND_MAX;
  }
};

#endif