#-------------------------------------------------
#
# Project created by QtCreator 2017-08-04T18:33:14
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TFZip2-2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += \
    QT_DEPRECATED_WARNINGS \
    UNICODE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        tfzip2.cpp \
    BZIP2Compress.cpp \
    BZIP2Decompress.cpp \
    BZProgressBar.cpp \
    IOUtils.cpp \
    ProgressBarLayout.cpp \
    tfbzlib.cpp \
    bzip2-1.0.6/blocksort.c \
    bzip2-1.0.6/bzlib.c \
    bzip2-1.0.6/compress.c \
    bzip2-1.0.6/crctable.c \
    bzip2-1.0.6/decompress.c \
    bzip2-1.0.6/huffman.c \
    bzip2-1.0.6/blocksort.c \
    bzip2-1.0.6/bzlib.c \
    bzip2-1.0.6/compress.c \
    bzip2-1.0.6/crctable.c \
    bzip2-1.0.6/decompress.c \
    bzip2-1.0.6/huffman.c \
    hash-library/sha1.cpp

HEADERS += \
    tfzip2.h \
    BZIP2Compress.h \
    BZIP2Decompress.h \
    BZProgressBar.h \
    IOUtils.h \
    ProgressBarLayout.h \
    tfbzlib.h \
    WorkerException.h \
    ParallelRunnable.hpp \
    bzip2-1.0.6/bzlib.h \
    bzip2-1.0.6/bzlib_private.h \
    bzip2-1.0.6/bzlib.h \
    bzip2-1.0.6/bzlib_private.h \
    hash-library/sha1.h

FORMS += tfzip2.ui

RESOURCES += \
    tfzip2.qrc

DISTFILES += \
    bzip2-1.0.6/manual.pdf \
    bzip2-1.0.6/manual.ps \
    bzip2-1.0.6/sample1.bz2 \
    bzip2-1.0.6/sample2.bz2 \
    bzip2-1.0.6/sample3.bz2 \
    bzip2-1.0.6/sample1.ref \
    bzip2-1.0.6/sample2.ref \
    bzip2-1.0.6/format.pl \
    bzip2-1.0.6/bzdiff \
    bzip2-1.0.6/bzgrep \
    bzip2-1.0.6/bzmore \
    bzip2-1.0.6/xmlproc.sh \
    bzip2-1.0.6/entities.xml \
    bzip2-1.0.6/manual.xml \
    bzip2-1.0.6/bzip.css \
    bzip2-1.0.6/manual.html \
    bzip2-1.0.6/bzip2.1 \
    bzip2-1.0.6/bzip2.1.preformatted \
    bzip2-1.0.6/bzip2.txt \
    bzip2-1.0.6/CHANGES \
    bzip2-1.0.6/dlltest.dsp \
    bzip2-1.0.6/libbz2.def \
    bzip2-1.0.6/libbz2.dsp \
    bzip2-1.0.6/LICENSE \
    bzip2-1.0.6/sample3.ref \
    bzip2-1.0.6/words0 \
    bzip2-1.0.6/words1 \
    bzip2-1.0.6/words2 \
    bzip2-1.0.6/words3 \
    bzip2-1.0.6/bzdiff.1 \
    bzip2-1.0.6/bzgrep.1 \
    bzip2-1.0.6/bzmore.1 \
    bzip2-1.0.6/README \
    bzip2-1.0.6/README.COMPILATION.PROBLEMS \
    bzip2-1.0.6/README.XML.STUFF
