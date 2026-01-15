#ifndef PRESCRIPTIONVIEW_H
#define PRESCRIPTIONVIEW_H

#include <QWidget>

namespace Ui {
class Prescriptionview;
}

class Prescriptionview : public QWidget
{
    Q_OBJECT

public:
    explicit Prescriptionview(QWidget *parent = nullptr);
    ~Prescriptionview();

private slots:
    void on_btnSearch_clicked();
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnDispense_clicked();
    void on_btnPayment_clicked();
    void on_btnAudit_clicked();
    void on_btnView_clicked();
    void on_dateFilter_dateChanged(const QDate &date);
    void on_cmbStatus_currentIndexChanged(int index);
    void on_cmbDoctor_currentIndexChanged(int index);
    void on_txtSearch_returnPressed();
    void on_tableView_doubleClicked(const QModelIndex &index);

signals:
    void goPrescriptionEditView(int rowNo);

private:
    void initTableView();
    void updateStats();
    void refreshData();
    void populateStatusFilter();
    void populateDoctorFilter();
    void highlightStatus();

    Ui::Prescriptionview *ui;
};

#endif // PRESCRIPTIONVIEW_H
