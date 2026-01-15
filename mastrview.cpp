// mastrview.cpp - 添加科室编辑视图实现
#include "mastrview.h"
#include "ui_mastrview.h"
#include <QDebug>
#include "idatabase.h"
#include "medicineeditview.h"
// 需要包含科室编辑视图头文件
#include "departmenteditview.h"
#include "prescriptioneditview.h"

#include "appointmenteditview.h"
MastrView::MastrView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MastrView)
    , welcomeView(nullptr)
    , doctorView(nullptr)
    , patientView(nullptr)
    , departmentView(nullptr)
    , loginView(nullptr)
    , patientEditView(nullptr)
    , appointmentView(nullptr)
    , consult_recordView(nullptr)
    , medicineView(nullptr)
    , prescriptionView(nullptr)
    , doctorEditView(nullptr)
    , departmentEditView(nullptr)
    , medicineEditView(nullptr)
    , prescriptionEditView(nullptr)  // 初始化处方编辑视图指针
    ,appointmentEditView(nullptr)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::FramelessWindowHint);

    goLoginView();

    IDatabase::getInstance();
}

MastrView::~MastrView()
{
    delete ui;
}

void MastrView::goLoginView()
{
    qDebug() << "goLoginView";
    loginView = new LoginView(this);
    pushWidgetToStackView(loginView);

    connect(loginView, SIGNAL(loginSuccess()), this, SLOT(goWelcomView()));
}

void MastrView::goWelcomView()
{
    qDebug() << "goWelcomView";
    welcomeView = new WelcomView(this);
    pushWidgetToStackView(welcomeView);

    connect(welcomeView, SIGNAL(goDoctorView()), this, SLOT(goDoctorView()));
    connect(welcomeView, SIGNAL(goPatientView()), this, SLOT(goPatientView()));
    connect(welcomeView, SIGNAL(goDepartmentView()), this, SLOT(goDepartmentView()));
    connect(welcomeView, SIGNAL(goPrescriptionview()), this, SLOT(goPrescriptionview()));
    connect(welcomeView, SIGNAL(gomedicineview()), this, SLOT(gomedicineview()));
    connect(welcomeView, SIGNAL(goconsult_recordview()), this, SLOT(goconsult_recordview()));
    connect(welcomeView, SIGNAL(goappointmentview()), this, SLOT(goappointmentview()));
    connect(appointmentView, SIGNAL(goAppointmentEditView(int)),
            this, SLOT(goAppointmentEditView(int)));
}

void MastrView::goDoctorView()
{
    qDebug() << "goDoctorView";
    doctorView = new DoctorView(this);
    pushWidgetToStackView(doctorView);

    // 连接信号，跳转到医生编辑视图
    connect(doctorView, SIGNAL(goDoctorEditView(int)), this, SLOT(goDoctorEditView(int)));
}

void MastrView::goDepartmentView()
{
    qDebug() << "goDepartmentView";
    departmentView = new DepartmentView();
    pushWidgetToStackView(departmentView);

    // 连接信号，跳转到科室编辑视图
    connect(departmentView, SIGNAL(goDepartmentEditView(int)), this, SLOT(goDepartmentEditView(int)));
}

void MastrView::goPatientEditView(int rowNo)
{
    qDebug() << "goPatientEditView";
    patientEditView = new PatientEditView(this, rowNo);
    pushWidgetToStackView(patientEditView);

    connect(patientEditView, SIGNAL(goPreviousView()), this, SLOT(goPreviousView()));
}

void MastrView::goPatientView()
{
    qDebug() << "goPaitentView";
    patientView = new PatientView(this);
    pushWidgetToStackView(patientView);

    connect(patientView, SIGNAL(goPatientEditView(int)), this, SLOT(goPatientEditView(int)));
}

void MastrView::goPreviousView()
{
    int count = ui->stackedWidget->count();

    if (count > 1) {
        ui->stackedWidget->setCurrentIndex(count - 2);
        ui->labelTitle->setText(ui->stackedWidget->currentWidget()->windowTitle());

        QWidget *widget = ui->stackedWidget->widget(count - 1);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }
}





