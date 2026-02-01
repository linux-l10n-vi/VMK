install(FILES fcitx5-vmk-server@.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system)

install(FILES 99-vmk.rules
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d)

install(FILES module-fcitx5-vmk.conf
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/modules-load.d)

install(FILES user-vmk.conf
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/sysusers.d)
