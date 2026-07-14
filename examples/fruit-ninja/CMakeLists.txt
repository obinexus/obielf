cmake_minimum_required(VERSION 3.21)

project(fruit_ninja_with_obielf VERSION 0.1.0 LANGUAGES C)

include(CTest)

option(FRUIT_OBIELF_EMBED_ASSETS
       "Load assets from a linkable OBIELF resource pack" ON)
option(FRUIT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

set(FRUIT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(FRUIT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(FRUIT_PACK "${FRUIT_GENERATED_DIR}/fruit_assets.obielf")
set(FRUIT_EMBED_C "${FRUIT_GENERATED_DIR}/fruit_assets_embed.c")
set(FRUIT_EMBED_H "${FRUIT_GENERATED_DIR}/fruit_assets_embed.h")

file(MAKE_DIRECTORY "${FRUIT_GENERATED_DIR}")
file(GLOB FRUIT_RESOURCE_INPUTS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png")

function(fruit_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(FRUIT_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        if(FRUIT_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

add_library(obielf_resource STATIC
    src/obielf_resource.c
)
target_include_directories(obielf_resource PUBLIC "${FRUIT_INCLUDE_DIR}")
fruit_enable_warnings(obielf_resource)

add_executable(obielf-pack
    tools/obielf_pack.c
)
target_link_libraries(obielf-pack PRIVATE obielf_resource)
fruit_enable_warnings(obielf-pack)

add_custom_command(
    OUTPUT "${FRUIT_PACK}" "${FRUIT_EMBED_C}" "${FRUIT_EMBED_H}"
    COMMAND "$<TARGET_FILE:obielf-pack>"
            --root "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            --manifest "${CMAKE_CURRENT_SOURCE_DIR}/assets/manifest.txt"
            --output "${FRUIT_PACK}"
            --c-output "${FRUIT_EMBED_C}"
            --header-output "${FRUIT_EMBED_H}"
            --symbol fruit_assets_obielf
    DEPENDS
        obielf-pack
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/manifest.txt"
        ${FRUIT_RESOURCE_INPUTS}
    COMMENT "Packing PNG resources into fruit_assets.obielf"
    VERBATIM
)

add_custom_target(obielf-assets ALL
    DEPENDS "${FRUIT_PACK}" "${FRUIT_EMBED_C}" "${FRUIT_EMBED_H}")

add_library(fruit_assets_linkable STATIC "${FRUIT_EMBED_C}")
add_dependencies(fruit_assets_linkable obielf-assets)
target_include_directories(fruit_assets_linkable PUBLIC "${FRUIT_GENERATED_DIR}")
set_target_properties(fruit_assets_linkable PROPERTIES
    OUTPUT_NAME fruit_assets_obielf)
fruit_enable_warnings(fruit_assets_linkable)

add_library(fruit_game_core STATIC
    src/game.c
)
target_include_directories(fruit_game_core PUBLIC "${FRUIT_INCLUDE_DIR}")
if(NOT MSVC)
    target_link_libraries(fruit_game_core PUBLIC m)
endif()
fruit_enable_warnings(fruit_game_core)

add_executable(fruit-ninja-obielf
    src/main.c
    src/resources.c
)
target_compile_definitions(fruit-ninja-obielf PRIVATE SDL_MAIN_HANDLED)
target_include_directories(fruit-ninja-obielf PRIVATE
    "${FRUIT_INCLUDE_DIR}"
    "${FRUIT_GENERATED_DIR}")
target_link_libraries(fruit-ninja-obielf PRIVATE
    fruit_game_core
    obielf_resource
    SDL2::SDL2
    SDL2_image::SDL2_image)

if(FRUIT_OBIELF_EMBED_ASSETS)
    target_compile_definitions(fruit-ninja-obielf PRIVATE
        FRUIT_OBIELF_EMBED_ASSETS=1)
    target_link_libraries(fruit-ninja-obielf PRIVATE fruit_assets_linkable)
else()
    target_compile_definitions(fruit-ninja-obielf PRIVATE
        FRUIT_OBIELF_EMBED_ASSETS=0)
endif()

if(NOT MSVC)
    target_link_libraries(fruit-ninja-obielf PRIVATE m)
endif()

fruit_enable_warnings(fruit-ninja-obielf)

add_custom_command(TARGET fruit-ninja-obielf POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:fruit-ninja-obielf>/assets"
    COMMENT "Copying direct-path assets beside the executable")

if(BUILD_TESTING)
    add_executable(resource-pack-test tests/resource_pack_test.c)
    target_include_directories(resource-pack-test PRIVATE "${FRUIT_INCLUDE_DIR}")
    target_link_libraries(resource-pack-test PRIVATE obielf_resource)
    add_dependencies(resource-pack-test obielf-assets)
    fruit_enable_warnings(resource-pack-test)

    add_executable(game-logic-test tests/game_logic_test.c)
    target_include_directories(game-logic-test PRIVATE "${FRUIT_INCLUDE_DIR}")
    target_link_libraries(game-logic-test PRIVATE fruit_game_core)
    fruit_enable_warnings(game-logic-test)

    add_test(NAME resource-pack
        COMMAND resource-pack-test "${FRUIT_PACK}")
    add_test(NAME pack-inspection
        COMMAND obielf-pack --inspect "${FRUIT_PACK}")
    add_test(NAME game-logic COMMAND game-logic-test)
    add_test(NAME native-assets
        COMMAND fruit-ninja-obielf --verify-assets)
    add_test(NAME native-smoke
        COMMAND fruit-ninja-obielf
                --smoke-test 180
                --screenshot "${CMAKE_CURRENT_BINARY_DIR}/fruit-smoke.bmp")

    if(FRUIT_OBIELF_EMBED_ASSETS)
        set(FRUIT_NATIVE_TEST_DIRECTORY "${FRUIT_GENERATED_DIR}")
    else()
        set(FRUIT_NATIVE_TEST_DIRECTORY
            "$<TARGET_FILE_DIR:fruit-ninja-obielf>")
    endif()
    set_tests_properties(native-assets native-smoke PROPERTIES
        WORKING_DIRECTORY "${FRUIT_NATIVE_TEST_DIRECTORY}"
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
endif()
