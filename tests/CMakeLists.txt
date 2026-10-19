if (NOT SS4S_ENABLE_TESTS)
    return()
endif ()

enable_testing()

message(STATUS "Configuring SS4S tests")
set(TEST_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../modules/interface/include)
set(TEST_LIBRARIES pthread)

function(ss4s_add_test NAME)
    cmake_parse_arguments(TEST "" "" "SOURCES;ARGS;LIBRARIES" ${ARGN})
    set(SS4S_TEST_CASE "ss4s_test_${NAME}")
    add_executable(${SS4S_TEST_CASE} ${TEST_SOURCES})
    add_test(${NAME} ${SS4S_TEST_CASE} ${TEST_ARGS})
    target_include_directories(${SS4S_TEST_CASE} PRIVATE ${TEST_INCLUDES})
    target_link_libraries(${SS4S_TEST_CASE} PRIVATE ss4s ${TEST_LIBRARIES})
    # Existing tests use assert() for both checks and setup calls. Keep them
    # active when exercising the Release library, otherwise setup disappears
    # and concurrency tests wait forever instead of testing anything.
    if (MSVC)
        target_compile_options(${SS4S_TEST_CASE} PRIVATE /UNDEBUG)
    else ()
        target_compile_options(${SS4S_TEST_CASE} PRIVATE -UNDEBUG)
    endif ()
    set_target_properties(${SS4S_TEST_CASE} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")
    set_tests_properties(${NAME} PROPERTIES SKIP_RETURN_CODE 127)
    set_tests_properties(${NAME} PROPERTIES TIMEOUT 30)
    set(SS4S_TEST_CASE ${SS4S_TEST_CASE} PARENT_SCOPE)
endfunction()

configure_file(../samples/av-threaded/sample.pcm ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(../samples/av-threaded/sample.h264 ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

ss4s_add_test(initialization SOURCES test_initialization.c)

ss4s_add_test(feed_guard SOURCES test_feed_guard.c)

add_executable(ss4s_test_stats test_stats.c)
target_include_directories(ss4s_test_stats PRIVATE ../src)
add_test(NAME stats COMMAND ss4s_test_stats)

ss4s_add_test(video_diagnostics_legacy SOURCES test_video_diagnostics.c ARGS dummy)
if (TARGET ss4s-module-ndl-webos5 AND NOT SS4S_TARGET_WEBOS)
    ss4s_add_test(video_diagnostics_ndl SOURCES test_video_diagnostics.c ARGS ndl-webos5)
    if (SS4S_NDL_DROP_STALE_FRAMES)
        foreach (MODE success retry reject)
            ss4s_add_test(ndl_render_drop_${MODE} SOURCES test_ndl_render_drop.c ARGS ${MODE})
        endforeach ()
    endif ()
endif ()

ss4s_add_test(video_concurrent_lifecycle_dummy SOURCES test_video_concurrent_lifecycle.c ARGS dummy)

ss4s_add_test(pcm_playback_sdl SOURCES test_pcm_playback.c ARGS sdl)
ss4s_add_test(pcm_playback_alsa SOURCES test_pcm_playback.c ARGS alsa)
ss4s_add_test(pcm_playback_pulse SOURCES test_pcm_playback.c ARGS pulse)

ss4s_add_test(pcm_playback_mmal SOURCES test_h264_playback.c ARGS mmal)

ss4s_add_test(h264_playback_lgnc SOURCES test_h264_playback.c ARGS lgnc)
