# WASM

## Desc

Test integration of webassembly with cmake. Example with pure cpp.

## Usage

See https://stunlock.gg/posts/emscripten_with_cmake/

```shell
# Build
> mkdir build && cd build
> source /path/to/emsdk/emsdk_env.sh
# https://github.com/emscripten-core/emscripten/issues/10078
> emcmake cmake ..
> make

# If use an archive instead of exe:
> ar x libcalc.a
> wasm-objdump -x calc.cpp.o
> emrun --browser=firefox *.html (see the computations in the developer console)
```

Open `calc.html` in firefox (In firefox: security.fileuri.strict_origin_policy)

For qml exploration, you will have to use the following options: `-DCMAKE_FIND_ROOT_PATH=/ -DQt5_DIR=/path/to/qt5config.cmake` (and qt libs as archive)
