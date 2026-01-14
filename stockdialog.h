#ifndef STOCKDIALOG_H
#define STOCKDIALOG_H

#include <QDialog>

namespace Ui {
class StockDialog;
}

class StockDialog : public QDialog
{
    Q_OBJECT

public:
    enum OperationType {
        StockIn,
        StockOut,
        StockAdjust
    };

    explicit StockDialog(QWidget *parent = nullptr, OperationType type = StockIn,
                         const QString &medicineName = "", const QString &medicineId = "");
    ~StockDialog();

    int getQuantity() const;
    QString getBatchNumber() const;
    QDate getExpiryDate() const;
    QString getNotes() const;

private slots:
    void on_btnConfirm_clicked();
    void on_btnCancel_clicked();

private:
    void initUI();

    Ui::StockDialog *ui;
    OperationType operationType;
    QString medicineId;
};

#endif // STOCKDIALOG_H
