include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KLsofWidget
    INCLUDE_FILE "lsof.h"
    GROUP "KSysGuard"
)

ecm_add_qtdesignerplugin(ksysguardlsofwidgets
    NAME KSysGuardLsofWidgets
    OUTPUT_NAME ksysguardlsof5widgets
    WIDGETS
        KLsofWidget
    LINK_LIBRARIES
        lsofui
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
