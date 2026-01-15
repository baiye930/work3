#include "prescriptionview.h"
#include "ui_prescriptionview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QInputDialog>

// 自定义委托，用于高亮显示
class PrescriptionHighlightDelegate : public QStyledItemDelegate
{
public:
    PrescriptionHighlightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        // 获取行数据
        QSqlTableModel *model = qobject_cast<QSqlTableModel*>(const_cast<QAbstractItemModel*>(index.model()));
        if (model) {
            QSqlRecord rec = model->record(index.row());
            QString paymentStatus = rec.value("payment_status").toString();
            QString dispensingStatus = rec.value("dispensing_status").toString();

            // 待收费高亮（红色）
            if (paymentStatus == "unpaid") {
                option->backgroundBrush = QBrush(QColor(255, 200, 200));
            }
            // 待发药高亮（橙色）
            else if (dispensingStatus == "pending" && paymentStatus == "paid") {
                option->backgroundBrush = QBrush(QColor(255, 220, 180));
            }
            // 已发药高亮（绿色）
            else if (dispensingStatus == "dispensed") {
                option->backgroundBrush = QBrush(QColor(200, 255, 200));
            }
        }
    }
};

Prescriptionview::Prescriptionview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Prescriptionview)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    populateStatusFilter();
    populateDoctorFilter();
    updateStats();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &Prescriptionview::on_tableView_doubleClicked);

    // 设置回车键搜索
    connect(ui->txtSearch, &QLineEdit::returnPressed,
            this, &Prescriptionview::on_txtSearch_returnPressed);
}

Prescriptionview::~Prescriptionview()
{
    delete ui;
}

void Prescriptionview::initTableView()
{
    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);

    // 设置自定义委托
    ui->tableView->setItemDelegate(new PrescriptionHighlightDelegate(this));

    // 初始化模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initPrescriptionModel()) {
        ui->tableView->setModel(iDatabase.prescriptionTabModel);
        ui->tableView->setSelectionModel(iDatabase.thePrescriptionSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.prescriptionTabModel->fieldIndex("created_time"));
        ui->tableView->hideColumn(iDatabase.prescriptionTabModel->fieldIndex("consult_record_id"));
        ui->tableView->hideColumn(iDatabase.prescriptionTabModel->fieldIndex("notes"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("prescription_number"), 120);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("patient_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("doctor_id"), 100);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("prescription_date"), 120);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("diagnosis"), 150);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("total_amount"), 100);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("payment_status"), 80);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("dispensing_status"), 80);
        ui->tableView->setColumnWidth(iDatabase.prescriptionTabModel->fieldIndex("status"), 80);
    } else {
        QMessageBox::warning(this, "错误", "初始化处方数据失败");
    }
}

void Prescriptionview::populateStatusFilter()
{
    ui->cmbStatus->clear();
    ui->cmbStatus->addItem("所有状态", "");
    ui->cmbStatus->addItem("待收费", "unpaid");
    ui->cmbStatus->addItem("待发药", "pending_paid");
    ui->cmbStatus->addItem("已发药", "dispensed");
    ui->cmbStatus->addItem("已审核", "audited");
}

void Prescriptionview::populateDoctorFilter()
{
    ui->cmbDoctor->clear();
    ui->cmbDoctor->addItem("所有医生", "");

    QList<QString> doctors = IDatabase::getInstance().getDoctorsForCombo();
    for (const QString &doctor : doctors) {
        ui->cmbDoctor->addItem(doctor);
    }
}

void Prescriptionview::updateStats()
{
    IDatabase &iDatabase = IDatabase::getInstance();

    // 获取今日统计
    QMap<QString, QVariant> todayStats = iDatabase.getTodayPrescriptionStats();
    int todayCount = todayStats["today_count"].toInt();
    double todayAmount = todayStats["today_amount"].toDouble();

    // 获取本月统计
    QMap<QString, QVariant> monthStats = iDatabase.getMonthPrescriptionStats();
    double monthAmount = monthStats["month_amount"].toDouble();

    // 获取待发药数量
    int pendingDispense = iDatabase.getPendingDispenseCount();

    // 获取待收费数量
    int unpaidCount = iDatabase.getUnpaidCount();

    // 更新界面
    ui->lblTodayPrescription->setText(QString("今日处方：%1张").arg(todayCount));
    ui->lblTodayAmount->setText(QString("今日金额：%1元").arg(todayAmount, 0, 'f', 2));
    ui->lblPendingDispense->setText(QString("待发药：%1张").arg(pendingDispense));
    ui->lblUnpaid->setText(QString("待收费：%1张").arg(unpaidCount));
    ui->lblMonthAmount->setText(QString("本月金额：%1元").arg(monthAmount, 0, 'f', 2));
}

