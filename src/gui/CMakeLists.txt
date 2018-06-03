set(CMAKE_AUTORCC True)
set(CMAKE_AUTOUIC True)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(lineedit)
add_subdirectory(properties)
add_subdirectory(powermanagement)
add_subdirectory(rss)
add_subdirectory(search)

add_library(qbt_gui STATIC
# headers
about_imp.h
addnewtorrentdialog.h
advancedsettings.h
autoexpandabledialog.h
banlistoptions.h
categoryfiltermodel.h
categoryfilterproxymodel.h
categoryfilterwidget.h
cookiesdialog.h
cookiesmodel.h
deletionconfirmationdlg.h
downloadfromurldialog.h
executionlog.h
fspathedit.h
fspathedit_p.h
guiiconprovider.h
hidabletabwidget.h
ipsubnetwhitelistoptionsdialog.h
loglistwidget.h
mainwindow.h
messageboxraised.h
optionsdlg.h
previewlistdelegate.h
previewselectdialog.h
scanfoldersdelegate.h
shutdownconfirmdlg.h
speedlimitdlg.h
statsdialog.h
statusbar.h
tagfiltermodel.h
tagfilterproxymodel.h
tagfilterwidget.h
torrentcategorydialog.h
torrentcontentfiltermodel.h
torrentcontentmodel.h
torrentcontentmodelfile.h
torrentcontentmodelfolder.h
torrentcontentmodelitem.h
torrentcontenttreeview.h
torrentcreatordlg.h
torrentmodel.h
trackerlogin.h
transferlistdelegate.h
transferlistfilterswidget.h
transferlistsortmodel.h
transferlistwidget.h
updownratiodlg.h
utils.h
theme/colorprovider_p.h
theme/colorproviders.h
theme/colortheme.h
theme/colortheme_impl.h
theme/fontprovider_p.h
theme/fontproviders.h
theme/fonttheme.h
theme/fonttheme_impl.h
theme/provider_p.h
theme/serializabletheme.h
theme/themecommon.h
theme/themeexceptions.h
theme/themeinfo.h
theme/themeprovider.h
# sources
addnewtorrentdialog.cpp
advancedsettings.cpp
autoexpandabledialog.cpp
banlistoptions.cpp
categoryfiltermodel.cpp
categoryfilterproxymodel.cpp
categoryfilterwidget.cpp
cookiesdialog.cpp
cookiesmodel.cpp
downloadfromurldialog.cpp
executionlog.cpp
fspathedit.cpp
fspathedit_p.cpp
guiiconprovider.cpp
ipsubnetwhitelistoptionsdialog.cpp
loglistwidget.cpp
mainwindow.cpp
messageboxraised.cpp
optionsdlg.cpp
previewselectdialog.cpp
scanfoldersdelegate.cpp
shutdownconfirmdlg.cpp
speedlimitdlg.cpp
statsdialog.cpp
statusbar.cpp
tagfiltermodel.cpp
tagfilterproxymodel.cpp
tagfilterwidget.cpp
torrentcategorydialog.cpp
torrentcontentfiltermodel.cpp
torrentcontentmodel.cpp
torrentcontentmodelfile.cpp
torrentcontentmodelfolder.cpp
torrentcontentmodelitem.cpp
torrentcontenttreeview.cpp
torrentcreatordlg.cpp
torrentmodel.cpp
trackerlogin.cpp
transferlistdelegate.cpp
transferlistfilterswidget.cpp
transferlistsortmodel.cpp
transferlistwidget.cpp
updownratiodlg.cpp
utils.cpp
theme/colorprovider_p.cpp
theme/colorproviders.cpp
theme/colortheme.cpp
theme/fonttheme.cpp
theme/fontprovider_p.cpp
theme/fontproviders.cpp
theme/provider_p.cpp
theme/serializabletheme.cpp
theme/themecommon.cpp
theme/themeexceptions.cpp
theme/themeinfo.cpp
theme/themeprovider.cpp
# forms
mainwindow.ui
about.ui
banlistoptions.ui
cookiesdialog.ui
ipsubnetwhitelistoptionsdialog.ui
previewselectdialog.ui
login.ui
downloadfromurldialog.ui
bandwidth_limit.ui
updownratiodlg.ui
confirmdeletiondlg.ui
executionlog.ui
addnewtorrentdialog.ui
autoexpandabledialog.ui
statsdialog.ui
optionsdlg.ui
torrentcategorydialog.ui
torrentcreatordlg.ui
shutdownconfirmdlg.ui
)

target_link_libraries(qbt_gui
    PRIVATE
        qbt_lineedit qbt_powermanagement qbt_rss qbt_properties qbt_searchengine
        qbt_base
        QtSingleApplication::QtSingleApplication
)

target_include_directories(qbt_gui PRIVATE ../app)

if (UNIX AND Qt5DBus_FOUND)
   add_subdirectory(qtnotify)
   target_link_libraries(qbt_gui PRIVATE qbt_qtnotify)
endif (UNIX AND Qt5DBus_FOUND)

if (APPLE)
    target_sources(qbt_gui PRIVATE macutilities.h macutilities.mm)
    find_package(Qt5 ${requiredQtVersion} REQUIRED COMPONENTS MacExtras)
    target_link_libraries(qbt_gui PRIVATE Qt5::MacExtras objc)
endif (APPLE)

if (WIN32 OR APPLE)
    target_sources(qbt_gui PRIVATE programupdater.h programupdater.cpp)
endif (WIN32 OR APPLE)

if (PLASMA_INTEGRATION)
    target_sources(qbt_gui PRIVATE
        theme/plasmacolorprovider.h
        utils/colorutils.h
        utils/plasmacolorscheme.h
        theme/plasmacolorprovider.cpp
        utils/colorutils.cpp
        utils/plasmacolorscheme.cpp
        )
endif (PLASMA_INTEGRATION)

qbt_target_sources(qBittorrent PRIVATE about.qrc theme/builtinthemes.qrc)

if(WIN32)
    find_package(Qt5 ${requiredQtVersion} REQUIRED COMPONENTS WinExtras)
    target_link_libraries(qbt_gui PRIVATE Qt5::WinExtras)
endif(WIN32)
