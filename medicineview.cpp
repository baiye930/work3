#include "medicineview.h"
#include "ui_medicineview.h"

medicineview::medicineview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::medicineview)
{
    ui->setupUi(this);
}

medicineview::~medicineview()
{
    delete ui;
}
