# Sets cache variable QBT_ADDITONAL_FLAGS and QBT_ADDITONAL_CXX_FLAGS to list of additional
# compiler flags for C and C++ (QBT_ADDITONAL_FLAGS) and for C++ only (QBT_ADDITONAL_CXX_FLAGS)
# and appends them to CMAKE_XXX_FLAGS variables.

# It could use add_compile_options(), but then it is needed to use generator expressions,
# and most interesting of them are not compatible with Visual Studio :(

macro(qbt_set_compiler_options)
# if (NOT QBT_ADDITONAL_FLAGS)
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
        #-Wshadow -Wconversion ?
        set(_GCC_COMMON_C_AND_CXX_FLAGS "-Wall -Wextra"
            "-Wcast-qual -Wcast-align"
            "-Winvalid-pch -Wno-long-long"
            #"-fstack-protector-all"
            "-Werror -Wno-error=deprecated-declarations"
        )
        set(_GCC_COMMON_CXX_FLAGS "-fexceptions -frtti"
            "-Woverloaded-virtual -Wold-style-cast"
            "-Wnon-virtual-dtor -Wfloat-equal -Wcast-qual -Wcast-align"
            "-Werror=overloaded-virtual"
    # 		"-Weffc++"
            "-Wno-error=cpp"
        )

        # GCC 4.8 has problems with std::array and its initialization
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
            list(APPEND _GCC_COMMON_CXX_FLAGS "-Wno-error=missing-field-initializers")
        endif()

        include(CheckCXXCompilerFlag)
        # check for -pedantic
        check_cxx_compiler_flag(-pedantic _PEDANTIC_IS_SUPPORTED)
        if (_PEDANTIC_IS_SUPPORTED)
            list(APPEND _GCC_COMMON_CXX_FLAGS "-pedantic -pedantic-errors")
        else (_PEDANTIC_IS_SUPPORTED)
            list(APPEND _GCC_COMMON_CXX_FLAGS "-Wpedantic")
        endif (_PEDANTIC_IS_SUPPORTED)

        check_cxx_compiler_flag(-Wformat-security _WFORMAT_SECURITY_IS_SUPPORTED)
        if (_WFORMAT_SECURITY_IS_SUPPORTED)
            list(APPEND _GCC_COMMON_C_AND_CXX_FLAGS -Wformat-security)
        endif()

        if (CMAKE_SYSTEM_NAME MATCHES Linux)
            add_definitions(-D_DEFAULT_SOURCE)
        endif()

        # Clang 5.0 still doesn't support -Wstrict-null-sentinel
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            check_cxx_compiler_flag(-Wstrict-null-sentinel _STRICT_NULL_SENTINEL_IS_SUPPORTED)
            if (_STRICT_NULL_SENTINEL_IS_SUPPORTED)
                list(APPEND _GCC_COMMON_CXX_FLAGS "-Wstrict-null-sentinel")
            endif (_STRICT_NULL_SENTINEL_IS_SUPPORTED)

            # Code should be improved to render this unneeded
            list(APPEND _GCC_COMMON_CXX_FLAGS "-Wno-error=unused-function -Wno-error=inconsistent-missing-override")
        else ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
            # GCC supports it
            list(APPEND _GCC_COMMON_CXX_FLAGS "-Wstrict-null-sentinel")
        endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

        string(REPLACE ";" " " _GCC_COMMON_C_AND_CXX_FLAGS_STRING "${_GCC_COMMON_C_AND_CXX_FLAGS}")
        string(REPLACE ";" " " _GCC_COMMON_CXX_FLAGS_STRING "${_GCC_COMMON_CXX_FLAGS}")

        string(APPEND CMAKE_C_FLAGS " ${_GCC_COMMON_C_AND_CXX_FLAGS_STRING}")
        string(APPEND CMAKE_CXX_FLAGS " ${_GCC_COMMON_C_AND_CXX_FLAGS_STRING} ${_GCC_COMMON_CXX_FLAGS_STRING}")

        # check whether we can enable -Og optimization for debug build
        check_cxx_compiler_flag(-Og _DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)

        if (_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
            set(QBT_ADDITONAL_FLAGS "-Og -g3 -pipe" CACHE STRING
                "Additional qBittorent compile flags")
            set(QBT_ADDITONAL_CXX_FLAGS "-Og -g3 -pipe" CACHE STRING
                "Additional qBittorent C++ compile flags")
        else(_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
            set(QBT_ADDITONAL_FLAGS "-O0 -g3 -pipe" CACHE STRING
                "Additional qBittorent compile flags")
            set(QBT_ADDITONAL_CXX_FLAGS "-O0 -g3 -pipe" CACHE STRING
                "Additional qBittorent C++ compile flags")
        endif (_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
    endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(QBT_ADDITONAL_FLAGS "/wd4251 /wd4275 /wd4290  /W4" CACHE STRING "Additional qBittorent compile flags")
    endif ()

    string(APPEND CMAKE_C_FLAGS " ${QBT_ADDITONAL_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${QBT_ADDITONAL_FLAGS}")

# endif (NOT QBT_ADDITONAL_FLAGS)
endmacro(qbt_set_compiler_options)
