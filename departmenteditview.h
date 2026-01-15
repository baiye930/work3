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

    QString getCurrentDepartmentId() const;  // 新增方法

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
    void loadDepartmentData();  // 新增方法

    Ui::DepartmentEditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewDepartment;
    QString currentDepartmentId;  // 存储当前科室ID
};

#endif // DEPARTMENTEDITVIEW_H
