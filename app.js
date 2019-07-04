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

  const DEFAULT_ALPHA = 1.2;
  const DEFAULT_BETA = 1.0;
  const DEFAULT_GAMMA = 1.0;
  const WIDTH = 512;
  const HEIGHT = 512;
  const DISH_SIZE = WIDTH * HEIGHT;
  let t0 = 0;
  let ctx = null;
  let iterations = 0;
  let iterationsEl = null;
  let fpsEl = null;
  let memory = null;
  let exports = null;
  let canvas = null;
  let alpha = +localStorage.getItem('bzr.alpha') || DEFAULT_ALPHA;
  let beta = +localStorage.getItem('bzr.beta') || DEFAULT_BETA;
  let gamma = +localStorage.getItem('bzr.gamma') || DEFAULT_GAMMA;

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
    exports._BZR_iterate(alpha, beta, gamma);
    exports._BZR_convertToRGB();
    const rgbBuffer = new Uint8ClampedArray(memory.buffer, exports._BZR_rgb_ref(), 4 * DISH_SIZE);
    const img = new ImageData(rgbBuffer, canvas.width, canvas.height);
    ctx.putImageData(img, 0, 0);
    requestAnimationFrame(update);
  };

  const initCanvas = () => {
    canvas = document.getElementById('canvas');
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    canvas.style.width = `${WIDTH}px`;
    canvas.style.height = `${HEIGHT}px`;
    canvas.style.clipPath = `circle(${WIDTH/2}px at center)`;
    ctx = canvas.getContext('2d');
  };

  const pour = () => {
    const seed = (Math.random() * 2 ** 30) | 0;
    exports._BZR_pour(seed);
  };

  async function init() {
    memory = new WebAssembly.Memory({ initial: 256, maximum: 256 });
    const env = { 'memory': memory };
    const importObject = { env };
    const wa = await createWebAssembly('bzr.wasm', importObject);
    exports = wa.instance.exports;

    iterationsEl = document.getElementById('iterations');
    fpsEl = document.getElementById('fps');
    document.getElementById('alpha').addEventListener('change', event => {
      alpha = +event.target.value;
      localStorage.setItem('bzr.alpha', alpha);
    });
    document.getElementById('beta').addEventListener('change', event => {
      beta = +event.target.value;
      localStorage.setItem('bzr.beta', beta);
    });
    document.getElementById('gamma').addEventListener('change', event => {
      gamma = +event.target.value;
      localStorage.setItem('bzr.gamma', gamma);
    });
    document.getElementById('alpha').value = alpha;
    document.getElementById('beta').value = beta;
    document.getElementById('gamma').value = gamma;
    document.getElementById('reset-dish-button').addEventListener('click', () => {
      iterations = 0;
      pour();
    });
    initCanvas();
    pour();
    requestAnimationFrame(update);
  }

  window.addEventListener('load', init);
})(window);
