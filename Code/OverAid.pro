QT       += core gui sql printsupport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 sdk_no_version_check

SOURCES += \
    Classes/RangeSlider.cpp \
    Classes/custommenu.cpp \
    lineform.cpp \
    main.cpp \
    manageaccount.cpp \
    managecategories.cpp \
    managesubscriptions.cpp \
    overaid.cpp \
    overaid_action.cpp \
    overaid_stats.cpp \
    overaid_update.cpp \
    transform.cpp

HEADERS += \
    Classes/RangeSlider.h \
    Classes/custommenu.h \
    lineform.h \
    manageaccount.h \
    managecategories.h \
    managesubscriptions.h \
    overaid.h \
    transform.h

FORMS += \
    lineform.ui \
    manageaccount.ui \
    managecategories.ui \
    managesubscriptions.ui \
    overaid.ui \
    transform.ui

ICON = logo.icns         #https://icon-icons.com/fr/pack/Currency-vol-1-Icons/1172
RC_ICONS = logo.ico

RESOURCES += \
    ressource.qrc

DISTFILES += \
    version.txt

TRANSLATIONS += \
    ressources/translations/qtbase_fr.ts \
    ressources/translations/english.ts \
    ressources/translations/qtbase_es.ts \
    ressources/translations/spanish.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
