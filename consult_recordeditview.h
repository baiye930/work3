#ifndef CONSULTRECORDEDITVIEW_H
#define CONSULTRECORDEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class ConsultRecordEditView;
}

class ConsultRecordEditView : public QWidget
{
    Q_OBJECT

public:
    explicit ConsultRecordEditView(QWidget *parent = nullptr, int index = 0);
    ~ConsultRecordEditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();
    void on_btnSelectPatient_clicked();
    void on_btnSelectDoctor_clicked();
    void on_btnSelectPrescription_clicked();

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    bool validateInput();
    void showError(const QString &message);
    void clearError();
    void updateConsultFee();

    Ui::ConsultRecordEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewConsultRecord;
    QString currentRecordId;
};

#endif // CONSULTRECORDEDITVIEW_H
