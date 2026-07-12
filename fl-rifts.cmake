if(MOD_ALE_FOUND)
  if(MSVC)
    set(FL_RIFTS_CONFIGURATION_NAME $(ConfigurationName)/)
  endif()

  set(FL_RIFTS_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
  set(FL_RIFTS_LUA_OUTPUT
    "${FL_RIFTS_BIN_DIR}/${FL_RIFTS_CONFIGURATION_NAME}lua_scripts/FLRifts")

  add_custom_command(TARGET modules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "${FL_RIFTS_LUA_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/modules/fl-rifts/lua"
      "${FL_RIFTS_LUA_OUTPUT}")

  if(WIN32)
    set(FL_RIFTS_LUA_INSTALL
      "${CMAKE_INSTALL_PREFIX}/lua_scripts/FLRifts")
  else()
    set(FL_RIFTS_LUA_INSTALL
      "${CMAKE_INSTALL_PREFIX}/bin/lua_scripts/FLRifts")
  endif()

  install(DIRECTORY "${CMAKE_SOURCE_DIR}/modules/fl-rifts/lua/"
    DESTINATION "${FL_RIFTS_LUA_INSTALL}")
endif()
