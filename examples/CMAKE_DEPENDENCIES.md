# CMake Dependency Management

## Overview

The crypto dashboard now has proper CMake dependency checking to ensure all required libraries are installed before building.

## Required Dependencies

### 1. CURL (HTTP Client)
- **CMake Package**: `CURL`
- **Debian Package**: `libcurl4-openssl-dev`
- **Purpose**: HTTP requests to cryptocurrency APIs

### 2. nlohmann-json (JSON Parser)
- **CMake Package**: `nlohmann_json`
- **Debian Package**: `nlohmann-json3-dev`
- **Purpose**: Robust JSON parsing of API responses
- **Version**: 3.0.0 or higher

## CMakeLists.txt Configuration

### examples/CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.25)

# Find required libraries
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.0.0 REQUIRED)

# Crypto dashboard executable
add_executable(crypto_dashboard crypto_dashboard.cpp)

target_link_libraries(crypto_dashboard
    PRIVATE
        epaper
        CURL::libcurl
        nlohmann_json::nlohmann_json
)
```

## Dependency Checking

### Automatic Verification

CMake will automatically verify that both libraries are installed:

```
-- Found CURL: /usr/lib/aarch64-linux-gnu/libcurl.so (found version "8.14.1")
-- Found nlohmann_json: /usr/share/cmake/nlohmann_json/nlohmann_jsonConfig.cmake
   (found suitable version "3.11.3", minimum required is "3.0.0")
```

### Error on Missing Dependencies

If a required library is missing, CMake will fail with a clear error:

```
CMake Error at examples/CMakeLists.txt:5 (find_package):
  Could not find a package configuration file provided by "nlohmann_json"
  with any of the following names:

    nlohmann_jsonConfig.cmake
    nlohmann_json-config.cmake

  Add the installation prefix of "nlohmann_json" to CMAKE_PREFIX_PATH or set
  "nlohmann_json_DIR" to a directory containing one of the above files.
```

## Installation Instructions

### All Dependencies at Once

```bash
sudo apt-get update
sudo apt-get install -y \
    libbcm2835-dev \
    cmake \
    g++-14 \
    libcurl4-openssl-dev \
    nlohmann-json3-dev
```

### Individual Dependencies

```bash
# Core dependencies (required for all examples)
sudo apt-get install libbcm2835-dev cmake g++-14

# Crypto dashboard dependencies
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev
```

## Build Process

### Clean Build

```bash
cd /home/jg/code/e-Paper
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-14
cmake --build . -j$(nproc)
```

### Rebuild After Dependency Changes

```bash
cd /home/jg/code/e-Paper/build
rm -f CMakeCache.txt
cmake .. -DCMAKE_CXX_COMPILER=g++-14
cmake --build . -j$(nproc)
```

## Troubleshooting

### Issue: "Could NOT find CURL"

**Solution:**
```bash
sudo apt-get install libcurl4-openssl-dev
```

### Issue: "Could not find a package configuration file provided by nlohmann_json"

**Solution:**
```bash
sudo apt-get install nlohmann-json3-dev
```

### Issue: CMake version too old

**Error:**
```
CMake 3.16 or higher is required. You are running version 3.13
```

**Solution:**
```bash
sudo apt-get update
sudo apt-get install cmake
# Or download latest CMake from cmake.org
```

### Issue: Compiler doesn't support C++23

**Error:**
```
error: unrecognized command line option '-std=c++23'
```

**Solution:**
```bash
sudo apt-get install g++-14
cmake .. -DCMAKE_CXX_COMPILER=g++-14
```

## Library Details

### CURL

**Version**: 8.14.1 (as detected)
- Thread-safe
- Supports HTTP/HTTPS
- Widely used and stable

**CMake Targets:**
- `CURL::libcurl` - Main library

### nlohmann-json

**Version**: 3.11.3 (as detected)
- Header-only library
- Modern C++ interface
- JSON for Modern C++ standard

**CMake Targets:**
- `nlohmann_json::nlohmann_json` - Header-only target

## Package Availability

Both packages are available in standard Debian/Raspberry Pi OS repositories:

### Debian Trixie (current)
- ✅ `libcurl4-openssl-dev` - Available
- ✅ `nlohmann-json3-dev` - Available (v3.11.3)

### Debian Bookworm
- ✅ `libcurl4-openssl-dev` - Available
- ✅ `nlohmann-json3-dev` - Available (v3.11.2)

### Debian Bullseye
- ✅ `libcurl4-openssl-dev` - Available
- ✅ `nlohmann-json3-dev` - Available (v3.9.1)

## Benefits of Proper Dependency Management

### 1. **Early Error Detection**
CMake fails immediately if dependencies are missing, before compilation starts.

### 2. **Clear Error Messages**
Users know exactly what to install.

### 3. **Version Control**
Can specify minimum versions: `find_package(nlohmann_json 3.0.0 REQUIRED)`

### 4. **Cross-Platform Support**
Works on different Linux distributions and architectures.

### 5. **CI/CD Integration**
Easy to set up automated builds and tests.

## Example Build Output

### Successful Configuration
```
-- The CXX compiler identification is GNU 14.2.0
-- The C compiler identification is GNU 14.2.0
-- Found CURL: /usr/lib/aarch64-linux-gnu/libcurl.so (found version "8.14.1")
-- Found nlohmann_json: /usr/share/cmake/nlohmann_json/nlohmann_jsonConfig.cmake
   (found suitable version "3.11.3", minimum required is "3.0.0")
-- Configuring done (1.8s)
-- Generating done (0.0s)
-- Build files have been written to: /home/jg/code/e-Paper/build
```

### Successful Build
```
[ 84%] Built target epaper
[ 92%] Building CXX object examples/CMakeFiles/crypto_dashboard.dir/crypto_dashboard.cpp.o
[100%] Linking CXX executable crypto_dashboard
[100%] Built target crypto_dashboard
```

## Documentation Updates

Updated documentation to include dependency information:

1. **README.md** - Added optional dependencies section
2. **examples/CRYPTO_DASHBOARD_README.md** - Installation instructions
3. **CMAKE_DEPENDENCIES.md** - This file

## Future Improvements

### Potential Enhancements

1. **Automatic Dependency Installation**
   - Create setup script
   - Check and install missing packages

2. **Version Compatibility Matrix**
   - Test with different library versions
   - Document minimum/maximum versions

3. **Alternative JSON Libraries**
   - Support multiple JSON parsers
   - Make nlohmann-json optional with fallback

4. **Docker Support**
   - Dockerfile with all dependencies
   - Easy cross-platform development

## Conclusion

Proper CMake dependency management ensures:
- ✅ **Reliable Builds**: Dependencies verified before compilation
- ✅ **Clear Errors**: Users know what's missing
- ✅ **Easy Setup**: Simple apt-get install commands
- ✅ **Maintainability**: Easy to add new dependencies
- ✅ **Documentation**: Clear requirements for users

The build system now properly checks for all required libraries and provides helpful error messages when they're missing!

