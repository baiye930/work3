#ifndef MEDICINESELECTDIALOG_H
#define MEDICINESELECTDIALOG_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
class MedicineSelectDialog;
}

class MedicineSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MedicineSelectDialog(QWidget *parent = nullptr);
    ~MedicineSelectDialog();

    // 获取选中的药品ID
    QString getSelectedMedicineId() const;
    // 获取选中的药品名称
    QString getSelectedMedicineName() const;
    // 获取数量
    int getQuantity() const;
    // 获取用法用量
    QString getDosage() const;

private slots:
    void on_btnSearch_clicked();
    void on_txtSearch_returnPressed();
    void on_btnSelect_clicked();
    void on_btnCancel_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_spinQuantity_valueChanged(int arg1);

private:
    void initTableView();
    void updateMedicineInfo();
    void calculateTotal();

    Ui::MedicineSelectDialog *ui;
    QSqlTableModel *medicineModel;
    QString selectedMedicineId;
    QString selectedMedicineName;
    double unitPrice;
};

#endif // MEDICINESELECTDIALOG_H
