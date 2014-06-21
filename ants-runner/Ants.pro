TEMPLATE = app
CONFIG += c++11
DEPENDPATH = . gui logic manager
QT += widgets
INCLUDEPATH += . \
               ./gui \
               ./logic \
               ./manager \
				/usr/local/Cellar/boost/1.53.0/include/
QMAKE_CXXFLAGS += -std=c++11 -g
LIBS += -ldl -lrt
TARGET   = Ants
CONFIG  += console debug
CONFIG  -= app_bundle

HEADERS += main_window.hpp \
           gui/IAntGUI.hpp \
           gui/IAntGuiImpl.hpp \
           logic/IAntLogic.hpp \
           logic/IAntLogicImpl.hpp \
           manager/AntManager.hpp #\
           #manager/Ant.hpp

SOURCES += main.cpp \
           main_window.cpp \
           gui/IAntGUI.cpp \
           gui/IAntGuiImpl.cpp \
           logic/IAntLogic.cpp \
           logic/IAntLogicImpl.cpp \
           manager/AntManager.cpp #\
           #manager/Ant.cpp

FORMS   +=
