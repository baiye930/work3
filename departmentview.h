#ifndef DEPARTMENTVIEW_H
#define DEPARTMENTVIEW_H

#include <QWidget>

namespace Ui {
class DepartmentView;
}

class DepartmentView : public QWidget
{
    Q_OBJECT

public:
    explicit DepartmentView(QWidget *parent = nullptr);
    ~DepartmentView();

private slots:
    void on_btnSearch_clicked();
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnEdit_clicked();
    void on_txtSearch_returnPressed();
    void on_tableView_doubleClicked(const QModelIndex &index);

signals:
    void goDepartmentEditView(int rowNo);

private:
    void initTableView();
    void updateCount();
    void refreshData();

    Ui::DepartmentView *ui;
};

#endif // DEPARTMENTVIEW_H
