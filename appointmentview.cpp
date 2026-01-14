#include "appointmentview.h"
#include "ui_appointmentview.h"

appointmentview::appointmentview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::appointmentview)
{
    ui->setupUi(this);
}

appointmentview::~appointmentview()
{
    delete ui;
}
