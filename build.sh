#!/bin/sh

emcc \
  -s WASM=1 \
  -s ONLY_MY_CODE=1 \
  -s EXPORTED_FUNCTIONS="['_BZR_init', '_BZR_iterate', '_BZR_pour', '_BZR_rgb_ref']" \
  -o bzr.js \
  bzr.c