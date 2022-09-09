cmake_minimum_required(VERSION 3.11)

project(devdump LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
