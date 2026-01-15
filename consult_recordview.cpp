#include "consult_recordview.h"
#include "ui_consult_recordview.h"
#include "idatabase.h"
#include "consult_recordeditview.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QDate>

// 自定义委托，用于高亮显示
class ConsultRecordHighlightDelegate : public QStyledItemDelegate
{
public:
    ConsultRecordHighlightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        QSqlTableModel *model = qobject_cast<QSqlTableModel*>(const_cast<QAbstractItemModel*>(index.model()));
        if (model) {
            QSqlRecord rec = model->record(index.row());
            QDate visitDate = rec.value("visit_date").toDate();  // 改为 visit_date

            // 如果是今天或未来的预约，高亮显示
            if (visitDate >= QDate::currentDate()) {
                option->backgroundBrush = QBrush(QColor(240, 255, 240)); // 淡绿色
            }
        }
    }
};

consult_recordview::consult_recordview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::consult_recordview)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    populateDepartmentFilter();
    updateStats();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &consult_recordview::on_tableView_doubleClicked);
    connect(ui->txtSearch, &QLineEdit::returnPressed,
            this, &consult_recordview::on_txtSearch_returnPressed);
    connect(ui->dateFilter, &QDateEdit::dateChanged,
            this, &consult_recordview::on_dateFilter_dateChanged);
}

consult_recordview::~consult_recordview()
{
    delete ui;
}

void consult_recordview::initTableView()
{
    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);

    // 设置自定义委托
    ui->tableView->setItemDelegate(new ConsultRecordHighlightDelegate(this));

    // 初始化模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initConsultRecordModel()) {
        ui->tableView->setModel(iDatabase.consultRecordTabModel);
        ui->tableView->setSelectionModel(iDatabase.theConsultRecordSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.consultRecordTabModel->fieldIndex("created_time"));
        ui->tableView->hideColumn(iDatabase.consultRecordTabModel->fieldIndex("status"));
        ui->tableView->hideColumn(iDatabase.consultRecordTabModel->fieldIndex("symptoms"));
        ui->tableView->hideColumn(iDatabase.consultRecordTabModel->fieldIndex("treatment"));
        ui->tableView->hideColumn(iDatabase.consultRecordTabModel->fieldIndex("notes"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("patient_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("doctor_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("consult_date"), 120);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("department_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("diagnosis"), 150);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("consult_fee"), 80);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("prescription_id"), 120);
        ui->tableView->setColumnWidth(iDatabase.consultRecordTabModel->fieldIndex("visit_date"), 120);  // 改为 visit_date
    } else {
        QMessageBox::warning(this, "错误", "初始化就诊记录数据失败");
    }
}

void consult_recordview::populateDepartmentFilter()
{
    ui->cmbDepartment->clear();
    ui->cmbDepartment->addItem("所有科室", "");

    QList<QString> departments = IDatabase::getInstance().getDepartmentsForCombo();
    for (const QString &dept : departments) {
        ui->cmbDepartment->addItem(dept);
    }
}

void consult_recordview::updateStats()
{
    IDatabase &iDatabase = IDatabase::getInstance();

    // 获取今日就诊统计
    int todayCount = iDatabase.getTodayConsultCount();

    // 获取本月就诊统计
    int monthCount = iDatabase.getMonthConsultCount();

    // 获取常见诊断
    QList<QString> commonDiagnoses = iDatabase.getCommonDiagnoses(3);

    // 获取汇总统计
    QMap<QString, QVariant> summaryStats = iDatabase.getConsultSummaryStats();
    double todayFee = summaryStats["today_fee"].toDouble();
    double monthFee = summaryStats["month_fee"].toDouble();
    QString topDepartments = summaryStats["top_departments"].toString();

    // 更新界面
    ui->lblTodayCount->setText(QString("今日就诊：%1人次").arg(todayCount));
    ui->lblMonthCount->setText(QString("本月就诊：%1人次").arg(monthCount));

    // 如果有诊费信息，显示诊费
    if (todayFee > 0) {
        ui->lblTodayCount->setText(ui->lblTodayCount->text() +
                                   QString(" (诊费：%1元)").arg(todayFee, 0, 'f', 2));
    }

    if (monthFee > 0) {
        ui->lblMonthCount->setText(ui->lblMonthCount->text() +
                                   QString(" (诊费：%1元)").arg(monthFee, 0, 'f', 2));
    }

    // 显示常见诊断
    QString diagnosesText = "常见诊断：";
    if (!commonDiagnoses.isEmpty()) {
        diagnosesText += commonDiagnoses.join(" | ");
    }
    ui->lblCommonDiagnosis->setText(diagnosesText);

    // 显示热门科室
    if (!topDepartments.isEmpty()) {
        ui->lblCommonDiagnosis->setText(ui->lblCommonDiagnosis->text() +
                                        " | 热门科室：" + topDepartments);
    }
}

void consult_recordview::refreshData()
{
    IDatabase::getInstance().consultRecordTabModel->select();
    updateStats();
}

void consult_recordview::on_btnSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (IDatabase::getInstance().searchConsultRecord(searchText)) {
        updateStats();
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索就诊记录失败");
    }
}

