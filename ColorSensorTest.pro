TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
    sensor.c

HEADERS += \
    sensor.h

# Default rules for target deployment.
target.path = /home/student/qt5
INSTALLS += target
