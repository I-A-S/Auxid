<div align="center">
  <img src="logo.png" alt="Auxid Logo" width="190" style="border-radius: 1.15rem;"/>
  <br/>
  
  <img src="https://img.shields.io/badge/license-apache_v2-darkblue.svg" alt="License"/>
  <img src="https://img.shields.io/badge/standard-C%2B%2B20-darkred.svg" alt="C++ Standard"/>
  <img src="https://img.shields.io/badge/compiler-MSVC | Clang-darkgreen.svg" alt="Compiler"/>

  <p style="padding-top: 0.2rem;">
    <b>Auxid: The Orthodox C++ Platform.</b>
  </p>
</div>

## **The Vision**

Auxid is a platform for building modern, high-performance C++ applications using the principles of Orthodox C++ and Data-Oriented Design (DOD).

Modern C++ has become bogged down by massive template metaprogramming, glacial compile times, and a Standard Library (STL) whose node-based containers and rigid allocator model actively fight against CPU caches and Data-Oriented Design. Auxid strips the language back to its highly-performant, close-to-metal roots.

LibAuxid augments the C++ Standard Library with a hyper-lean, DOD-friendly template layer built on explicit heap and arena allocators. Where the standard library is already the right tool (for example, `std::filesystem`) LibAuxid forwards it through a thin, LibAuxid-compatible inline wrapper with no extra overhead.

### Core Features
* Zero STL Overhead: No `<iostream>`, no `<vector>`, no hidden allocations.

* World-Class Allocators: Integrated rpmalloc for lightning-fast, thread-caching heap allocations, alongside custom Arena allocators.

* Cache-Friendly Containers: Custom Sparse-Dense HashMap, Small-String Optimized (SSO) String, and strictly aligned VecT implementations.

* Safe Error Handling: Union-based Result<T, E> and Option<T> types that compile down to trivial registers, with Rust-like AU_TRY macros.

## **The Ecosystem**

The Auxid platform is split across two repositories for modularity and maintenance. This repository is the home of LibAuxid (the core template library).

| Name            | Description                            | Repo                                     |
|-----------------|----------------------------------------|------------------------------------------|
| **LibAuxid**        | Auxid custom template library and core platform | https://github.com/I-A-S/Auxid           |
| **Project Template**       | Easy to use production-ready template for scaffolding new Auxid projects         | https://github.com/I-A-S/Auxid-Project-Template       |

## **Quick Start (CMake Integration)**

Auxid is designed to be highly adaptable. You can drop LibAuxid into any existing CMake pipeline using FetchContent.

> [!NOTE]
> **Opt-In Platform Configurations**
>
> To remain unobtrusive to standard developer workflows, Auxid does not force strict compiler or linker flags by default. However, to help you write predictable "Orthodox C++", we provide an opt-in configuration that you can explicitly link your targets against:
>
> **`auxid_platform_standard` (Recommended):** Disables C++ exceptions (`-fno-exceptions` or `/EHs-c-`). We highly recommend linking this for all projects using Auxid to ensure predictable control flow and performance.

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyOrthodoxEngine CXX)

include(FetchContent)

FetchContent_Declare(
  auxid
  GIT_REPOSITORY [https://github.com/I-A-S/Auxid.git](https://github.com/I-A-S/Auxid.git)
  GIT_TAG        main # Use a specific release tag instead for best stability
)
FetchContent_MakeAvailable(auxid)

auxid_setup_project() # (OPTIONAL) Sets up cmake project settings (e.g. Setting C++ Standard to 20)

add_executable(my_app main.cpp)

# Link the core library
target_link_libraries(my_app PRIVATE libauxid)

# (Recommended) Opt-in to the standard Orthodox C++ configuration
target_link_libraries(my_app PRIVATE auxid_platform_standard)
```

### Example Usage

```cpp
#include <auxid/containers/vec.hpp>
#include <auxid/containers/string.hpp>

using namespace au;

auto main() -> int 
{
    auxid::MainThreadGuard _main_thread_guard;

    // Custom Auxid containers present an *almost* fully identical
    // interface to its std counterparts.
    Vec<String> names;
    names.push_back(String("Orthodox"));
    names.push_back(String("C++"));

    return 0;
}
```

## **License**

Copyright (C) 2026 I-A-S. Licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).
