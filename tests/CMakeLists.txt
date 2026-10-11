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

add_executable(blu2usb_test_hope_remapper_flow_consolidated
    test_hope_remapper_flow_consolidated.c)
target_link_libraries(blu2usb_test_hope_remapper_flow_consolidated PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(
    NAME hope_remapper_flow_consolidated
    COMMAND blu2usb_test_hope_remapper_flow_consolidated
)

add_test(
    NAME hope_remapper_flow_consolidated_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope_remapper_flow_consolidated_source.py
)

add_executable(blu2usb_test_hope04_saved_devices test_hope04_saved_devices.c)
target_link_libraries(blu2usb_test_hope04_saved_devices PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope04_saved_devices COMMAND blu2usb_test_hope04_saved_devices)

add_test(
    NAME hope04_saved_devices_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope04_saved_devices_source.py
)

add_executable(blu2usb_test_hope05_remove_this test_hope05_remove_this.c)
target_link_libraries(blu2usb_test_hope05_remove_this PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME hope05_remove_this COMMAND blu2usb_test_hope05_remove_this)

add_test(
    NAME hope05_remove_this_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope05_remove_this_source.py
)

add_executable(blu2usb_test_hope30_help_remove_this
    test_hope30_help_remove_this.c)
target_link_libraries(blu2usb_test_hope30_help_remove_this PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(
    NAME hope30_help_remove_this
    COMMAND blu2usb_test_hope30_help_remove_this
)

add_test(
    NAME hope30_help_remove_this_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope30_help_remove_this_source.py
)

add_executable(blu2usb_test_hope23_remove_learn_home
    test_hope23_remove_learn_home.c)
target_link_libraries(blu2usb_test_hope23_remove_learn_home PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(
    NAME hope23_remove_learn_home
    COMMAND blu2usb_test_hope23_remove_learn_home
)

add_test(
    NAME hope23_remove_learn_home_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope23_remove_learn_home_source.py
)

add_executable(blu2usb_test_hope31_final_inventory
    test_hope31_final_inventory.c)
target_link_libraries(blu2usb_test_hope31_final_inventory PRIVATE
    blu2usb_ux_model
)
add_test(
    NAME hope31_final_inventory
    COMMAND blu2usb_test_hope31_final_inventory
)

add_test(
    NAME hope31_final_inventory_source
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test_hope31_final_inventory_source.py
)

add_executable(blu2usb_test_mov01_connected_entry test_mov01_connected_entry.c)
target_link_libraries(blu2usb_test_mov01_connected_entry PRIVATE blu2usb_renderer blu2usb_ux_model)
add_test(NAME mov01_connected_entry COMMAND blu2usb_test_mov01_connected_entry)

add_executable(blu2usb_test_mov02_math test_mov02_math.c)
target_link_libraries(blu2usb_test_mov02_math PRIVATE blu2usb_movement)
add_test(NAME mov02_vector_math COMMAND blu2usb_test_mov02_math)

add_executable(blu2usb_test_mov03_complete_flow test_mov03_complete_flow.c)
target_link_libraries(blu2usb_test_mov03_complete_flow PRIVATE blu2usb_renderer blu2usb_ux_model)
add_test(NAME mov03_complete_flow COMMAND blu2usb_test_mov03_complete_flow)

add_executable(blu2usb_test_mov02_horizontal_slow test_mov02_horizontal_slow.c)
target_link_libraries(blu2usb_test_mov02_horizontal_slow PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mov02_horizontal_slow_physical_flow COMMAND blu2usb_test_mov02_horizontal_slow)

add_executable(blu2usb_test_mov03_horizontal_fast test_mov03_horizontal_fast.c)
target_link_libraries(blu2usb_test_mov03_horizontal_fast PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mov03_fast_dpi_and_saved_devices_tones COMMAND blu2usb_test_mov03_horizontal_fast)

add_executable(blu2usb_test_mov04_vertical_slow test_mov04_vertical_slow.c)
target_link_libraries(blu2usb_test_mov04_vertical_slow PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
    blu2usb_movement
)
add_test(NAME mov04_vertical_slow_physical_flow COMMAND blu2usb_test_mov04_vertical_slow)

add_executable(blu2usb_test_mov05_vertical_fast test_mov05_vertical_fast.c)
target_link_libraries(blu2usb_test_mov05_vertical_fast PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
    blu2usb_movement
)
add_test(NAME mov05_vertical_fast_physical_flow COMMAND blu2usb_test_mov05_vertical_fast)

add_executable(blu2usb_test_mov06_rotation test_mov06_rotation.c)
target_link_libraries(blu2usb_test_mov06_rotation PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
    blu2usb_movement
)
add_test(NAME mov06_rotation_physical_flow COMMAND blu2usb_test_mov06_rotation)

add_executable(blu2usb_test_mov07_home_connected test_mov07_home_connected.c)
target_link_libraries(blu2usb_test_mov07_home_connected PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
    blu2usb_movement
)
add_test(NAME mov07_connected_home_four_routes COMMAND blu2usb_test_mov07_home_connected)

add_test(
    NAME mov07_pico_ble_home_runtime_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mov07_home_runtime_source.py
)

add_executable(blu2usb_test_mov08_saved_devices test_mov08_saved_devices.c)
target_link_libraries(blu2usb_test_mov08_saved_devices PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mov08_saved_devices_identity_and_values COMMAND blu2usb_test_mov08_saved_devices)

add_executable(blu2usb_test_mov09_remove_this test_mov09_remove_this.c)
target_link_libraries(blu2usb_test_mov09_remove_this PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mov09_remove_this_bond_scoped_flow COMMAND blu2usb_test_mov09_remove_this)

add_test(
    NAME mov09_pico_removal_runtime_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mov09_remove_runtime_source.py
)

add_executable(blu2usb_test_mov10_all_help test_mov10_all_help.c)
target_link_libraries(blu2usb_test_mov10_all_help PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mov10_all_help_copy_and_any_key_back COMMAND blu2usb_test_mov10_all_help)

# MOD-02 — six combinations pages + navigation and no-op reset boundary.
add_executable(blu2usb_test_mod02_combinations test_mod02_combinations.c)
target_link_libraries(blu2usb_test_mod02_combinations PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
add_test(NAME mod02_combinations_physical_flow COMMAND blu2usb_test_mod02_combinations)
add_test(NAME mod02_combinations_source_contract
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod02_combinations_source.py
            ${PROJECT_SOURCE_DIR}
)

# MOD-03: 30 action picker/roundtrip with v2-safe migration and commit-first UX.
add_executable(blu2usb_test_mod03_shortcuts test_mod03_shortcuts.c)
target_link_libraries(blu2usb_test_mod03_shortcuts PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
    blu2usb_profiles
    blu2usb_storage
)
add_test(NAME mod03_shortcuts_flow_and_migration COMMAND blu2usb_test_mod03_shortcuts)
add_test(NAME mod03_shortcuts_source_contract
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod03_shortcuts_source.py
            ${PROJECT_SOURCE_DIR}
)

# MOD-04: exclusive physical modifier, reversible UNAVAILABLE and legacy v3.
add_executable(blu2usb_test_mod04_modifier test_mod04_modifier.c)
target_link_libraries(blu2usb_test_mod04_modifier PRIVATE
    blu2usb_profiles
    blu2usb_ux_model
    blu2usb_renderer
    blu2usb_remap
)
# The production workflow configures Release (-DNDEBUG). Keep new gate
# assertions active so test runs actually evaluate the physical and
# storage invariants rather than compiling those checks away.
target_compile_options(blu2usb_test_mod02_combinations PRIVATE -UNDEBUG)
target_compile_options(blu2usb_test_mod03_shortcuts PRIVATE -UNDEBUG)
target_compile_options(blu2usb_test_mod04_modifier PRIVATE -UNDEBUG)
add_test(NAME mod04_modifier_exclusivity_and_masks COMMAND blu2usb_test_mod04_modifier)
add_test(NAME mod04_modifier_source_contract
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod04_modifier_source.py)

# MOD-04 physical corrective candidate: 4 operator-reported UI regressions.
add_executable(blu2usb_test_mod04_ui_bugfix test_mod04_ui_bugfix.c)
target_link_libraries(blu2usb_test_mod04_ui_bugfix PRIVATE
    blu2usb_renderer
    blu2usb_ux_model
)
target_compile_options(blu2usb_test_mod04_ui_bugfix PRIVATE -UNDEBUG)
add_test(NAME mod04_operator_ui_regressions COMMAND blu2usb_test_mod04_ui_bugfix)

add_executable(blu2usb_test_mod05_buttons test_mod05_buttons.c)
target_link_libraries(blu2usb_test_mod05_buttons PRIVATE blu2usb_modifier_shortcuts blu2usb_hid_aggregator blu2usb_usb_hid)
target_compile_options(blu2usb_test_mod05_buttons PRIVATE -UNDEBUG)
add_test(NAME mod05_button_chords_usb_backpressure COMMAND blu2usb_test_mod05_buttons)
add_test(NAME mod05_runtime_source_contract COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod05_runtime_source.py)

# MOD-06: wheel detents, directional MOVEs in post-rotation pre-gain coordinates.
add_executable(blu2usb_test_mod06_gestures test_mod06_gestures.c)
target_link_libraries(blu2usb_test_mod06_gestures PRIVATE
    blu2usb_modifier_shortcuts blu2usb_movement blu2usb_hid_aggregator)
target_compile_options(blu2usb_test_mod06_gestures PRIVATE -UNDEBUG)
add_test(NAME mod06_wheel_rotation_gestures COMMAND blu2usb_test_mod06_gestures)
add_test(NAME mod06_runtime_before_movement_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod06_runtime_source.py)

# MOD-07: 22 Linux-first Keyboard usage/chord mappings, USB ACK and 6KRO ownership.
add_executable(blu2usb_test_mod07_keyboard_catalog test_mod07_keyboard_catalog.c)
target_link_libraries(blu2usb_test_mod07_keyboard_catalog PRIVATE
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator)
target_compile_options(blu2usb_test_mod07_keyboard_catalog PRIVATE -UNDEBUG)
add_test(NAME mod07_keyboard_catalog_22_actions
    COMMAND blu2usb_test_mod07_keyboard_catalog)
add_test(NAME mod07_keyboard_runtime_source_contract
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod07_keyboard_source.py)

# MOD-08 Consumer Media / Volume / Brightness and Report-ID ABI.
add_executable(blu2usb_test_mod08_consumer test_mod08_consumer.c)
target_link_libraries(blu2usb_test_mod08_consumer PRIVATE
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator blu2usb_usb_hid)
target_compile_options(blu2usb_test_mod08_consumer PRIVATE -UNDEBUG)
add_test(NAME mod08_consumer_8_usages_ack COMMAND blu2usb_test_mod08_consumer)
add_test(NAME mod08_combined_descriptor_boot_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod08_descriptor_source.py)

# MOD-08 corrective candidate: operator-reported KDE, Android & Obsidian cases.
add_executable(blu2usb_test_mod08_reported_fixes test_mod08_reported_fixes.c)
target_link_libraries(blu2usb_test_mod08_reported_fixes PRIVATE
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator blu2usb_movement)
target_compile_options(blu2usb_test_mod08_reported_fixes PRIVATE -UNDEBUG)
add_test(NAME mod08_operator_kde_android_zooms_and_media_dwell
    COMMAND blu2usb_test_mod08_reported_fixes)

# MOD-09: recovery, busy USB, BLE cancellation and held source ownership.
add_executable(blu2usb_test_mod09_recovery test_mod09_recovery.c)
target_link_libraries(blu2usb_test_mod09_recovery PRIVATE
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator blu2usb_movement)
target_compile_options(blu2usb_test_mod09_recovery PRIVATE -UNDEBUG)
add_test(NAME mod09_recovery_cancel_overflow_and_ownership
    COMMAND blu2usb_test_mod09_recovery)
add_test(NAME mod09_app_saved_devices_safety_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod09_source.py)

# MOD-10 release qualification: integrated HOME->picker->v3 persistence,
# all 30 typed HID actions and production safety/ABI invariants.
add_executable(blu2usb_test_mod10_full_flow test_mod10_full_flow.c)
target_link_libraries(blu2usb_test_mod10_full_flow PRIVATE
    blu2usb_renderer blu2usb_ux_model blu2usb_profiles
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator)
target_compile_options(blu2usb_test_mod10_full_flow PRIVATE -UNDEBUG)
add_test(NAME mod10_home_editor_persistence_mask_and_reboot
    COMMAND blu2usb_test_mod10_full_flow)

add_executable(blu2usb_test_mod10_all_actions test_mod10_all_actions.c)
target_link_libraries(blu2usb_test_mod10_all_actions PRIVATE
    blu2usb_modifier_shortcuts blu2usb_hid_aggregator blu2usb_usb_hid)
target_compile_options(blu2usb_test_mod10_all_actions PRIVATE -UNDEBUG)
add_test(NAME mod10_full_30_action_keyboard_consumer_matrix
    COMMAND blu2usb_test_mod10_all_actions)

add_test(NAME mod10_production_security_abi_source
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mod10_source.py)

# MNT-01 / HF-01 (v3.x): Saved Devices implicit single selected action
# and system gray hint colors, without changing bond navigation.
add_executable(blu2usb_test_mnt01_saved_devices_colors
    test_mnt01_saved_devices_colors.c)
target_link_libraries(blu2usb_test_mnt01_saved_devices_colors PRIVATE
    blu2usb_renderer blu2usb_ux_model)
target_compile_options(blu2usb_test_mnt01_saved_devices_colors PRIVATE -UNDEBUG)
add_test(NAME mnt01_saved_devices_implicit_selection_gray_hints
    COMMAND blu2usb_test_mnt01_saved_devices_colors)

# MNT-02 / HF-02 + HF-03: Remove This KEY X Help and KEY Y Lock
# Atomic UX copy + runtime, with pending BLE deletion and Help regressions.
add_executable(blu2usb_test_mnt02_remove_this_key_x_help
    test_mnt02_remove_this_key_x_help.c)
target_link_libraries(blu2usb_test_mnt02_remove_this_key_x_help PRIVATE
    blu2usb_renderer blu2usb_ux_model)
target_compile_options(blu2usb_test_mnt02_remove_this_key_x_help PRIVATE -UNDEBUG)
add_test(NAME mnt02_remove_this_key_x_help_and_y_lock
    COMMAND blu2usb_test_mnt02_remove_this_key_x_help)
