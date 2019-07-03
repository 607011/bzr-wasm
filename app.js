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

(function (window) {
  'use strict';

  const DEFAULT_K1 = 3;
  const DEFAULT_K2 = 1;
  const DEFAULT_G = 14;
  const DEFAULT_N = 125;
  const memory = new WebAssembly.Memory({ initial: 256, maximum: 256 });
  let exports = null;
  let canvas = null;
  let k1 = DEFAULT_K1;
  let k2 = DEFAULT_K2;
  let g = DEFAULT_G;
  let n = DEFAULT_N;
  let t0 = 0;

  const WIDTH = 512;
  const HEIGHT = 512;
  const DISH_SIZE = WIDTH * HEIGHT;
  const DOTSIZE = 1;
  let ctx = null;
  let iterations = 0;
  let iterationsEl = null;
  let fpsEl = null;

  async function createWebAssembly(path, importObject) {
    const result = await window.fetch(path);
    const bytes = await result.arrayBuffer();
    return WebAssembly.instantiate(bytes, importObject);
  }

  const update = t => {
    fpsEl.innerText = `${(1000 / (t - t0)).toFixed(1)} fps`;
    t0 = t;
    ++iterations;
    iterationsEl.innerText = iterations;
    exports._BZR_iterate(k1, k2, g, n);
    const rgbBuffer = new Uint8ClampedArray(memory.buffer, exports._BZR_rgb_ref(), 4 * DISH_SIZE);
    const img = new ImageData(rgbBuffer, canvas.width, canvas.height);
    ctx.putImageData(img, 0, 0);
    requestAnimationFrame(update);
  };

  const initCanvas = () => {
    canvas = document.getElementById('canvas');
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    canvas.style.width = `${WIDTH * DOTSIZE}px`;
    canvas.style.height = `${HEIGHT * DOTSIZE}px`;
    ctx = canvas.getContext('2d');
  };

  const pour = () => {
    const seed = (Math.random() * 2 ** 30) | 0;
    exports._BZR_pour(seed);
  };

  async function init() {
    const env = {
      'abortStackOverflow': () => { throw new Error('overflow'); },
      'table': new WebAssembly.Table({ initial: 0, maximum: 0, element: 'anyfunc' }),
      '__table_base': 0,
      'memory': memory,
      '__memory_base': 1024,
      'STACKTOP': 0,
      'STACK_MAX': memory.buffer.byteLength,
    };
    const importObject = { env };
    const wa = await createWebAssembly('bzr.wasm', importObject);
    exports = wa.instance.exports;
    exports._BZR_init();

    iterationsEl = document.getElementById('iterations');
    fpsEl = document.getElementById('fps');
    document.getElementById('k1').addEventListener('change', event => {
      k1 = parseInt(event.target.value);
      document.getElementById('k1-value').innerText = k1;
    });
    document.getElementById('k2').addEventListener('change', event => {
      k2 = parseInt(event.target.value);
      document.getElementById('k2-value').innerText = k2;
    });
    document.getElementById('g').addEventListener('change', event => {
      g = parseInt(event.target.value);
      document.getElementById('g-value').innerText = g;
    });
    document.getElementById('n').addEventListener('change', event => {
      n = parseInt(event.target.value);
      document.getElementById('n-value').innerText = n;
    });
    document.getElementById('k1').value = DEFAULT_K1;
    document.getElementById('k2').value = DEFAULT_K2;
    document.getElementById('g').value = DEFAULT_G;
    document.getElementById('n').value = DEFAULT_N;
    document.getElementById('k1-value').innerText = DEFAULT_K1;
    document.getElementById('k2-value').innerText = DEFAULT_K2;
    document.getElementById('g-value').innerText = DEFAULT_G;
    document.getElementById('n-value').innerText = DEFAULT_N;
    initCanvas();
    restart();
  }

  const restart = () => {
    iterations = 0;
    pour();
    requestAnimationFrame(update);
  };

  window.addEventListener('load', init);
})(window);
