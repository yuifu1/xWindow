cmake_minimum_required(VERSION 3.27)
project(xWindow C)

# Find X11 package
find_package(X11 REQUIRED)

add_executable(xWindow
        main.c
        nishigori.c
)

# Link X11 and math libraries
target_link_libraries(xWindow ${X11_LIBRARIES} m)
target_include_directories(xWindow PRIVATE ${X11_INCLUDE_DIR})

