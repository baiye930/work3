#include "mastrview.h"
#include "ui_mastrview.h"
#include <QDebug>
#include "idatabase.h"

MastrView::MastrView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MastrView)
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
    qDebug()<<"goLoginView";
    loginView=new LoginView(this);
    pushWidgetToStackView(loginView);

    connect(loginView,SIGNAL(loginSuccess()),this,SLOT(goWelcomView()));

}

void MastrView::goWelcomView()
{
    qDebug()<<"goWelcomView";
    welcomeView=new WelcomView(this);
    pushWidgetToStackView(welcomeView);

    connect(welcomeView,SIGNAL(goDoctorView()),this,SLOT(goDoctorView()));
    connect(welcomeView,SIGNAL(goPatientView()),this,SLOT(goPatientView()));
    connect(welcomeView,SIGNAL(goDepartmentView()),this,SLOT(goDepartmentView()));
    connect(welcomeView,SIGNAL(goPrescriptionview()),this,SLOT(goPrescriptionview()));
    connect(welcomeView,SIGNAL(gomedicineview()),this,SLOT(gomedicineview()));
    connect(welcomeView,SIGNAL(goconsult_recordview()),this,SLOT(goconsult_recordview()));
    connect(welcomeView,SIGNAL(goappointmentview()),this,SLOT(goappointmentview()));

}

void MastrView::goDoctorView()
{
    qDebug()<<"goDoctorView";
    doctorView=new DoctorView(this);
    pushWidgetToStackView(doctorView);

}

void MastrView::goDepartmentView()
{
    qDebug()<<"goDepartmentView";
    departmentView=new DepartmentView();
    pushWidgetToStackView(departmentView);

}

void MastrView::goPatientEditView(int rowNo)
{
    qDebug()<<"goPatientEditView";
    patientEditView=new PatientEditView(this,rowNo);
    pushWidgetToStackView(patientEditView);

    connect(patientEditView,SIGNAL(goPreviousView()),this,SLOT(goPreviousView()));

}

void MastrView::goPatientView()
{
    qDebug()<<"goPaitentView";
    patientView=new PatientView(this);
    pushWidgetToStackView(patientView);

    connect(patientView,SIGNAL(goPatientEditView(int)),this,SLOT(goPatientEditView(int)));


}

void MastrView::goPreviousView()
{
    int count=ui->stackedWidget->count();

    if(count>1){
        ui->stackedWidget->setCurrentIndex(count-2);
        ui->labelTitle->setText(ui->stackedWidget->currentWidget()->windowTitle());

        QWidget *widget=ui->stackedWidget->widget(count-1);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

}

void MastrView::goPrescriptionview()
{

    qDebug() << "goPrescriptionview";
    prescriptionView = new Prescriptionview(this);
    pushWidgetToStackView(prescriptionView);
}

void MastrView::gomedicineview()
{
    qDebug() << "gomedicineview";
    medicineView = new medicineview(this);
    pushWidgetToStackView(medicineView);
}

void MastrView::goconsult_recordview()
{
    qDebug() << "goconsult_recordview";
    consult_recordView = new consult_recordview(this);
    pushWidgetToStackView(consult_recordView);
}

void MastrView::goappointmentview()
{
    qDebug() << "goappointmentview";
    appointmentView = new appointmentview(this);
    pushWidgetToStackView(appointmentView);
}



void MastrView::pushWidgetToStackView(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    int count=ui->stackedWidget->count();
    ui->stackedWidget->setCurrentIndex(count-1);
    ui->labelTitle->setText(widget->windowTitle());
}


void MastrView::on_btBack_clicked()
{
    goPreviousView();
}


void MastrView::on_stackedWidget_currentChanged(int arg1)
{
    int count=ui->stackedWidget->count();
    if(count>1)
        ui->btBack->setEnabled(true);
    else
        ui->btBack->setEnabled(false);

    QString title =ui->stackedWidget->currentWidget()->windowTitle();

    if(title=="欢迎"){
        ui->btLogout->setEnabled(true);
        ui->btBack->setEnabled(false);
    }else {
        ui->btLogout->setEnabled(false);

    }

}


void MastrView::on_btLogout_clicked()
{
    goPreviousView();
}

