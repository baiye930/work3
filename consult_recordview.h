#ifndef CONSULT_RECORDVIEW_H
#define CONSULT_RECORDVIEW_H

#include <QWidget>
#include <QFileDialog>
#include <QDir>

namespace Ui {
class consult_recordview;
}

class consult_recordview : public QWidget
{
    Q_OBJECT

public:
    explicit consult_recordview(QWidget *parent = nullptr);
    ~consult_recordview();

private slots:
    // 按钮点击槽函数
    void on_btnSearch_clicked();
    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnView_clicked();
    void on_btnPrint_clicked();
    void on_btnPrescription_clicked();
    void on_btnExport_clicked();
    void on_btnStatistics_clicked();

    // 其他槽函数
    void on_txtSearch_returnPressed();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_dateFilter_dateChanged(const QDate &date);
    void on_cmbDepartment_currentIndexChanged(int index);

signals:
    // 信号：跳转到就诊记录编辑界面
    void goConsultRecordEditView(int rowNo);

private:
    void initTableView();
    void updateStats();
    void refreshData();
    void populateDepartmentFilter();

    Ui::consult_recordview *ui;
};

#endif // CONSULTRECORDVIEW_H
