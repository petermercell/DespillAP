# NukePlugin.cmake - CMake functions for building Nuke plugins

if(NOT NUKE_FOUND)
    message(FATAL_ERROR "Nuke must be found before including NukePlugin.cmake")
endif()

# Platform-specific settings
if(APPLE)
    set(NUKE_PLUGIN_EXTENSION ".dylib")
    set(NUKE_PLUGIN_PREFIX "")
elseif(WIN32)
    set(NUKE_PLUGIN_EXTENSION ".dll")
    set(NUKE_PLUGIN_PREFIX "")
else() # Linux
    set(NUKE_PLUGIN_EXTENSION ".so")
    set(NUKE_PLUGIN_PREFIX "")
endif()

# Common compile definitions for Nuke plugins
set(NUKE_COMPILE_DEFINITIONS
    USE_GLEW
    NDEBUG
    _GNU_SOURCE
)

# Platform-specific compile definitions
if(UNIX AND NOT APPLE)
    list(APPEND NUKE_COMPILE_DEFINITIONS LINUX)
elseif(APPLE)
    list(APPEND NUKE_COMPILE_DEFINITIONS MACOS)
elseif(WIN32)
    list(APPEND NUKE_COMPILE_DEFINITIONS WINDOWS)
endif()

# Common compile flags
set(NUKE_COMPILE_FLAGS
    -fPIC
    -DUSE_GLEW
)

# Function to add a Nuke plugin (simplified approach that works)
function(add_plugin PLUGIN_NAME)
    set(sources ${ARGN})
    add_library(${PLUGIN_NAME} MODULE ${sources})
    add_library(NukePlugins::${PLUGIN_NAME} ALIAS ${PLUGIN_NAME})
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${NUKE_DDIMAGE_LIBRARY})
    target_include_directories(${PLUGIN_NAME} PRIVATE 
        ${NUKE_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # Add essential compile definitions
    target_compile_definitions(${PLUGIN_NAME} PRIVATE 
        FN_PLUGIN
        USE_GLEW
        NDEBUG
        _GNU_SOURCE
    )
    
    if(NUKE_VERSION_MAJOR AND NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 14)
        target_compile_definitions(${PLUGIN_NAME} PRIVATE NOMINMAX _USE_MATH_DEFINES)
    endif()
    
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        PREFIX ""
        SUFFIX "${NUKE_PLUGIN_EXTENSION}"
        POSITION_INDEPENDENT_CODE ON
    )
    
    # Configurar directorio de salida según la plataforma
    if(WIN32)
        # Windows: poner en /build/lib/Release/
        set_target_properties(${PLUGIN_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Release"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Release"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
        )
    else()
        # Linux/macOS: poner directamente en /build/lib/
        set_target_properties(${PLUGIN_NAME} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        )
    endif()
    
    # Platform-specific settings
    if(UNIX AND NOT APPLE)
        target_link_libraries(${PLUGIN_NAME} PRIVATE dl pthread)
    elseif(WIN32)
        target_compile_definitions(${PLUGIN_NAME} PRIVATE 
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()
    
    # Custom command para asegurar que el archivo se copie (backup method)
    if(WIN32)
        # Windows: copiar a /build/lib/Release/
        add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lib/Release"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "$<TARGET_FILE:${PLUGIN_NAME}>" 
                "${CMAKE_BINARY_DIR}/lib/Release/"
            COMMENT "Copying ${PLUGIN_NAME} to ${CMAKE_BINARY_DIR}/lib/Release/"
        )
        set(OUTPUT_MESSAGE "${CMAKE_BINARY_DIR}/lib/Release/")
    else()
        # Linux/macOS: copiar a /build/lib/
        add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lib"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                "$<TARGET_FILE:${PLUGIN_NAME}>" 
                "${CMAKE_BINARY_DIR}/lib/"
            COMMENT "Copying ${PLUGIN_NAME} to ${CMAKE_BINARY_DIR}/lib/"
        )
        set(OUTPUT_MESSAGE "${CMAKE_BINARY_DIR}/lib/")
    endif()
    
    message(STATUS "Plugin ${PLUGIN_NAME} configured successfully")
    message(STATUS "  Output directory: ${OUTPUT_MESSAGE}")
endfunction()

# Function to set common Nuke plugin properties
function(set_nuke_plugin_properties TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        PREFIX "${NUKE_PLUGIN_PREFIX}"
        SUFFIX "${NUKE_PLUGIN_EXTENSION}"
        POSITION_INDEPENDENT_CODE ON
    )
endfunction()

# Print Nuke configuration info
message(STATUS "Nuke Configuration:")
message(STATUS "  Include dirs: ${NUKE_INCLUDE_DIRS}")
message(STATUS "  Library dirs: ${NUKE_LIBRARY_DIRS}")
message(STATUS "  DDImage library: ${NUKE_DDIMAGE_LIBRARY}")
if(NUKE_RIPFRAMEWORK_LIBRARY)
    message(STATUS "  RIPFramework library: ${NUKE_RIPFRAMEWORK_LIBRARY}")
else()
    message(STATUS "  RIPFramework library: NOT FOUND (minimal installation)")
endif()
if(NUKE_EXECUTABLE)
    message(STATUS "  Executable: ${NUKE_EXECUTABLE}")
else()
    message(STATUS "  Executable: NOT FOUND (dev container)")
endif()
message(STATUS "  Plugin extension: ${NUKE_PLUGIN_EXTENSION}")
if(NUKE_VERSION_MAJOR)
    message(STATUS "  Version: ${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}.${NUKE_VERSION_RELEASE}")
endif()

add_plugin(DespillAP DespillAP.cpp)