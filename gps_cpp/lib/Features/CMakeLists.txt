add_subdirectory(DataFeatures)
add_subdirectory(SpatialFeatures)
add_library(Features SHARED Feature-Base.cpp ${DataFeatureSrc} ${SpatialFeatureSrc})

set_target_properties(Features PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)