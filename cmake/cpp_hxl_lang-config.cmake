include(CMakePackageConfigHelpers)

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

add_library(cpp_hxl_lang STATIC IMPORTED)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_hxl_lang-Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_hxl_lang-ConfigVersion.cmake"
        DESTINATION lib/cmake/cpp_hxl_lang
)

set_target_properties(cpp_hxl_lang PROPERTIES
        IMPORTED_LOCATION_DEBUG "${PACKAGE_PREFIX_DIR}/build/debug/lib/libcpp_hxl_lang.a"
        IMPORTED_LOCATION_RELEASE "${PACKAGE_PREFIX_DIR}/build/release/lib/libcpp_hxl_lang.a"
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
)