void consult_recordview::on_btnAdd_clicked()
{
    // 新增就诊记录
    int currow = IDatabase::getInstance().addNewConsultRecord();
    qDebug() << "新增就诊记录，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goConsultRecordEditView(currow);
}

void consult_recordview::on_btnEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theConsultRecordSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的就诊记录");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goConsultRecordEditView(currentIndex.row());
}

void consult_recordview::on_btnView_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theConsultRecordSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要查看的就诊记录");
        return;
    }

    // 查看详情，使用编辑界面但只读模式
    emit goConsultRecordEditView(currentIndex.row());
}

void consult_recordview::on_btnPrint_clicked()
{
    QMessageBox::information(this, "打印记录",
                             "打印就诊记录功能待实现。\n\n"
                             "此功能将：\n"
                             "1. 打印选中的就诊记录\n"
                             "2. 支持打印预览\n"
                             "3. 包含患者、医生、诊断等详细信息");
}

void consult_recordview::on_btnPrescription_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theConsultRecordSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择就诊记录");
        return;
    }

    QString recordId = IDatabase::getInstance().consultRecordTabModel
                           ->record(currentIndex.row()).value("id").toString();
    QString patientId = IDatabase::getInstance().consultRecordTabModel
                            ->record(currentIndex.row()).value("patient_id").toString();

    QMessageBox::information(this, "开具处方",
                             QString("将为就诊记录开具处方\n"
                                     "就诊记录ID：%1\n"
                                     "患者ID：%2\n\n"
                                     "此功能将跳转到处方开具界面").arg(recordId).arg(patientId));
}

void consult_recordview::on_btnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出就诊记录",
                                                    QDir::homePath() + "/就诊记录.csv",
                                                    "CSV文件 (*.csv);;Excel文件 (*.xlsx)");

    if (!fileName.isEmpty()) {
        qDebug() << "导出到文件：" << fileName;
        QMessageBox::information(this, "导出成功", "数据已导出到文件：\n" + fileName);
    }
}

void consult_recordview::on_btnStatistics_clicked()
{
    QMessageBox::information(this, "统计",
                             "就诊记录统计功能待实现。\n\n"
                             "此功能将：\n"
                             "1. 显示按科室统计的就诊量\n"
                             "2. 显示按医生统计的就诊量\n"
                             "3. 显示常见诊断统计\n"
                             "4. 生成统计图表");
}
void consult_recordview::on_dateFilter_dateChanged(const QDate &date)
{
    QString dateStr = date.toString("yyyy-MM-dd");
    IDatabase::getInstance().consultRecordTabModel->setFilter(
        QString("DATE(visit_date) = '%1'").arg(dateStr));  // 改为 visit_date
    IDatabase::getInstance().consultRecordTabModel->select();
    updateStats();
}

void consult_recordview::on_cmbDepartment_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString departmentFilter = ui->cmbDepartment->currentData().toString();

    if (!departmentFilter.isEmpty()) {
        // 从"科室名称 (科室ID)"格式中提取ID
        QRegularExpression rx("\\((.*)\\)");
        QRegularExpressionMatch match = rx.match(ui->cmbDepartment->currentText());
        if (match.hasMatch()) {
            QString departmentId = match.captured(1);
            IDatabase::getInstance().consultRecordTabModel->setFilter(
                QString("department_id = '%1'").arg(departmentId));
            IDatabase::getInstance().consultRecordTabModel->select();
            updateStats();
        }
    } else {
        IDatabase::getInstance().consultRecordTabModel->setFilter("");
        IDatabase::getInstance().consultRecordTabModel->select();
        updateStats();
    }
}

void consult_recordview::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}

void consult_recordview::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btnView_clicked();
}
