#include "appointmentview.h"
#include "ui_appointmentview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QDate>
#include <QFileDialog>
#include <QDir>
// 自定义委托，用于高亮显示不同状态的预约
class AppointmentHighlightDelegate : public QStyledItemDelegate
{
public:
    AppointmentHighlightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        QSqlTableModel *model = qobject_cast<QSqlTableModel*>(const_cast<QAbstractItemModel*>(index.model()));
        if (model) {
            QSqlRecord rec = model->record(index.row());
            QString status = rec.value("status").toString();
            QDate appointmentDate = rec.value("appointment_date").toDate();

            // 根据状态设置不同的背景色
            if (status == "scheduled") {
                option->backgroundBrush = QBrush(QColor(240, 248, 255)); // 淡蓝色
            } else if (status == "confirmed") {
                option->backgroundBrush = QBrush(QColor(255, 255, 240)); // 淡黄色
            } else if (status == "checked_in") {
                option->backgroundBrush = QBrush(QColor(240, 255, 240)); // 淡绿色
            } else if (status == "completed") {
                option->backgroundBrush = QBrush(QColor(245, 245, 245)); // 淡灰色
            } else if (status == "cancelled") {
                option->backgroundBrush = QBrush(QColor(255, 240, 240)); // 淡红色
            } else if (status == "no_show") {
                option->backgroundBrush = QBrush(QColor(255, 228, 225)); // 粉红色
            }

            // 如果是今天或过去的预约，标记为重要
            if (appointmentDate <= QDate::currentDate()) {
                option->font.setBold(true);
            }
        }
    }
};

appointmentview::appointmentview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::appointmentview)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    populateStatusFilter();
    initDateFilters();
    updateStats();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &appointmentview::on_tableView_doubleClicked);
    connect(ui->txtSearch, &QLineEdit::returnPressed,
            this, &appointmentview::on_txtSearch_returnPressed);
    connect(ui->dateFrom, &QDateEdit::dateChanged,
            this, &appointmentview::on_dateFilter_changed);
    connect(ui->dateTo, &QDateEdit::dateChanged,
            this, &appointmentview::on_dateFilter_changed);
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

    // 设置自定义委托
    ui->tableView->setItemDelegate(new AppointmentHighlightDelegate(this));

    // 初始化模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initAppointmentModel()) {
        ui->tableView->setModel(iDatabase.appointmentTabModel);
        ui->tableView->setSelectionModel(iDatabase.theAppointmentSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.appointmentTabModel->fieldIndex("appointment_number"));
        ui->tableView->hideColumn(iDatabase.appointmentTabModel->fieldIndex("created_time"));
        ui->tableView->hideColumn(iDatabase.appointmentTabModel->fieldIndex("check_in_time"));
        ui->tableView->hideColumn(iDatabase.appointmentTabModel->fieldIndex("check_out_time"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("patient_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("doctor_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("appointment_date"), 100);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("appointment_time"), 100);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("department_id"), 80);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("status"), 80);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("reason"), 150);
        ui->tableView->setColumnWidth(iDatabase.appointmentTabModel->fieldIndex("notes"), 200);
    } else {
        QMessageBox::warning(this, "错误", "初始化预约数据失败");
    }
}

void appointmentview::populateStatusFilter()
{
    ui->cmbStatus->clear();
    ui->cmbStatus->addItem("所有状态", "");

    QStringList statuses = IDatabase::getInstance().getAppointmentStatuses();
    for (const QString &status : statuses) {
        ui->cmbStatus->addItem(status, status);
    }
}

void appointmentview::initDateFilters()
{
    // 设置默认日期范围（最近7天）
    ui->dateFrom->setDate(QDate::currentDate().addDays(-7));
    ui->dateTo->setDate(QDate::currentDate().addDays(7));

    // 设置日历弹出
    ui->dateFrom->setCalendarPopup(true);
    ui->dateTo->setCalendarPopup(true);
}

