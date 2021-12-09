# Introduction

![](https://i.imgur.com/Jw9vy7V.gif)

參考了用 C 語言寫的 [pittacus](https://github.com/izeigerman/pittacus) 用 Boost 把他重寫成 C++
構想是想要做類似像 redis-cluster 的簡化版
使用 Gossip 協議講每台主機狀態同步
除了減輕伺服器請求負擔外
若是一台伺服器壞掉 其他伺服器還可以使用

# Techniques

- Cmake
- Boost
  - asio
  - serialize
- C++11 template, smart_ptr, virtual function ...
- Vcpkg

# Todo

- Visualize
- Unit test
- completely error handleing

# Senario

- 以穩定性大於高效率

# Problems

## boost serialization vcpkg cmake link undefined symbol

[Solved](https://github.com/microsoft/vcpkg/issues/4481#issuecomment-503912053)

```=
find_package(Boost REQUIRED COMPONENTS system serialization)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(my_lib ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY} bcrypt...)
```

## boost serialization derived class and pointer

[Solved](https://theboostcpplibraries.com/boost.serialization-class-hierarchies)

## template header link error

[???](https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file)

[Solved](https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl)

[Solved](http://fcamel-life.blogspot.com/2013/09/c-template.html)

> Notice that foo-impl.cpp #includes a .cpp file, not a .h file. If that’s confusing, click your heels twice, think of Kansas, and repeat after me, “I will do it anyway even though it’s confusing.” You can trust me on this one. But if you don’t trust me or are simply curious, the rationale is given earlier.
