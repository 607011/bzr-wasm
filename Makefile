export EMCC_DEBUG=1

bzr.wasm.js: bzr.cpp
	em++ --bind --std=c++11 bzr.cpp -s WASM=1 -s ALLOW_MEMORY_GROWTH=1  -s ONLY_MY_CODE=1 -s EXPORTED_FUNCTIONS="['_BZR_iterate', '_BZR_pour', '_BZR_rgb_ref', '_BZR_convertToRGB']" -o bzr.wasm

clean:
	rm *.wasm
