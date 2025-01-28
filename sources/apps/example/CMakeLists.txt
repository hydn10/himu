set(target example)

add_executable(${target}
  src/main.cpp
  )

target_include_directories(${target} PRIVATE include)
target_compile_features(${target} PRIVATE cxx_std_23)

target_link_libraries(${target} PRIVATE mylib)
target_link_libraries(${target} PRIVATE STDEXEC::stdexec)

install(TARGETS ${target})