void Prescriptionview::refreshData()
{
    IDatabase::getInstance().prescriptionTabModel->select();
    updateStats();
    highlightStatus();
}

void Prescriptionview::highlightStatus()
{
    // 表格委托会自动高亮，这里主要是为了更新统计信息
    updateStats();
}

void Prescriptionview::on_btnSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (IDatabase::getInstance().searchPrescription(searchText)) {
        updateStats();
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索处方信息失败");
    }
}

void Prescriptionview::on_btnAdd_clicked()
{
    // 新增处方
    int currow = IDatabase::getInstance().addNewPrescription();
    qDebug() << "新增处方，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goPrescriptionEditView(currow);
}

void Prescriptionview::on_btnEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的处方");
        return;
    }

    QString dispensingStatus = IDatabase::getInstance().prescriptionTabModel
                                   ->record(currentIndex.row()).value("dispensing_status").toString();

    // 已发药的处方不能编辑
    if (dispensingStatus == "dispensed") {
        QMessageBox::warning(this, "错误", "已发药的处方不能编辑");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goPrescriptionEditView(currentIndex.row());
}

void Prescriptionview::on_btnView_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要查看的处方");
        return;
    }

    // 查看详情，使用编辑界面但只读模式
    emit goPrescriptionEditView(currentIndex.row());
}

void Prescriptionview::on_btnDispense_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要发药的处方");
        return;
    }

    QString prescriptionId = IDatabase::getInstance().prescriptionTabModel
                                 ->record(currentIndex.row()).value("id").toString();
    QString prescriptionNumber = IDatabase::getInstance().prescriptionTabModel
                                     ->record(currentIndex.row()).value("prescription_number").toString();
    QString paymentStatus = IDatabase::getInstance().prescriptionTabModel
                                ->record(currentIndex.row()).value("payment_status").toString();
    QString dispensingStatus = IDatabase::getInstance().prescriptionTabModel
                                   ->record(currentIndex.row()).value("dispensing_status").toString();

    // 检查支付状态
    if (paymentStatus != "paid") {
        QMessageBox::warning(this, "错误", "处方未收费，不能发药");
        return;
    }

    // 检查发药状态
    if (dispensingStatus == "dispensed") {
        QMessageBox::warning(this, "提示", "处方已发药");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认发药",
                                  QString("确定要为处方 '%1' 发药吗？").arg(prescriptionNumber),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().dispensePrescription(prescriptionId)) {
            refreshData();
            QMessageBox::information(this, "成功", "处方发药成功");
        } else {
            QMessageBox::warning(this, "错误", "发药失败，请检查药品库存");
        }
    }
}

