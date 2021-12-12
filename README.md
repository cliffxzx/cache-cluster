# Introduction

cache-cluster is built by react, redux and WebExtension, please read shopback/react-webextension-boilerplate before you start to develop Whale.

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

## Constructor vs casting operator

https://softwareengineering.stackexchange.com/questions/269709/constructor-vs-casting-operator

## why stl without split implement and header c++

## [When should I not split my code into header and source files?](https://stackoverflow.com/questions/7805748/when-should-i-not-split-my-code-into-header-and-source-files)

## [Effective C++: Overview](https://silverfoxkkk.pixnet.net/blog/post/57487944)

## [how stl use template split hpp cpp c++](https://www.codeproject.com/Articles/48575/How-to-define-a-template-class-in-a-h-file-and-imp)

## [Should function declarations include parameter names? [closed]](https://stackoverflow.com/questions/7891526/should-function-declarations-include-parameter-names)

## [C++20: Useful concepts: Requiring type T to be derived from a base class](https://oopscenities.net/2021/07/13/c20-useful-concepts-requiring-type-t-to-be-derived-from-a-base-class/)

## [Cmake](https://zhuanlan.zhihu.com/p/93895403)
