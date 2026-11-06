cmake_minimum_required(VERSION 3.22)

# vcpkg library settings
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_LIST_DIR}/vcpkg-overlays")

if(WIN32 AND NOT DEFINED VCPKG_LIBRARY_LINKAGE)
    set(VCPKG_HOST_TRIPLET "x64-windows-static-md" CACHE STRING "")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
    set(VCPKG_LIBRARY_LINKAGE "static" CACHE STRING "")
endif()

# -----------------------------------------------------------------------------
# UMAP Analysis Plugin
# -----------------------------------------------------------------------------
set(CELLCLUSTERINGPLUGIN "CellClusteringPlugin")
PROJECT(${CELLCLUSTERINGPLUGIN} 
        VERSION 0.1.0
        DESCRIPTION "A ManiVault plugin that wraps alxvth/CellClustering"
        LANGUAGES CXX)

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /Zc:preprocessor /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets Concurrent REQUIRED)
find_package(ManiVault COMPONENTS Core PointData CONFIG QUIET)

find_package(OpenMP REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS graph)
find_package(CellClustering CONFIG REQUIRED)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
# Define the plugin sources
set(CELLCLUSTERING_PLUGIN
    src/CellClusteringPlugin.h
    src/CellClusteringPlugin.cpp
)

set(CELLCLUSTERING_SETTINGS
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/KnnSettingsAction.h
    src/KnnSettingsAction.cpp
    src/AdvancedSettingsAction.h
    src/AdvancedSettingsAction.cpp
    src/ClusterColorSettingsAction.h
    src/ClusterColorSettingsAction.cpp
)

set(CELLCLUSTERING_AUX
    PluginInfo.json
)

set(CELLCLUSTERING_SOURCES
    ${CELLCLUSTERING_PLUGIN}
    ${CELLCLUSTERING_SETTINGS}
)

source_group(Plugin FILES ${CELLCLUSTERING_PLUGIN})
source_group(Settings FILES ${CELLCLUSTERING_SETTINGS})
source_group(Aux FILES ${CELLCLUSTERING_AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
# Create dynamic library for the plugin
add_library(${CELLCLUSTERINGPLUGIN} SHARED ${CELLCLUSTERING_SOURCES} ${CELLCLUSTERING_AUX})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault headers, including system data plugins
target_include_directories(${CELLCLUSTERINGPLUGIN}
    PRIVATE 
        "${ManiVault_INCLUDE_DIR}"
)

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++20
target_compile_features(${CELLCLUSTERINGPLUGIN} PRIVATE cxx_std_20)

set_target_properties(${CELLCLUSTERINGPLUGIN} PROPERTIES 
    AUTOMOC ON
    UNITY_BUILD ${MV_UNITY_BUILD}
)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${CELLCLUSTERINGPLUGIN} 
    PRIVATE 
        Qt6::Widgets
        Qt6::WebEngineWidgets
        Qt6::Concurrent
        ManiVault::Core
        ManiVault::PointData
        ManiVault::ClusterData
        CellClustering::CellClustering
)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
# Install the shared plugin library to the "Plugins" folder in the ManiVault install directory
install(TARGETS ${CELLCLUSTERINGPLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGIN # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGIN # Linux/Mac .so
)

add_custom_command(TARGET ${CELLCLUSTERINGPLUGIN} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --component PLUGIN
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

mv_handle_plugin_config(${CELLCLUSTERINGPLUGIN})

#get_target_property(CELL_CLUSTERING_LIB_DEBUG CellClustering::CellClustering IMPORTED_LOCATION_DEBUG)
#get_target_property(CELL_CLUSTERING_LIB_RELEASE CellClustering::CellClustering IMPORTED_LOCATION_RELEASE)

#set(CELL_CLUSTERING_LIB
#    "$<IF:$<CONFIG:Debug>,${CELL_CLUSTERING_LIB_DEBUG},${CELL_CLUSTERING_LIB_RELEASE}>"
#)

#set(DEPENDENCIES_FOLDERS "${CELL_CLUSTERING_LIB}")
set(DEPENDENCIES_FOLDERS "")
mv_install_dependencies(${CELLCLUSTERINGPLUGIN})

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${CELLCLUSTERINGPLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${CELLCLUSTERINGPLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()
