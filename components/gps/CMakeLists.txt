idf_component_register(SRCS "ww_gps.cpp"
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES "driver" "udpClient")
                       