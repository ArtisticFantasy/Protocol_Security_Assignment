add_executable(main main.cpp)

target_link_libraries(main Features TBB::tbb)

set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)