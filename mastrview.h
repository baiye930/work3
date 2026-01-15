#ifndef MASTRVIEW_H
#define MASTRVIEW_H

#include <QWidget>
#include "loginview.h"
#include "doctorview.h"
#include "departmentview.h"
#include "patienteditview.h"
#include "patientview.h"
#include "welcomview.h"
#include "appointmentview.h"
#include "consult_recordview.h"
#include "medicineview.h"
#include "prescriptionview.h"
#include "doctoreditview.h"
#include "departmenteditview.h"
#include "medicineeditview.h"
#include "prescriptioneditview.h"
#include "appointmenteditview.h"
#include "consult_recordeditview.h"  // 确保包含这个头文件

QT_BEGIN_NAMESPACE
namespace Ui {
class MastrView;
}
QT_END_NAMESPACE

class MastrView : public QWidget
{
    Q_OBJECT

public:
    MastrView(QWidget *parent = nullptr);
    ~MastrView();

public slots:
    void goLoginView();
    void goWelcomView();
    void goDoctorView();
    void goDepartmentView();
    void goPatientEditView(int rowNo);
    void goPatientView();
    void goPreviousView();
    void goPrescriptionview();
    void gomedicineview();
    void goconsult_recordview();
    void goappointmentview();
    void goDoctorEditView(int rowNo);
    void goDepartmentEditView(int rowNo);
    void goMedicineEditView(int rowNo);
    void goPrescriptionEditView(int rowNo);
    void goAppointmentEditView(int rowNo);
    void goConsultRecordEditView(int rowNo);  // 添加这个槽函数

private slots:
    void on_stackedWidget_currentChanged(int arg1);
    void on_btBack_clicked();
    void on_btLogout_clicked();

private:
    void pushWidgetToStackView(QWidget *widget);

    Ui::MastrView *ui;

    WelcomView *welcomeView;
    DoctorView *doctorView;
    PatientView *patientView;
    DepartmentView *departmentView;
    LoginView *loginView;
    PatientEditView *patientEditView;
    appointmentview *appointmentView;
    consult_recordview *consult_recordView;
    medicineview *medicineView;
    Prescriptionview *prescriptionView;
    DoctoreditView *doctorEditView;
    DepartmentEditView *departmentEditView;
    MedicineEditView *medicineEditView;
    PrescriptionEditView *prescriptionEditView;
    AppointmentEditView *appointmentEditView;
    ConsultRecordEditView *consultRecordEditView;  // 使用正确的类名
};

#endif // MASTRVIEW_H
