add_executable(blu2usb_test_g02 test_g02.c)
target_link_libraries(blu2usb_test_g02 PRIVATE
    blu2usb_profiles
    blu2usb_ux_model
)
add_test(NAME g02_interaction_ux COMMAND blu2usb_test_g02)

add_executable(blu2usb_test_g03_renderer test_g03_renderer.c)
target_link_libraries(blu2usb_test_g03_renderer PRIVATE
    blu2usb_renderer
    blu2usb_hat
    blu2usb_ux_model
)
add_test(NAME g03_renderer_hat COMMAND blu2usb_test_g03_renderer)

add_executable(blu2usb_test_g04_usb test_g04_usb.c)
target_link_libraries(blu2usb_test_g04_usb PRIVATE
    blu2usb_usb_hid
)
add_test(NAME g04_fixed_usb_hid COMMAND blu2usb_test_g04_usb)

add_executable(blu2usb_test_g05_hid_ownership test_g05_hid_ownership.c)
target_link_libraries(blu2usb_test_g05_hid_ownership PRIVATE
    blu2usb_hid_aggregator
)
add_test(NAME g05_canonical_hid_ownership COMMAND blu2usb_test_g05_hid_ownership)

add_executable(blu2usb_test_g05_ble_mouse test_g05_ble_mouse.c)
target_link_libraries(blu2usb_test_g05_ble_mouse PRIVATE
    blu2usb_ble_hogp
    blu2usb_bt_runtime
    blu2usb_hid_aggregator
    blu2usb_usb_hid
)
add_test(NAME g05_ble_hogp_mouse_passthrough COMMAND blu2usb_test_g05_ble_mouse)

add_executable(blu2usb_test_g06_profiles_remap_hidpp test_g06_profiles_remap_hidpp.c)
target_link_libraries(blu2usb_test_g06_profiles_remap_hidpp PRIVATE
    blu2usb_profiles
    blu2usb_remap
    blu2usb_hid_aggregator
    blu2usb_logitech_hidpp
)
add_test(NAME g06_profiles_remap_hidpp COMMAND blu2usb_test_g06_profiles_remap_hidpp)

add_executable(blu2usb_test_g06_storage test_g06_storage.c)
target_link_libraries(blu2usb_test_g06_storage PRIVATE
    blu2usb_profiles
    blu2usb_storage
)
add_test(NAME g06_product_persistence COMMAND blu2usb_test_g06_storage)

add_executable(blu2usb_test_g06_profile_ui test_g06_profile_ui.c)
target_link_libraries(blu2usb_test_g06_profile_ui PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME g06_profile_ui_regressions COMMAND blu2usb_test_g06_profile_ui)

add_test(
    NAME g06_architecture
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_g06_architecture.py
            ${PROJECT_SOURCE_DIR}
)

add_test(
    NAME screen_contract
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_screen_contract.py
            ${PROJECT_SOURCE_DIR}
)

add_executable(blu2usb_test_hope01_searching_first test_hope01_searching_first.c)
target_link_libraries(blu2usb_test_hope01_searching_first PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope01_searching_first COMMAND blu2usb_test_hope01_searching_first)

add_executable(blu2usb_test_hope02_first_mouse_connected test_hope02_first_mouse_connected.c)
target_link_libraries(blu2usb_test_hope02_first_mouse_connected PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope02_first_mouse_connected COMMAND blu2usb_test_hope02_first_mouse_connected)

add_executable(blu2usb_test_hope08_home_searching test_hope08_home_searching.c)
target_link_libraries(blu2usb_test_hope08_home_searching PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope08_home_searching COMMAND blu2usb_test_hope08_home_searching)

add_executable(blu2usb_test_hope24_home_searching_help test_hope24_home_searching_help.c)
target_link_libraries(blu2usb_test_hope24_home_searching_help PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope24_home_searching_help COMMAND blu2usb_test_hope24_home_searching_help)

add_executable(blu2usb_test_hope09_home_retry test_hope09_home_retry.c)
target_link_libraries(blu2usb_test_hope09_home_retry PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope09_home_retry COMMAND blu2usb_test_hope09_home_retry)

add_test(
    NAME hope09_flow_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope09_flow_source.py
)

add_executable(blu2usb_test_hope25_home_retry_help test_hope25_home_retry_help.c)
target_link_libraries(blu2usb_test_hope25_home_retry_help PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope25_home_retry_help COMMAND blu2usb_test_hope25_home_retry_help)

add_executable(blu2usb_test_hope06_pair_new test_hope06_pair_new.c)
target_link_libraries(blu2usb_test_hope06_pair_new PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope06_pair_new COMMAND blu2usb_test_hope06_pair_new)

add_test(
    NAME hope06_pair_new_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope06_pair_new_source.py
)

add_executable(blu2usb_test_hope26_help_pair_new test_hope26_help_pair_new.c)
target_link_libraries(blu2usb_test_hope26_help_pair_new PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope26_help_pair_new COMMAND blu2usb_test_hope26_help_pair_new)

add_test(
    NAME hope26_help_pair_new_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope26_help_pair_new_source.py
)

add_executable(blu2usb_test_hope07_retry_pair_new test_hope07_retry_pair_new.c)
target_link_libraries(blu2usb_test_hope07_retry_pair_new PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope07_retry_pair_new COMMAND blu2usb_test_hope07_retry_pair_new)

add_test(
    NAME hope07_retry_pair_new_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope07_retry_pair_new_source.py
)

add_executable(blu2usb_test_hope27_help_retry_pair_new test_hope27_help_retry_pair_new.c)
target_link_libraries(blu2usb_test_hope27_help_retry_pair_new PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope27_help_retry_pair_new COMMAND blu2usb_test_hope27_help_retry_pair_new)

add_test(
    NAME hope27_help_retry_pair_new_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope27_help_retry_pair_new_source.py
)

add_executable(blu2usb_test_hope03_home_connected test_hope03_home_connected.c)
target_link_libraries(blu2usb_test_hope03_home_connected PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope03_home_connected COMMAND blu2usb_test_hope03_home_connected)

add_test(
    NAME hope03_home_connected_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope03_home_connected_source.py
)

add_executable(blu2usb_test_hope28_help_home_connected test_hope28_help_home_connected.c)
target_link_libraries(blu2usb_test_hope28_help_home_connected PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope28_help_home_connected COMMAND blu2usb_test_hope28_help_home_connected)

add_test(
    NAME hope28_help_home_connected_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope28_help_home_connected_source.py
)

add_executable(blu2usb_test_hope10_remapper_options test_hope10_remapper_options.c)
target_link_libraries(blu2usb_test_hope10_remapper_options PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope10_remapper_options COMMAND blu2usb_test_hope10_remapper_options)

add_test(
    NAME hope10_remapper_options_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hope10_remapper_options_source.py
)
