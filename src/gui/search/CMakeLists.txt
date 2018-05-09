add_library(qbt_searchengine STATIC
pluginselectdlg.ui
pluginsourcedlg.ui
searchwidget.ui
pluginselectdlg.h
pluginsourcedlg.h
searchlistdelegate.h
searchsortmodel.h
searchtab.h
searchwidget.h
pluginselectdlg.cpp
pluginsourcedlg.cpp
searchlistdelegate.cpp
searchsortmodel.cpp
searchtab.cpp
searchwidget.cpp
)

set(QBT_SEARCHENGINE_RESOURCES
# search.qrc
)

target_link_libraries(qbt_searchengine PUBLIC qbt_base)
target_include_directories(qbt_searchengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lineedit/src)
