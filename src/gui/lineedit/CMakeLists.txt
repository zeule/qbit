add_library(qbt_lineedit STATIC
src/lineedit.cpp
src/lineedit.h
)

target_link_libraries(qbt_lineedit PUBLIC Qt5::Widgets)
