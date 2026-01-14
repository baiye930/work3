#ifndef MEDICINEEDITVIEW_H
#define MEDICINEEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class MedicineEditView;
}

class MedicineEditView : public QWidget
{
    Q_OBJECT

public:
    explicit MedicineEditView(QWidget *parent = nullptr, int index = 0);
    ~MedicineEditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    bool validateInput();
    void showError(const QString &message);
    void clearError();
    void calculateExpiryDate();

    Ui::MedicineEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewMedicine;
};

#endif // MEDICINEEDITVIEW_H
