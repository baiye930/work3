#ifndef WELCOMVIEW_H
#define WELCOMVIEW_H

#include <QWidget>

namespace Ui {
class WelcomView;
}

class WelcomView : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomView(QWidget *parent = nullptr);
    ~WelcomView();

private slots:
    void on_btDepartment_clicked();

    void on_btDoctor_clicked();

    void on_btPatient_clicked();

signals:
    void goDepartmentView();
    void goDoctorView();
    void goPatientView();

private:
    Ui::WelcomView *ui;
};

#endif // WELCOMVIEW_H
