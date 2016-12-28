QT += core
QT += gui

CONFIG += c++11

TARGET = pgn2dcg
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    chess/board.cpp \
    chess/ecocode.cpp \
    chess/game.cpp \
    chess/game_node.cpp \
    chess/gui_printer.cpp \
    chess/move.cpp \
    chess/pgn_printer.cpp \
    chess/pgn_reader.cpp \
    chess/polyglot.cpp \
    chess/namebase.cpp \
    chess/sitebase.cpp \
    chess/database.cpp \
    chess/dcgencoder.cpp

HEADERS += \
    chess/board.h \
    chess/ecocode.h \
    chess/game.h \
    chess/game_node.h \
    chess/gui_printer.h \
    chess/move.h \
    chess/pgn_printer.h \
    chess/pgn_reader.h \
    chess/polyglot.h \
    chess/namebase.h \
    chess/sitebase.h \
    chess/database.h \
    chess/dcgencoder.h
