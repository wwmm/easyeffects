install(FILES 
    icons/com.github.wwmm.easyeffects.svg 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES 
    icons/com.github.wwmm.easyeffects-symbolic.svg 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps
)

install(PROGRAMS 
    com.github.wwmm.easyeffects.desktop.in
    DESTINATION ${KDE_INSTALL_APPDIR}
    RENAME com.github.wwmm.easyeffects.desktop
)