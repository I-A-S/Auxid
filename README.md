<div align="center">
  <img src="logo.png" alt="Auxid Logo" width="190" style="border-radius: 1.15rem;"/>
  <br/>
  
  <img src="https://img.shields.io/badge/license-apache_v2-blue.svg" alt="License"/>
  <img src="https://img.shields.io/badge/standard-C%2B%2B20-yellow.svg" alt="C++ Standard"/>
  <img src="https://img.shields.io/badge/compiler-clang_only-red.svg" alt="Compiler"/>

  <p style="padding-top: 0.2rem;">
    <b>Auxid: The Orthodox C++ Platform.</b>
  </p>
</div>

## **The Vision**

Auxid is a platform for building modern, high-performance C++ applications using the principles of Orthodox C++ and Data-Oriented Design (DOD).

Modern C++ has become bogged down by massive template metaprogramming, glacial compile times, and a Standard Library (STL) whose node-based containers and rigid allocator model actively fight against CPU caches and Data-Oriented Design. Auxid strips the language back to its highly-performant, close-to-metal roots.

LibAuxid completely replaces the C++ Standard Library. It compiles in a purely freestanding environment (`-nostdlib++`, `-ffreestanding`, `-fno-exceptions`) and provides a hyper-lean, DOD-friendly template library built around explicit heap and arena memory allocators.

### Core Features
* Zero STL Overhead: No `<iostream>`, no `<vector>`, no hidden allocations.

* World-Class Allocators: Integrated rpmalloc for lightning-fast, thread-caching heap allocations, alongside custom Arena allocators.

* Cache-Friendly Containers: Custom Sparse-Dense HashMap, Small-String Optimized (SSO) String, and strictly aligned VecT implementations.

* Safe Error Handling: Union-based Result<T, E> and Option<T> types that compile down to trivial registers, with Rust-like AU_TRY macros.

* Strictly Clang: Built exclusively for Clang (Linux/macOS) and Clang-CL (Windows) to guarantee predictable ABIs and optimal code generation.

## **The Ecosystem**

The Auxid platform is split across multiple repositories for modularity and maintenance. This repository is the home of LibAuxid (the core template library).

| Name            | Description                            | Repo                                     |
|-----------------|----------------------------------------|------------------------------------------|
| **LibAuxid**        | Auxid custom template library and core platform | https://github.com/I-A-S/Auxid           |
| **Validator** | Clang-based static analysis and validator tool   | https://github.com/I-A-S/Auxid-Validator |
| **VSCode**    | Official VSCode Extension for Auxid integration            | https://github.com/I-A-S/Auxid-VSCode    |
| **CLI**       | Command Line Utility for project management         | https://github.com/I-A-S/Auxid-CLI       |
| **Project Template**       | Easy to use production-ready template for scaffolding new Auxid projects         | https://github.com/I-A-S/Auxid-Project-Template       |

## **Quick Start (CMake Integration)**

Auxid is designed to be highly adaptable. You can drop LibAuxid into any existing CMake pipeline using FetchContent.

> [!NOTE]
> **Opt-In Bare-Metal Configuration**
>
> To remain unobtrusive to standard developer workflows, Auxid does not apply strict, platform-specific compiler or linker flags (such as `-nostdlib++`, `-ffreestanding`, or `-fno-exceptions`) by default.
>
> However, if your goal is to write absolute "Orthodox C++" and maximize platform performance, you can explicitly link your targets against `auxid_platform`. Doing so automatically applies the optimal close-to-metal configuration flags for your environment.

> [!WARNING]
> **STL Access is Disabled**
>
> Linking against `auxid_platform` configures your target for close-to-metal execution, which completely removes access to the C++ Standard Template Library (STL). Consequently, your project—and any external dependencies—cannot rely on STL features.
> * **Existing Projects:** We strongly advise against linking `auxid_platform` in established codebases. If your project or its external libraries depend on the STL, this will break your build.
>
> * **New Projects:** We highly recommend this target for new, resource-constrained projects where maximum CPU and memory efficiency are critical, provided you can comfortably build without STL dependencies.

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyOrthodoxEngine CXX)

include(FetchContent)

FetchContent_Declare(
  auxid
  GIT_REPOSITORY https://github.com/I-A-S/Auxid.git
  GIT_TAG        main # Replace with a specific release tag
)
FetchContent_MakeAvailable(auxid)

auxid_setup_project() # (OPTIONAL) Sets up cmake project settings

add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE libauxid)
```

### Example Usage

```cpp
#include <auxid/containers/vec.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

auto main() -> int 
{
    Vec<String> names;
    names.push_back(String("Orthodox"));
    names.push_back(String("C++"));

    return 0;
}
```

## **License**

Copyright (C) 2026 IAS. Licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).
