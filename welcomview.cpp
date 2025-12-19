#include "welcomview.h"
#include "ui_welcomview.h"
#include <QDebug>

WelcomView::WelcomView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomView)
{
    qDebug()<<"create WelcomeView";
    ui->setupUi(this);
}

WelcomView::~WelcomView()
{
     qDebug()<<"destroy WelcomeView";
    delete ui;
}

void WelcomView::on_btDepartment_clicked()
{
    emit goDepartmentView();
}


void WelcomView::on_btDoctor_clicked()
{
    emit goDoctorView();
}


void WelcomView::on_btPatient_clicked()
{
    emit goPatientView();
}

