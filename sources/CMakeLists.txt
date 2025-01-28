if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(mylib)
add_subdirectory(apps)
