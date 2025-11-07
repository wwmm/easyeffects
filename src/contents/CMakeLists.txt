install(FILES 
    icons/${APPLICATION_ID}.svg 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES 
    icons/com.github.wwmm.easyeffects-symbolic.svg 
    DESTINATION ${KDE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps
)

configure_file(com.github.wwmm.easyeffects.metainfo.xml.in
    com.github.wwmm.easyeffects.metainfo.xml.in
)

install(FILES 
    com.github.wwmm.easyeffects.metainfo.xml.in
    DESTINATION ${KDE_INSTALL_METAINFODIR}
    RENAME ${APPLICATION_ID}.metainfo.xml
)

configure_file(com.github.wwmm.easyeffects.desktop.in 
    com.github.wwmm.easyeffects.desktop.in
)

install(FILES 
    com.github.wwmm.easyeffects.desktop.in
    DESTINATION ${KDE_INSTALL_APPDIR}
    RENAME ${APPLICATION_ID}.desktop
)