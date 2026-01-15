#ifndef APPOINTMENTVIEW_H
#define APPOINTMENTVIEW_H

#include <QWidget>

namespace Ui {
class appointmentview;
}

class appointmentview : public QWidget
{
    Q_OBJECT

public:
    explicit appointmentview(QWidget *parent = nullptr);
    ~appointmentview();

private slots:
    // 按钮点击槽函数
    void on_btnSearch_clicked();
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnCancel_clicked();
    void on_btnComplete_clicked();
    void on_btnConfirm_clicked();
    void on_btnPrintSchedule_clicked();
    void on_btnExport_clicked();

    // 其他槽函数
    void on_txtSearch_returnPressed();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_dateFilter_changed(); // 日期范围改变
    void on_cmbStatus_currentIndexChanged(int index);

signals:
    // 信号：跳转到预约编辑界面，参数为行号
    void goAppointmentEditView(int rowNo);

private:
    // 初始化函数
    void initTableView();
    void initDateFilters();
    void populateStatusFilter();
    void updateStats();
    void refreshData();
    void applyFilters();

    Ui::appointmentview *ui;
};

#endif // APPOINTMENTVIEW_H
