set(TARGET_PLUGIN "DespillAP")

# Boost
set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.70.0 REQUIRED COMPONENTS filesystem system thread)
message("##################################")
message("Using Boost ${Boost_VERSION_STRING}")
message("Boost include dir: ${Boost_INCLUDE_DIRS}")
message("Boost library dir: ${Boost_LIBRARY_DIRS}")
message("##################################")

# Modules
add_library(${TARGET_PLUGIN} MODULE
    DespillAP.cpp
    Threading.cpp
)

# Alias
add_library(NukePlugins::${TARGET_PLUGIN} ALIAS ${TARGET_PLUGIN})

# Includes
target_include_directories(${TARGET_PLUGIN}
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${NUKE_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Case Nuke 14
if (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 14.0)
    target_compile_definitions(${TARGET_PLUGIN} PRIVATE NOMINMAX _USE_MATH_DEFINES)
endif()

if (WIN32)
    target_compile_definitions(${TARGET_PLUGIN}
        PRIVATE
            BOOST_ALL_NO_LIB
            _WIN32_WINNT=0x0501
            WIN32_LEAN_AND_MEAN
    )
endif()

# Link Nuke, Boost y CUDA
target_link_libraries(${TARGET_PLUGIN}
    PRIVATE
        ${NUKE_DDIMAGE_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
)

# Post-build
add_custom_command(TARGET ${TARGET_PLUGIN} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${TARGET_PLUGIN}>
        "${PLUGIN_OUTPUT_DIR}/"
)

# Install
install(TARGETS ${TARGET_PLUGIN} DESTINATION .)
