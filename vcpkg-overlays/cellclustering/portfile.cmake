vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alxvth/CellClustering
    REF "740e12965e1b6eb937b4c75d2ba68fda9495f164"
    SHA512 b4c02797f9a7fb5841b66499ec8c483246d9d3a84c63138aa0a545612a4cb9466ff5cedf39821951e0e5b7aec5099b3cffa34b785d17a1410f74035d2bb715d1
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