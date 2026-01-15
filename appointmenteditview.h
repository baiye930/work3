#ifndef APPOINTMENTEDITVIEW_H
#define APPOINTMENTEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>
#include <QMap>

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
    void on_txtPatientId_editingFinished();
    void on_txtDoctorId_editingFinished();
    void on_btnSearchPatient_clicked();
    void on_btnSearchDoctor_clicked();
    void on_dateAppointmentDate_dateChanged(const QDate &date);
    void on_timeAppointmentTime_timeChanged(const QTime &time);

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    bool validateInput();
    void showError(const QString &message);
    void clearError();
    void loadAppointmentData();
    void checkTimeConflict();
    void updateTimeConflictWarning();

    // 查询患者/医生信息
    void searchPatientInfo(const QString &patientId);
    void searchDoctorInfo(const QString &doctorId);
    // 显示查询结果
    void showPatientInfo(const QString &name, bool found);
    void showDoctorInfo(const QString &name, bool found);

    Ui::AppointmentEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewAppointment;
    QString currentAppointmentId;

    // 时间冲突标志
    bool hasTimeConflict;

    // 存储ID映射（用于编辑时显示）
    QMap<QString, QString> patientIdMap;  // ID -> 姓名
    QMap<QString, QString> doctorIdMap;   // ID -> 姓名
};

#endif // APPOINTMENTEDITVIEW_H
