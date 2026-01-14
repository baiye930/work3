#include "departmentview.h"
#include "ui_departmentview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>

DepartmentView::DepartmentView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DepartmentView)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    updateCount();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &DepartmentView::on_tableView_doubleClicked);

    // 设置回车键搜索
    connect(ui->txtSearch, &QLineEdit::returnPressed,
            this, &DepartmentView::on_txtSearch_returnPressed);
}

DepartmentView::~DepartmentView()
{
    delete ui;
}

void DepartmentView::initTableView()
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
    if (iDatabase.initDepartmentModel()) {
        ui->tableView->setModel(iDatabase.departmentTabModel);
        ui->tableView->setSelectionModel(iDatabase.theDepartmentSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.departmentTabModel->fieldIndex("created_time"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("name"), 150);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("location"), 120);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("phone"), 120);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("description"), 200);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("director_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("established_date"), 100);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("bed_count"), 80);
        ui->tableView->setColumnWidth(iDatabase.departmentTabModel->fieldIndex("status"), 80);
    } else {
        QMessageBox::warning(this, "错误", "初始化科室数据失败");
    }
}

void DepartmentView::updateCount()
{
    IDatabase &iDatabase = IDatabase::getInstance();
    QSqlQuery query("SELECT COUNT(*) FROM department");

    int count = 0;
    if (query.exec() && query.next()) {
        count = query.value(0).toInt();
    }

    ui->lblCount->setText(QString("总计：%1个科室").arg(count));
}

void DepartmentView::refreshData()
{
    IDatabase::getInstance().departmentTabModel->select();
    updateCount();
}

void DepartmentView::on_btnSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (IDatabase::getInstance().searchDepartment(searchText)) {
        updateCount();
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索科室信息失败");
    }
}

void DepartmentView::on_btnAdd_clicked()
{
    // 新增科室
    int currow = IDatabase::getInstance().addNewDepartment();
    qDebug() << "新增科室，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goDepartmentEditView(currow);
}

void DepartmentView::on_btnDelete_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theDepartmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的科室");
        return;
    }

    QString deptName = IDatabase::getInstance().departmentTabModel
                           ->record(currentIndex.row()).value("name").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除科室 '%1' 吗？").arg(deptName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentDepartment()) {
            refreshData();
            QMessageBox::information(this, "成功", "科室删除成功");
        } else {
            QMessageBox::warning(this, "错误",
                                 "无法删除科室，该科室可能包含医生或有未完成的预约。建议将状态改为inactive而不是删除。");
        }
    }
}

void DepartmentView::on_btnEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theDepartmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的科室");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goDepartmentEditView(currentIndex.row());
}

void DepartmentView::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}

void DepartmentView::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btnEdit_clicked();
}
