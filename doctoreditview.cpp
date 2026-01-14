#include "doctoreditview.h"
#include "ui_doctoreditview.h"

DoctoreditView::DoctoreditView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctoreditView)
{
    ui->setupUi(this);
}

DoctoreditView::~DoctoreditView()
{
    delete ui;
}
