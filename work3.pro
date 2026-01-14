QT       += core gui sql


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    appointmentview.cpp \
    consult_recordview.cpp \
    departmenteditview.cpp \
    departmentview.cpp \
    doctoreditview.cpp \
    doctorview.cpp \
    idatabase.cpp \
    loginview.cpp \
    main.cpp \
    mastrview.cpp \
    medicineview.cpp \
    patienteditview.cpp \
    patientview.cpp \
    prescriptionview.cpp \
    welcomview.cpp

HEADERS += \
    appointmentview.h \
    consult_recordview.h \
    departmenteditview.h \
    departmentview.h \
    doctoreditview.h \
    doctorview.h \
    idatabase.h \
    loginview.h \
    mastrview.h \
    medicineview.h \
    patienteditview.h \
    patientview.h \
    prescriptionview.h \
    welcomview.h

FORMS += \
    appointmentview.ui \
    consult_recordview.ui \
    departmenteditview.ui \
    departmentview.ui \
    doctoreditview.ui \
    doctorview.ui \
    loginview.ui \
    mastrview.ui \
    medicineview.ui \
    patienteditview.ui \
    patientview.ui \
    prescriptionview.ui \
    welcomview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