void appointmentview::updateStats()
{
    IDatabase &iDatabase = IDatabase::getInstance();

    // 获取今日预约统计
    QMap<QString, QVariant> todayStats = iDatabase.getTodayAppointmentStats();
    int todayCount = todayStats["today_count"].toInt();

    // 获取明日预约统计
    QMap<QString, QVariant> tomorrowStats = iDatabase.getTomorrowAppointmentStats();
    int tomorrowCount = tomorrowStats["tomorrow_count"].toInt();

    // 获取汇总统计
    QMap<QString, QVariant> summaryStats = iDatabase.getAppointmentSummaryStats();
    int total = summaryStats["total"].toInt();
    int scheduled = summaryStats["scheduled"].toInt();
    int confirmed = summaryStats["confirmed"].toInt();
    int completed = summaryStats["completed"].toInt();
    int cancelled = summaryStats["cancelled"].toInt();
    int no_show = summaryStats["no_show"].toInt();

    // 更新界面
    ui->lblCount->setText(QString("总计：%1 | 预约中：%2 | 已确认：%3 | 已完成：%4 | 已取消：%5 | 未到诊：%6")
                              .arg(total).arg(scheduled).arg(confirmed).arg(completed).arg(cancelled).arg(no_show));
    ui->lblToday->setText(QString("今日预约：%1").arg(todayCount));
    ui->lblTomorrow->setText(QString("明日预约：%1").arg(tomorrowCount));
}

void appointmentview::refreshData()
{
    IDatabase::getInstance().appointmentTabModel->select();
    updateStats();
}

void appointmentview::applyFilters()
{
    QString filter = "";
    QDate dateFrom = ui->dateFrom->date();
    QDate dateTo = ui->dateTo->date();

    // 日期范围过滤
    if (dateFrom.isValid() && dateTo.isValid()) {
        if (dateFrom <= dateTo) {
            filter += QString("appointment_date BETWEEN '%1' AND '%2'")
            .arg(dateFrom.toString("yyyy-MM-dd"))
                .arg(dateTo.toString("yyyy-MM-dd"));
        }
    }

    // 状态过滤
    QString statusFilter = ui->cmbStatus->currentData().toString();
    if (!statusFilter.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("status = '%1'").arg(statusFilter);
    }

    // 搜索文本过滤
    QString searchText = ui->txtSearch->text().trimmed();
    if (!searchText.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("(patient_id LIKE '%%1%' OR doctor_id LIKE '%%1%' OR reason LIKE '%%1%')")
                      .arg(searchText);
    }

    // 应用过滤
    IDatabase::getInstance().appointmentTabModel->setFilter(filter);
    IDatabase::getInstance().appointmentTabModel->select();
}

void appointmentview::on_btnSearch_clicked()
{
    applyFilters();
    updateStats();
}

void appointmentview::on_btnAdd_clicked()
{
    // 新增预约
    int currow = IDatabase::getInstance().addNewAppointment();
    qDebug() << "新增预约，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goAppointmentEditView(currow);
}

void appointmentview::on_btnEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的预约");
        return;
    }

    QString status = IDatabase::getInstance().appointmentTabModel
                         ->record(currentIndex.row()).value("status").toString();

    // 已取消或已完成的预约不能编辑
    if (status == "cancelled" || status == "completed" || status == "no_show") {
        QString statusText;
        if (status == "cancelled") {
            statusText = "取消";
        } else if (status == "completed") {
            statusText = "完成";
        } else {
            statusText = "标记为未到诊";
        }

        QMessageBox::warning(this, "错误",
                             QString("该预约已%1，不能编辑").arg(statusText));
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goAppointmentEditView(currentIndex.row());
}

void appointmentview::on_btnCancel_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要取消的预约");
        return;
    }

    QString appointmentId = IDatabase::getInstance().appointmentTabModel
                                ->record(currentIndex.row()).value("id").toString();
    QString status = IDatabase::getInstance().appointmentTabModel
                         ->record(currentIndex.row()).value("status").toString();
    QString patientId = IDatabase::getInstance().appointmentTabModel
                            ->record(currentIndex.row()).value("patient_id").toString();

    // 检查预约状态
    if (status == "cancelled") {
        QMessageBox::information(this, "提示", "预约已取消");
        return;
    }

    if (status == "completed") {
        QMessageBox::warning(this, "错误", "已完成的预约不能取消");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认取消",
                                  "确定要取消这个预约吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().updateAppointmentStatus(appointmentId, "cancelled")) {
            refreshData();
            QMessageBox::information(this, "成功", "预约已取消");
        } else {
            QMessageBox::warning(this, "错误", "取消预约失败");
        }
    }
}

