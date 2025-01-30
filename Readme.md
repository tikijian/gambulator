# Gameboy emulator in C

Just a learning project.

### Requirements
Add SDL 3.2.0 to `./libs/SDL3` folder

### Build

First-time build for SDL

```bash
cmake -S . -B build
cmake --build build
```

Rebuild
```bash
cmake ./ -B build
```

Recompile
```bash
cd build
make
```

Run
```bash
build/gambulator
```