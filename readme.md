!! Project currently hard-paths to `~/Documents/LuauBytecode` for loading file `bytecode`, which is expected to be a binary output of `luau-compile --binary script.luau > bytecode` 

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

Cmake will build everything within ./build, leaving ./external and ./src untouched.
The binary output for RayLuau will be found at ./build/bin/RayLuau

## 3. Run

Execute the RayLuau binary via shell.