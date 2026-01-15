#ifndef PRESCRIPTIONEDITVIEW_H
#define PRESCRIPTIONEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class PrescriptionEditView;
}

class PrescriptionEditView : public QWidget
{
    Q_OBJECT

public:
    explicit PrescriptionEditView(QWidget *parent = nullptr, int index = 0);
    ~PrescriptionEditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();
    void on_btnAddMedicine_clicked();
    void on_btnDeleteMedicine_clicked();
    void on_btnSelectPatient_clicked();
    void on_btnSelectDoctor_clicked();

    // 新增：计算总金额
    void updateTotalAmount();

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    void loadPrescriptionDetails();
    bool validateInput();
    void showError(const QString &message);
    void clearError();

    // 新增：添加药品到表格
    void addMedicineToTable(const QString &medicineId, const QString &medicineName,
                            int quantity, const QString &dosage, double unitPrice);

    Ui::PrescriptionEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewPrescription;
    QString currentPrescriptionId;
};

#endif // PRESCRIPTIONEDITVIEW_H