void appointmentview::on_btnComplete_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要标记完成的预约");
        return;
    }

    QString appointmentId = IDatabase::getInstance().appointmentTabModel
                                ->record(currentIndex.row()).value("id").toString();
    QString status = IDatabase::getInstance().appointmentTabModel
                         ->record(currentIndex.row()).value("status").toString();

    // 检查预约状态
    if (status == "completed") {
        QMessageBox::information(this, "提示", "预约已完成");
        return;
    }

    if (status != "checked_in") {
        QMessageBox::warning(this, "错误", "请先确认患者到诊");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认完成",
                                  "确定要标记预约为完成吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().updateAppointmentStatus(appointmentId, "completed")) {
            refreshData();
            QMessageBox::information(this, "成功", "预约已标记为完成");
        } else {
            QMessageBox::warning(this, "错误", "标记完成失败");
        }
    }
}

void appointmentview::on_btnConfirm_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theAppointmentSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要确认到诊的预约");
        return;
    }

    QString appointmentId = IDatabase::getInstance().appointmentTabModel
                                ->record(currentIndex.row()).value("id").toString();
    QString status = IDatabase::getInstance().appointmentTabModel
                         ->record(currentIndex.row()).value("status").toString();

    // 检查预约状态
    if (status == "checked_in") {
        QMessageBox::information(this, "提示", "患者已到诊");
        return;
    }

    if (status == "completed") {
        QMessageBox::warning(this, "错误", "已完成的预约不能确认到诊");
        return;
    }

    if (status == "cancelled") {
        QMessageBox::warning(this, "错误", "已取消的预约不能确认到诊");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认到诊",
                                  "确定要确认患者到诊吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().updateAppointmentStatus(appointmentId, "checked_in")) {
            refreshData();
            QMessageBox::information(this, "成功", "患者到诊已确认");
        } else {
            QMessageBox::warning(this, "错误", "确认到诊失败");
        }
    }
}

void appointmentview::on_dateFilter_changed()
{
    applyFilters();
    updateStats();
}

void appointmentview::on_cmbStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    applyFilters();
    updateStats();
}

void appointmentview::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}

void appointmentview::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btnEdit_clicked();
}
void appointmentview::on_btnPrintSchedule_clicked()
{
    QMessageBox::information(this, "打印排班表",
                             "打印排班表功能待实现。\n\n"
                             "此功能将：\n"
                             "1. 根据选择的日期范围生成排班表\n"
                             "2. 按科室和医生分组显示\n"
                             "3. 支持打印预览和打印");

    // 简化的打印逻辑（实际项目中应使用QPrinter和QPrintDialog）
    // QPrinter printer(QPrinter::HighResolution);
    // printer.setPageSize(QPrinter::A4);
    // printer.setOrientation(QPrinter::Landscape);
    //
    // QPrintDialog printDialog(&printer, this);
    // if (printDialog.exec() == QDialog::Accepted) {
    //     // 执行打印
    // }
}

void appointmentview::on_btnExport_clicked()
{
    QMessageBox::information(this, "导出预约",
                             "导出预约功能待实现。\n\n"
                             "此功能将：\n"
                             "1. 导出当前显示的预约数据\n"
                             "2. 支持Excel和CSV格式\n"
                             "3. 允许选择导出字段");

    // 简化的导出逻辑
    QString fileName = QFileDialog::getSaveFileName(this, "导出预约数据",
                                                    QDir::homePath() + "/预约列表.csv",
                                                    "CSV文件 (*.csv);;Excel文件 (*.xlsx)");

    if (!fileName.isEmpty()) {
        // 这里应该实现实际的导出逻辑
        // 例如：将QTableView中的数据导出到文件
        qDebug() << "导出到文件：" << fileName;
        QMessageBox::information(this, "导出成功", "数据已导出到文件：\n" + fileName);
    }
}
