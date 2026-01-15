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
    void on_btnSearch_clicked();
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnEdit_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void refreshData();
    void updateCount();

signals:
    void goAppointmentEditView(int rowNo);

private:
    void initTableView();
    void initButtons();
    void loadAppointmentData();
    void setupTableViewHeaders();  // 添加这个声明

    Ui::appointmentview *ui;
    bool isDataLoaded;
};

#endif // APPOINTMENTVIEW_H
