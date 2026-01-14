#ifndef DEPARTMENTEDITVIEW_H
#define DEPARTMENTEDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class DepartmentEditView;
}

class DepartmentEditView : public QWidget
{
    Q_OBJECT

public:
    explicit DepartmentEditView(QWidget *parent = nullptr, int index = 0);
    ~DepartmentEditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();

signals:
    void goPreviousView();

private:
    void initUI();
    bool validateInput();
    void showError(const QString &message);
    void clearError();

    Ui::DepartmentEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewDepartment;
};

#endif // DEPARTMENTEDITVIEW_H