void MastrView::goconsult_recordview()
{
    qDebug() << "goconsult_recordview";
    consult_recordView = new consult_recordview(this);
    pushWidgetToStackView(consult_recordView);
}



void MastrView::goDoctorEditView(int rowNo)
{
    qDebug() << "goDoctorEditView, row:" << rowNo;
    doctorEditView = new DoctoreditView(this, rowNo);
    pushWidgetToStackView(doctorEditView);

    connect(doctorEditView, SIGNAL(goPreviousView()), this, SLOT(goPreviousView()));
}

// 新增：跳转到科室编辑视图
void MastrView::goDepartmentEditView(int rowNo)
{
    qDebug() << "goDepartmentEditView, row:" << rowNo;
    departmentEditView = new DepartmentEditView(this, rowNo);
    pushWidgetToStackView(departmentEditView);

    connect(departmentEditView, SIGNAL(goPreviousView()), this, SLOT(goPreviousView()));
}



void MastrView::on_btBack_clicked()
{
    goPreviousView();
}

void MastrView::on_stackedWidget_currentChanged(int arg1)
{
    Q_UNUSED(arg1);

    int count = ui->stackedWidget->count();
    if (count > 1)
        ui->btBack->setEnabled(true);
    else
        ui->btBack->setEnabled(false);

    QString title = ui->stackedWidget->currentWidget()->windowTitle();

    if (title == "欢迎") {
        ui->btLogout->setEnabled(true);
        ui->btBack->setEnabled(false);
    } else {
        ui->btLogout->setEnabled(false);
    }
}

void MastrView::on_btLogout_clicked()
{
    goPreviousView();
}
void MastrView::gomedicineview()
{
    qDebug() << "gomedicineview";
    medicineView = new medicineview(this);
    pushWidgetToStackView(medicineView);

    // 连接信号，跳转到药品编辑视图
    connect(medicineView, SIGNAL(goMedicineEditView(int)), this, SLOT(goMedicineEditView(int)));
}
// 新增：跳转到药品编辑视图
void MastrView::goMedicineEditView(int rowNo)
{
    qDebug() << "goMedicineEditView, row:" << rowNo;
    medicineEditView = new MedicineEditView(this, rowNo);
    pushWidgetToStackView(medicineEditView);

    connect(medicineEditView, SIGNAL(goPreviousView()), this, SLOT(goPreviousView()));
}

void MastrView::pushWidgetToStackView(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    int count = ui->stackedWidget->count();
    ui->stackedWidget->setCurrentIndex(count - 1);
    ui->labelTitle->setText(widget->windowTitle());
}
void MastrView::goPrescriptionview()
{
    qDebug() << "goPrescriptionview";
    prescriptionView = new Prescriptionview(this);
    pushWidgetToStackView(prescriptionView);

    // 连接信号，跳到处方编辑视图
    connect(prescriptionView, SIGNAL(goPrescriptionEditView(int)), this, SLOT(goPrescriptionEditView(int)));
}
void MastrView::goPrescriptionEditView(int rowNo)
{
    qDebug() << "goPrescriptionEditView, row:" << rowNo;
    prescriptionEditView = new PrescriptionEditView(this, rowNo);
    pushWidgetToStackView(prescriptionEditView);

    connect(prescriptionEditView, SIGNAL(goPreviousView()), this, SLOT(goPreviousView()));
}


void MastrView::goappointmentview()
{
    qDebug() << "goappointmentview";
    appointmentView = new appointmentview(this);
    pushWidgetToStackView(appointmentView);

    // 连接信号，跳转到预约编辑界面
    connect(appointmentView, SIGNAL(goAppointmentEditView(int)),
            this, SLOT(goAppointmentEditView(int)));
}
// 实现跳转函数
void MastrView::goAppointmentEditView(int rowNo)
{
    qDebug() << "goAppointmentEditView, row:" << rowNo;
    appointmentEditView = new AppointmentEditView(this, rowNo);
    pushWidgetToStackView(appointmentEditView);

    connect(appointmentEditView, SIGNAL(goPreviousView()),
            this, SLOT(goPreviousView()));
}
