# Introduction

參考了用 C 語言寫的 pittacus 把他重寫成 C++

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

## template header link error

[???](https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file)

[Solved](https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl)

> Notice that foo-impl.cpp #includes a .cpp file, not a .h file. If that’s confusing, click your heels twice, think of Kansas, and repeat after me, “I will do it anyway even though it’s confusing.” You can trust me on this one. But if you don’t trust me or are simply curious, the rationale is given earlier.
