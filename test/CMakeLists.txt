file(GLOB TEST_SRC
    *.cpp
)

add_executable(${PROJECT_NAME}_test
    test1.cpp
)

target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
    ${PROJECT_NAME}
    pthread
)
