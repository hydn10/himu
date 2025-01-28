set(target mylib)

add_library(${target} INTERFACE)

target_compile_features (${target} INTERFACE cxx_std_23)
