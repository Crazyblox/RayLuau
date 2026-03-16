!! Project has only been validated to compile on macOS Sequoia. Compile on Windows/Linux at your own discretion.

## Setup:

### 1. Clone

```bash
git clone https://github.com/Crazyblox/RayLuau --recurse-submodules
cd RayLuau
```

--recurse-submodules can be followed up if you already cloned the repo:
```bash
git submodules update --init --recursive
```

### 2. Build

```bash
cmake -S . -B build
cmake --build build
```

Cmake will build everything within `./build`, leaving `./extern` and `./src` untouched.

The binary output for RayLuau will be found at `./build/bin/RayLuau`.

### 3. Run

Execute the RayLuau binary via shell.

RayLuau makes use of `script.luau` in the same directory; this is an example script for running in RayLuau.

Feel free to modify it and make use of the `raylib` library's functions in Luau.