void Prescriptionview::on_btnPayment_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要收费的处方");
        return;
    }

    QString prescriptionId = IDatabase::getInstance().prescriptionTabModel
                                 ->record(currentIndex.row()).value("id").toString();
    QString prescriptionNumber = IDatabase::getInstance().prescriptionTabModel
                                     ->record(currentIndex.row()).value("prescription_number").toString();
    double totalAmount = IDatabase::getInstance().prescriptionTabModel
                             ->record(currentIndex.row()).value("total_amount").toDouble();
    QString paymentStatus = IDatabase::getInstance().prescriptionTabModel
                                ->record(currentIndex.row()).value("payment_status").toString();

    // 检查支付状态
    if (paymentStatus == "paid") {
        QMessageBox::warning(this, "提示", "处方已收费");
        return;
    }

    if (totalAmount <= 0) {
        QMessageBox::warning(this, "错误", "处方金额为0，无需收费");
        return;
    }

    // 简化的收费对话框，实际项目中应该有更完整的支付界面
    QStringList paymentMethods = {"现金", "银行卡", "医保", "支付宝", "微信"};
    bool ok;
    QString paymentMethod = QInputDialog::getItem(this, "选择支付方式",
                                                  QString("处方号：%1\n金额：%2元\n请选择支付方式：")
                                                      .arg(prescriptionNumber).arg(totalAmount, 0, 'f', 2),
                                                  paymentMethods, 0, false, &ok);

    if (ok && !paymentMethod.isEmpty()) {
        QMessageBox::StandardButton confirm;
        confirm = QMessageBox::question(this, "确认收费",
                                        QString("确定要收取处方 '%1' 的费用吗？\n金额：%2元\n支付方式：%3")
                                            .arg(prescriptionNumber).arg(totalAmount, 0, 'f', 2).arg(paymentMethod),
                                        QMessageBox::Yes | QMessageBox::No);

        if (confirm == QMessageBox::Yes) {
            if (IDatabase::getInstance().processPayment(prescriptionId, paymentMethod)) {
                refreshData();
                QMessageBox::information(this, "成功", QString("收费成功，金额：%1元").arg(totalAmount, 0, 'f', 2));
            } else {
                QMessageBox::warning(this, "错误", "收费失败");
            }
        }
    }
}

void Prescriptionview::on_btnAudit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要审核的处方");
        return;
    }

    QString prescriptionId = IDatabase::getInstance().prescriptionTabModel
                                 ->record(currentIndex.row()).value("id").toString();
    QString prescriptionNumber = IDatabase::getInstance().prescriptionTabModel
                                     ->record(currentIndex.row()).value("prescription_number").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认审核",
                                  QString("确定要审核处方 '%1' 吗？").arg(prescriptionNumber),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().auditPrescription(prescriptionId)) {
            refreshData();
            QMessageBox::information(this, "成功", "处方审核成功");
        } else {
            QMessageBox::warning(this, "错误", "审核失败");
        }
    }
}

void Prescriptionview::on_dateFilter_dateChanged(const QDate &date)
{
    QString dateStr = date.toString("yyyy-MM-dd");
    IDatabase::getInstance().prescriptionTabModel->setFilter(
        QString("DATE(prescription_date) = '%1'").arg(dateStr));
    IDatabase::getInstance().prescriptionTabModel->select();
    updateStats();
}

void Prescriptionview::on_cmbStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString statusFilter = ui->cmbStatus->currentData().toString();

    if (!statusFilter.isEmpty()) {
        if (statusFilter == "pending_paid") {
            IDatabase::getInstance().prescriptionTabModel->setFilter(
                "dispensing_status = 'pending' AND payment_status = 'paid'");
        } else if (statusFilter == "unpaid") {
            IDatabase::getInstance().prescriptionTabModel->setFilter(
                "payment_status = 'unpaid'");
        } else if (statusFilter == "dispensed") {
            IDatabase::getInstance().prescriptionTabModel->setFilter(
                "dispensing_status = 'dispensed'");
        } else if (statusFilter == "audited") {
            IDatabase::getInstance().prescriptionTabModel->setFilter(
                "status = 'audited'");
        }
        IDatabase::getInstance().prescriptionTabModel->select();
        updateStats();
    } else {
        IDatabase::getInstance().prescriptionTabModel->setFilter("");
        IDatabase::getInstance().prescriptionTabModel->select();
        updateStats();
    }
}

void Prescriptionview::on_cmbDoctor_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString doctorFilter = ui->cmbDoctor->currentData().toString();

    if (!doctorFilter.isEmpty()) {
        // 从"医生姓名 (医生ID)"格式中提取ID
        QRegularExpression rx("\\((.*)\\)");
        QRegularExpressionMatch match = rx.match(ui->cmbDoctor->currentText());
        if (match.hasMatch()) {
            QString doctorId = match.captured(1);
            IDatabase::getInstance().prescriptionTabModel->setFilter(
                QString("doctor_id = '%1'").arg(doctorId));
            IDatabase::getInstance().prescriptionTabModel->select();
            updateStats();
        }
    } else {
        IDatabase::getInstance().prescriptionTabModel->setFilter("");
        IDatabase::getInstance().prescriptionTabModel->select();
        updateStats();
    }
}

void Prescriptionview::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}

void Prescriptionview::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btnView_clicked();
}
