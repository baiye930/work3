#include "doctorview.h"
#include "ui_doctorview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>

DoctorView::DoctorView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorView)
    , isNewDoctor(false)
    , currentRow(-1)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    loadDepartmentFilter();
    updateStats();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &DoctorView::on_tableView_doubleClicked);
}

DoctorView::~DoctorView()
{
    delete ui;
}

void DoctorView::initTableView()
{
    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);

    // 设置右键菜单
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 初始化模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initDoctorModel()) {
        ui->tableView->setModel(iDatabase.doctorTabModel);
        ui->tableView->setSelectionModel(iDatabase.theDoctorSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.doctorTabModel->fieldIndex("birth_date"));
        ui->tableView->hideColumn(iDatabase.doctorTabModel->fieldIndex("qualification"));
        ui->tableView->hideColumn(iDatabase.doctorTabModel->fieldIndex("license_number"));
        ui->tableView->hideColumn(iDatabase.doctorTabModel->fieldIndex("created_time"));
        ui->tableView->hideColumn(iDatabase.doctorTabModel->fieldIndex("schedule"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("employee_id"), 80);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("name"), 100);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("gender"), 60);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("department_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("title"), 80);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("specialization"), 150);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("phone"), 120);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("email"), 150);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("work_years"), 80);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("consultation_fee"), 80);
        ui->tableView->setColumnWidth(iDatabase.doctorTabModel->fieldIndex("status"), 60);
    } else {
        QMessageBox::warning(this, "错误", "初始化医生数据失败");
    }
}

void DoctorView::loadDepartmentFilter()
{
    ui->cmbDepartmentFilter->clear();
    ui->cmbDepartmentFilter->addItem("所有科室", "");

    QList<QString> departments = IDatabase::getInstance().getDepartmentsForCombo();
    for (const QString &dept : departments) {
        ui->cmbDepartmentFilter->addItem(dept);
    }
}

void DoctorView::updateStats()
{
    IDatabase &iDatabase = IDatabase::getInstance();
    QSqlQuery query;

    int total = 0, active = 0, leave = 0, retired = 0;

    if (query.exec("SELECT status, COUNT(*) FROM doctor GROUP BY status")) {
        while (query.next()) {
            QString status = query.value(0).toString();
            int count = query.value(1).toInt();
            total += count;

            if (status == "active") active = count;
            else if (status == "leave") leave = count;
            else if (status == "retired") retired = count;
        }
    }

    ui->lblStats->setText(QString("总计：%1 | 在职：%2 | 休假：%3 | 退休：%4")
                              .arg(total).arg(active).arg(leave).arg(retired));
}

void DoctorView::on_btSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (IDatabase::getInstance().searchDoctor(searchText)) {
        updateStats();
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索医生信息失败");
    }
}

void DoctorView::on_btAdd_clicked()
{
    // 新增医生
    int currow = IDatabase::getInstance().addNewDoctor();
    qDebug() << "新增医生，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goDoctorEditView(currow);
}

void DoctorView::on_btDelete_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theDoctorSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的医生");
        return;
    }

    QString doctorName = IDatabase::getInstance().doctorTabModel
                             ->record(currentIndex.row()).value("name").toString();
    QString employeeId = IDatabase::getInstance().doctorTabModel
                             ->record(currentIndex.row()).value("employee_id").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除医生 '%1 (工号: %2)' 吗？")
                                      .arg(doctorName).arg(employeeId),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentDoctor()) {
            IDatabase::getInstance().doctorTabModel->select();
            updateStats();
            QMessageBox::information(this, "成功", "医生删除成功");
        } else {
            QMessageBox::warning(this, "错误", "无法删除医生，可能存在关联数据或该医生有未完成的预约");
        }
    }
}

void DoctorView::on_btEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theDoctorSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的医生");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    currentRow = currentIndex.row();
    emit goDoctorEditView(currentRow);
}

void DoctorView::on_btnRefresh_clicked()
{
    if (IDatabase::getInstance().doctorTabModel->select()) {
        updateStats();
        ui->txtSearch->clear();
        ui->cmbDepartmentFilter->setCurrentIndex(0);
        QMessageBox::information(this, "刷新", "医生列表已刷新");
    } else {
        QMessageBox::warning(this, "刷新失败", "刷新医生列表失败");
    }
}

void DoctorView::on_cmbDepartmentFilter_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString departmentFilter = ui->cmbDepartmentFilter->currentData().toString();

    if (!departmentFilter.isEmpty()) {
        IDatabase::getInstance().doctorTabModel->setFilter(
            QString("department_id LIKE '%%1%'").arg(departmentFilter));
        IDatabase::getInstance().doctorTabModel->select();
    }
}

void DoctorView::on_txtSearch_returnPressed()
{
    on_btSearch_clicked();
}

void DoctorView::on_btnExport_clicked()
{
    // 简化的导出功能 - 实际项目中可以使用QTextDocument或第三方库
    QMessageBox::information(this, "导出", "导出功能待实现");
}

void DoctorView::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btEdit_clicked();
}
