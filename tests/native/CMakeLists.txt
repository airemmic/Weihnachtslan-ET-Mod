# Link the complete production game module into a native test executable.
# No gameplay implementation is duplicated or replaced by test stubs.
get_target_property(ETLAB_GAME_SOURCES qagame SOURCES)
get_target_property(ETLAB_GAME_INCLUDES qagame INCLUDE_DIRECTORIES)
add_executable(etlab_loadout_tests etlab_loadouts.c ${ETLAB_GAME_SOURCES})
target_include_directories(etlab_loadout_tests PRIVATE ${ETLAB_GAME_INCLUDES})
get_target_property(ETLAB_GAME_LIBRARIES qagame LINK_LIBRARIES)
get_target_property(ETLAB_GAME_DEFINITIONS qagame COMPILE_DEFINITIONS)
target_link_libraries(etlab_loadout_tests PRIVATE ${ETLAB_GAME_LIBRARIES})
target_compile_definitions(etlab_loadout_tests PRIVATE ${ETLAB_GAME_DEFINITIONS})
# The German mod catalog must translate every ETLab award text.
target_compile_definitions(etlab_loadout_tests PRIVATE ETLAB_LOCALE_DE="${CMAKE_SOURCE_DIR}/etmain/locale/mod/de.po")
add_test(NAME etlab_loadouts COMMAND etlab_loadout_tests)
