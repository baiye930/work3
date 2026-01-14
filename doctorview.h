#ifndef DOCTORVIEW_H
#define DOCTORVIEW_H

#include <QWidget>

namespace Ui {
class DoctorView;
}

class DoctorView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorView(QWidget *parent = nullptr);
    ~DoctorView();

private slots:
    void on_btSearch_clicked();
    void on_btAdd_clicked();
    void on_btDelete_clicked();
    void on_btEdit_clicked();
    void on_btnRefresh_clicked();
    void on_cmbDepartmentFilter_currentIndexChanged(int index);
    void on_txtSearch_returnPressed();
    void on_btnExport_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

signals:
    // 与患者管理模块保持一致的信号
    void goDoctorEditView(int rowNo);

private:
    void initTableView();
    void updateStats();
    void loadDepartmentFilter();

private:
    Ui::DoctorView *ui;
    bool isNewDoctor;  // 跟踪是否是新增模式
    int currentRow;    // 当前行号
};

#endif // DOCTORVIEW_H
