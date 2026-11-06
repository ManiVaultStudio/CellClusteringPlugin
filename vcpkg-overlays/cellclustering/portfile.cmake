vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alxvth/CellClustering
    REF "31dcdaba40a6e6fcc0fe710457155689c532c163"
    SHA512 d02dc5336210af7e753c6b32137725ec5319976045ed16e471ab8f739bade24db415e8a306453e61493cbecbaf497e5442baffc469941039e03faf1b8bdac280
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS 
        -DCELLCLUSTERING_BUILD_TESTS=OFF
        -DCELLCLUSTERING_BUILD_EXAMPLES=OFF
        -DCELLCLUSTERING_INSTALL=ON
        -DCELLCLUSTERING_BUILD_SHARED_LIBS=ON
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/CellClustering)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")