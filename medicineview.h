#ifndef MEDICINEVIEW_H
#define MEDICINEVIEW_H

#include <QWidget>

namespace Ui {
class medicineview;
}

class medicineview : public QWidget
{
    Q_OBJECT

public:
    explicit medicineview(QWidget *parent = nullptr);
    ~medicineview();

private slots:
    void on_btSearch_clicked();
    void on_btAdd_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();
    void on_btnLowStock_clicked();
    void on_btnNearExpiry_clicked();
    void on_btnStockIn_clicked();
    void on_btnStockOut_clicked();
    void on_btnAdjustStock_clicked();
    void on_cmbCategory_currentIndexChanged(int index);
    void on_txtSearch_returnPressed();
    void on_tableView_doubleClicked(const QModelIndex &index);

signals:
    void goMedicineEditView(int rowNo);

private:
    void initTableView();
    void updateStats();
    void refreshData();
    void populateCategoryFilter();
    void highlightLowStock();
    void highlightNearExpiry();

    Ui::medicineview *ui;
};

#endif // MEDICINEVIEW_H
