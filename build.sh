#!/bin/sh

em++ \
  -O3 \
  -s WASM=1 \
  -s ONLY_MY_CODE=1 \
  -s EXPORTED_FUNCTIONS="['_BZR_init', '_BZR_iterate', '_BZR_pour', '_BZR_rgb_ref', '_BZR_convertToRGB']" \
  -o bzr.wasm \
  bzr.cpp
