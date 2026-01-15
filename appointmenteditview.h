#ifndef APPOINTMENTEDITVIEW_H
#define APPOINTMENTEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class AppointmentEditView;
}

class AppointmentEditView : public QWidget
{
    Q_OBJECT

public:
    explicit AppointmentEditView(QWidget *parent = nullptr, int index = 0);
    ~AppointmentEditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();
    void on_btnSelectPatient_clicked();
    void on_btnSelectDoctor_clicked();
    void on_dateAppointmentDate_dateChanged(const QDate &date);
    void on_cmbDoctor_currentIndexChanged(int index);

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    bool validateInput();
    void showError(const QString &message);
    void clearError();
    void loadDoctorSchedule();
    void checkTimeConflict();

    Ui::AppointmentEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewAppointment;
    QString currentAppointmentId;
};

#endif // APPOINTMENTEDITVIEW_H
