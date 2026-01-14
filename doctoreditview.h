#ifndef DOCTOREDITVIEW_H
#define DOCTOREDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class DoctoreditView;
}

class DoctoreditView : public QWidget
{
    Q_OBJECT

public:
    // 修改构造函数，与PatientEditView保持一致
    explicit DoctoreditView(QWidget *parent = nullptr, int index = 0);
    ~DoctoreditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();

signals:
    void goPreviousView();

private:
    void initUI();
    void populateComboBoxes();
    void loadDepartmentData();
    bool validateInput();
    void showError(const QString &message);
    void clearError();

    Ui::DoctoreditView *ui;
    QDataWidgetMapper *dataMapper;
    bool isNewDoctor; // 内部判断是否是新增
    QString originalDepartmentText;
};

#endif // DOCTOREDITVIEW_H
