DEFINES += QTCGTEST_LIBRARY

include(paths.pri)

# QtcGtest files

SOURCES += \
    src/QtcGtestPlugin.cpp \
    src/PaneWidget.cpp \
    src/OutputParser.cpp \
    src/TestModel.cpp \
    src/OutputPane.cpp \
    src/ParseState.cpp

HEADERS += \
    src/Constants.h \
    src/PluginGlobal.h \
    src/QtcGtestPlugin.h \
    src/PaneWidget.h \
    src/OutputParser.h \
    src/TestModel.h \
    src/OutputPane.h \
    src/ParseState.h

TRANSLATIONS += \
    translation/QtcGtest_ru.ts

OTHER_FILES += \
    LICENSE.md \
    README.md

PROVIDER = Gres

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = QtcGtest
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin\
    projectexplorer

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

FORMS += \
    src/PaneWidget.ui

