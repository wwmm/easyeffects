configure_file(com.github.wwmm.easyeffects.service.in com.github.wwmm.easyeffects.service)

install(FILES 
    icons/com.github.wwmm.easyeffects.svg 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/com.github.wwmm.easyeffects.service 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/dbus-1/services
)

install(PROGRAMS 
    com.github.wwmm.easyeffects.desktop 
    DESTINATION ${KDE_INSTALL_APPDIR}
)