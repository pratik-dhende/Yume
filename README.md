# Yume (Vulkan)

## Dependencies

| Dependency | Version | 
|-----------|---------|
| Vulkan SDK | 1.4.341.1 |
| CMake     | 3.20+  |
| C++       | 20      |

GLM is automatically downloaded via CMake FetchContent.

## Configure

```
mkdir build && cmake -S . -B build
```

## Build
```
cmake --build build --config [Debug|Release]
```

