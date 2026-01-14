#include "consult_recordview.h"
#include "ui_consult_recordview.h"

consult_recordview::consult_recordview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::consult_recordview)
{
    ui->setupUi(this);
}

consult_recordview::~consult_recordview()
{
    delete ui;
}
