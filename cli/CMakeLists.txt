add_executable(ft
    main.cpp
    cmd_lib.cpp
    cmd_pic.cpp
)

target_link_libraries(ft PRIVATE ft_lib)

# stb_image needed in cli for PNG I/O
target_include_directories(ft PRIVATE ${CMAKE_SOURCE_DIR}/lib/vendor)

if(MSVC)
    target_compile_options(ft PRIVATE /W3 /WX-)
else()
    target_compile_options(ft PRIVATE -Wall -Wextra)
endif()
