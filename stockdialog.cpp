#include "stockdialog.h"
#include "ui_stockdialog.h"
#include <QMessageBox>
#include <QDebug>

StockDialog::StockDialog(QWidget *parent, OperationType type,
                         const QString &medicineName, const QString &medicineId)
    : QDialog(parent)
    , ui(new Ui::StockDialog)
    , operationType(type)
    , medicineId(medicineId)
{
    ui->setupUi(this);
    initUI();

    // 设置药品名称
    if (!medicineName.isEmpty()) {
        ui->lblMedicineName->setText(medicineName);
    }

    // 根据操作类型设置窗口标题和标签
    switch (type) {
    case StockIn:
        this->setWindowTitle("药品入库");
        ui->lblQuantity->setText("入库数量：");
        break;
    case StockOut:
        this->setWindowTitle("药品出库");
        ui->lblQuantity->setText("出库数量：");
        ui->gbBatchInfo->setVisible(false);
        break;
    case StockAdjust:
        this->setWindowTitle("库存调整");
        ui->lblQuantity->setText("调整后库存：");
        ui->gbBatchInfo->setVisible(false);
        break;
    }

    // 连接信号槽
    connect(ui->btnConfirm, &QPushButton::clicked, this, &StockDialog::on_btnConfirm_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &StockDialog::on_btnCancel_clicked);
}

StockDialog::~StockDialog()
{
    delete ui;
}

void StockDialog::initUI()
{
    // 设置日期范围
    ui->dateExpiry->setDate(QDate::currentDate().addYears(1));
    ui->dateExpiry->setMinimumDate(QDate::currentDate());

    // 设置数值范围
    ui->spinQuantity->setMinimum(1);
    ui->spinQuantity->setMaximum(10000);
    ui->spinQuantity->setValue(1);

    // 设置表格
    ui->txtNotes->setPlaceholderText("请输入备注信息...");
}

int StockDialog::getQuantity() const
{
    return ui->spinQuantity->value();
}

QString StockDialog::getBatchNumber() const
{
    return ui->txtBatchNumber->text().trimmed();
}

QDate StockDialog::getExpiryDate() const
{
    return ui->dateExpiry->date();
}

QString StockDialog::getNotes() const
{
    return ui->txtNotes->toPlainText().trimmed();
}

void StockDialog::on_btnConfirm_clicked()
{
    int quantity = ui->spinQuantity->value();

    if (quantity <= 0) {
        QMessageBox::warning(this, "错误", "数量必须大于0");
        return;
    }

    if (operationType == StockIn && ui->txtBatchNumber->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "错误", "请填写批次号");
        return;
    }

    accept();
}

void StockDialog::on_btnCancel_clicked()
{
    reject();
}
