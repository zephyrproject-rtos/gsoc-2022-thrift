# SPDX-License-Identifier: Apache-2.0

find_program(THRIFT_EXECUTABLE thrift)
find_package(PkgConfig REQUIRED)
pkg_search_module(THRIFT REQUIRED thrift)

message(STATUS "Found thrift executable: ${THRIFT_EXECUTABLE}")
message(STATUS "Found pkg-config executable: ${PKGCONFIG_EXECUTABLE}")

function(thrift
    target          # CMake target (for dependencies / headers)
    lang            # The language for generated sources
    lang_opts       # Language options (e.g. ':no_skeleton')
    out_dir         # Output directory for generated files
                    # (do not include 'gen-cpp', etc)
    source_file     # The .thrift source file
    options         # Additional thrift options

                    # Generated files in ${ARGN}
    )
  add_custom_command(
    OUTPUT ${ARGN}
    COMMAND
    ${THRIFT_EXECUTABLE}
    --gen ${lang}${lang_opts}
    -o ${out_dir}
    "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}"
    ${options}
    DEPENDS ${source_file}
    )

    target_include_directories(${target} PRIVATE ${out_dir}/gen-${lang})
    target_include_directories(${target} PRIVATE ${THRIFT_INCLUDE_DIRS})
endfunction()
