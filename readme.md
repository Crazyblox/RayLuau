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

Make use of the following instructions based on your given platform.

For debugging, config cmake's build type to `RelWithDebInfo`.

Windows:
```bash
cmake . -B build
cmake --build build --config Release
```

MacOS:
```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Cmake will build everything to the `./build` directory. `./extern` and `./src` will remain untouched.

The binary output for RayLuau will be found at `./build/bin/RayLuau`.

### 3. Run

RayLuau executes `/init.luau` located via the provided `RAYLUAU_SCRIPT_DIR` define in `main.cpp`.

You can replace the scripts in here with any other scripts, as long as `/init.luau` remains present.

Keep in mind that this runtime's requirer can not detect symlinks, so avoid doing anything fancy file-wise with your scripts directory.