#include "prescriptionview.h"
#include "ui_prescriptionview.h"

Prescriptionview::Prescriptionview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Prescriptionview)
{
    ui->setupUi(this);
}

Prescriptionview::~Prescriptionview()
{
    delete ui;
}
