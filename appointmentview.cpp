#include "appointmentview.h"
#include "ui_appointmentview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

appointmentview::appointmentview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::appointmentview)
    , isDataLoaded(false)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    initButtons();

    // 加载数据
    loadAppointmentData();
}

appointmentview::~appointmentview()
{
    delete ui;
}

void appointmentview::initTableView()
{
    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);

    // 设置右键菜单
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void appointmentview::initButtons()
{
    // 设置按钮图标和工具提示
    ui->btnSearch->setToolTip("搜索预约");
    ui->btnAdd->setToolTip("新增预约");
    ui->btnDelete->setToolTip("删除预约");
    ui->btnEdit->setToolTip("编辑预约");

    // 连接信号槽
    connect(ui->btnSearch, &QPushButton::clicked, this, &appointmentview::on_btnSearch_clicked);
    connect(ui->btnAdd, &QPushButton::clicked, this, &appointmentview::on_btnAdd_clicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &appointmentview::on_btnDelete_clicked);
    connect(ui->btnEdit, &QPushButton::clicked, this, &appointmentview::on_btnEdit_clicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &appointmentview::on_tableView_doubleClicked);
    // 连接刷新按钮
    if (ui->btnRefresh) {
        connect(ui->btnRefresh, &QPushButton::clicked, this, &appointmentview::refreshData);
    }
}

void appointmentview::loadAppointmentData()
{
    qDebug() << "开始加载预约数据...";

    // 初始化预约模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initAppointmentModel()) {
        qDebug() << "预约模型初始化成功";

        // 设置模型到表格
        ui->tableView->setModel(iDatabase.appointmentTabModel);
        ui->tableView->setSelectionModel(iDatabase.theAppointmentSelection);

        // 设置自定义表头（将ID字段转换为可读名称）
        setupTableViewHeaders();

        // 更新统计信息
        updateCount();

        isDataLoaded = true;
        qDebug() << "预约数据加载完成，记录数：" << iDatabase.appointmentTabModel->rowCount();
    } else {
        qDebug() << "预约模型初始化失败";
        QMessageBox::warning(this, "错误", "无法加载预约数据。请检查数据库连接和表结构。");
    }
}

void appointmentview::setupTableViewHeaders()
{
    IDatabase &iDatabase = IDatabase::getInstance();
    QSqlTableModel *model = iDatabase.appointmentTabModel;

    if (!model) {
        qDebug() << "模型为空，无法设置表头";
        return;
    }

    qDebug() << "开始设置表头，字段数：" << model->columnCount();

    // 隐藏不需要显示的字段
    for (int i = 0; i < model->columnCount(); i++) {
        QString fieldName = model->headerData(i, Qt::Horizontal).toString();
        qDebug() << "字段" << i << ":" << fieldName;

        // 设置合适的列宽
        if (fieldName == "预约号" || fieldName == "appointment_number") {
            ui->tableView->setColumnWidth(i, 150);
        } else if (fieldName == "患者" || fieldName == "patient_id") {
            ui->tableView->setColumnWidth(i, 120);
        } else if (fieldName == "医生" || fieldName == "doctor_id") {
            ui->tableView->setColumnWidth(i, 120);
        } else if (fieldName == "预约日期" || fieldName == "appointment_date") {
            ui->tableView->setColumnWidth(i, 100);
        } else if (fieldName == "预约时间" || fieldName == "time_slot") {
            ui->tableView->setColumnWidth(i, 80);
        } else if (fieldName == "状态" || fieldName == "status") {
            ui->tableView->setColumnWidth(i, 80);
        } else if (fieldName == "事由" || fieldName == "reason") {
            ui->tableView->setColumnWidth(i, 150);
        } else if (fieldName == "创建时间" || fieldName == "created_time") {
            ui->tableView->setColumnWidth(i, 120);
        } else {
            // 隐藏其他不常用字段
            ui->tableView->hideColumn(i);
        }
    }
}

void appointmentview::refreshData()
{
    if (isDataLoaded) {
        IDatabase &iDatabase = IDatabase::getInstance();
        if (iDatabase.appointmentTabModel) {
            iDatabase.appointmentTabModel->select();
            updateCount();
            qDebug() << "数据已刷新，记录数：" << iDatabase.appointmentTabModel->rowCount();
        }
    }
}

void appointmentview::updateCount()
{
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.appointmentTabModel) {
        int count = iDatabase.appointmentTabModel->rowCount();
        ui->lblCount->setText(QString("总计：%1 条预约").arg(count));

        // 获取今日预约统计
        QMap<QString, QVariant> stats = iDatabase.getTodayAppointmentStats();
        int todayCount = stats.value("today_count", 0).toInt();

        // 更新状态栏信息
        QString statusText = QString("今日预约：%1 条 | 总计：%2 条").arg(todayCount).arg(count);
        // 如果存在状态栏标签，可以更新它
    }
}

void appointmentview::on_btnSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    qDebug() << "搜索预约：" << searchText;

    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.searchAppointment(searchText)) {
        updateCount();
        qDebug() << "搜索完成，显示" << iDatabase.appointmentTabModel->rowCount() << "条记录";
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索预约信息失败");
    }
}

void appointmentview::on_btnAdd_clicked()
{
    qDebug() << "点击新增预约按钮";

    // 新增预约
    int currow = IDatabase::getInstance().addNewAppointment();
    qDebug() << "新增预约，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goAppointmentEditView(currow);
}

void appointmentview::on_btnDelete_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的预约");
        return;
    }

    // 获取预约信息
    QString appointmentNumber = IDatabase::getInstance().appointmentTabModel
                                    ->record(currentIndex.row()).value("appointment_number").toString();
    if (appointmentNumber.isEmpty()) {
        appointmentNumber = "未知预约";
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除预约 '%1' 吗？").arg(appointmentNumber),
                                  QMessageBox::Yes | QMessageBox::No);  // 修正后的代码

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentAppointment()) {
            refreshData();
            QMessageBox::information(this, "成功", "预约删除成功");
        } else {
            QMessageBox::warning(this, "错误",
                                 "无法删除该预约，可能状态不允许删除。");
        }
    }
}

void appointmentview::on_btnEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的预约");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goAppointmentEditView(currentIndex.row());
}

void appointmentview::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btnEdit_clicked();
}
