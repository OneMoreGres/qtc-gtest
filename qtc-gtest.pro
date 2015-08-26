DEFINES += QTCGTEST_LIBRARY

VERSION_SUFFIX = "_1"

include(paths.pri)

# QtcGtest files

SOURCES += \
    src/QtcGtestPlugin.cpp \
    src/PaneWidget.cpp \
    src/OutputParser.cpp \
    src/TestModel.cpp \
    src/OutputPane.cpp \
    src/ParseState.cpp \
    src/TestProject.cpp \
    src/CustomRunConfiguration.cpp \
    src/AutoToolTipDelegate.cpp

HEADERS += \
    src/Constants.h \
    src/PluginGlobal.h \
    src/QtcGtestPlugin.h \
    src/PaneWidget.h \
    src/OutputParser.h \
    src/TestModel.h \
    src/OutputPane.h \
    src/ParseState.h \
    src/TestProject.h \
    src/CustomRunConfiguration.h \
    src/AutoToolTipDelegate.h

FORMS += \
    src/PaneWidget.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    translation/QtcGtest_ru.ts

OTHER_FILES += \
    LICENSE.md \
    README.md \
    images/README.md \
    util/README.md

PROVIDER = Gres

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = QtcGtest
QTC_LIB_DEPENDS += \
    cplusplus

QTC_PLUGIN_DEPENDS += \
    coreplugin\
    projectexplorer\
    cpptools

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
