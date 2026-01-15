#include "prescriptioneditview.h"
#include "ui_prescriptioneditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QDebug>
#include <QSqlQuery>
#include "medicineselectdialog.h"
PrescriptionEditView::PrescriptionEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::PrescriptionEditView)
    , dataMapper(nullptr)
    , isNewPrescription(false)
    , currentPrescriptionId("")
{
    ui->setupUi(this);
    initUI();
    populateComboBoxes();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().prescriptionTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString prescriptionNumber = rec.value("prescription_number").toString();
        if (prescriptionNumber.isEmpty()) {
            isNewPrescription = true;
        } else {
            currentPrescriptionId = rec.value("id").toString();
        }
    } else {
        isNewPrescription = false;
    }

    // 设置窗口标题
    if (isNewPrescription) {
        this->setWindowTitle("开具处方");
    } else {
        this->setWindowTitle("编辑处方");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射
    dataMapper->addMapping(ui->txtPrescriptionNumber, tabModel->fieldIndex("prescription_number"));
    dataMapper->addMapping(ui->txtPatientId, tabModel->fieldIndex("patient_id"));
    dataMapper->addMapping(ui->txtDoctorId, tabModel->fieldIndex("doctor_id"));
    dataMapper->addMapping(ui->txtDiagnosis, tabModel->fieldIndex("diagnosis"));
    dataMapper->addMapping(ui->txtNotes, tabModel->fieldIndex("notes"));
    dataMapper->addMapping(ui->datePrescriptionDate, tabModel->fieldIndex("prescription_date"));

    // 设置当前索引
    if (isNewPrescription) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->datePrescriptionDate->setDateTime(QDateTime::currentDateTime());
        ui->cmbPaymentStatus->setCurrentText("unpaid");
        ui->cmbDispensingStatus->setCurrentText("pending");
    } else {
        dataMapper->setCurrentIndex(index);

        // 加载处方详情
        loadPrescriptionDetails();

        // 获取当前记录的状态
        QSqlRecord rec = tabModel->record(index);
        QString paymentStatus = rec.value("payment_status").toString();
        QString dispensingStatus = rec.value("dispensing_status").toString();

        if (!paymentStatus.isEmpty()) {
            int statusIndex = ui->cmbPaymentStatus->findText(paymentStatus);
            if (statusIndex >= 0) ui->cmbPaymentStatus->setCurrentIndex(statusIndex);
        }

        if (!dispensingStatus.isEmpty()) {
            int statusIndex = ui->cmbDispensingStatus->findText(dispensingStatus);
            if (statusIndex >= 0) ui->cmbDispensingStatus->setCurrentIndex(statusIndex);
        }
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &PrescriptionEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &PrescriptionEditView::on_btnCancel_clicked);
    connect(ui->btnAddMedicine, &QPushButton::clicked, this, &PrescriptionEditView::on_btnAddMedicine_clicked);
    connect(ui->btnDeleteMedicine, &QPushButton::clicked, this, &PrescriptionEditView::on_btnDeleteMedicine_clicked);
    connect(ui->btnSelectPatient, &QPushButton::clicked, this, &PrescriptionEditView::on_btnSelectPatient_clicked);
    connect(ui->btnSelectDoctor, &QPushButton::clicked, this, &PrescriptionEditView::on_btnSelectDoctor_clicked);
}

PrescriptionEditView::~PrescriptionEditView()
{
    delete ui;
}

void PrescriptionEditView::initUI()
{
    // 设置日期时间
    ui->datePrescriptionDate->setDateTime(QDateTime::currentDateTime());

    // 设置表格属性
    ui->tableViewDetails->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewDetails->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewDetails->setAlternatingRowColors(true);

    // 隐藏错误标签
    clearError();
}

void PrescriptionEditView::populateComboBoxes()
{
    // 支付状态
    ui->cmbPaymentStatus->clear();
    ui->cmbPaymentStatus->addItem("unpaid");
    ui->cmbPaymentStatus->addItem("paid");
    ui->cmbPaymentStatus->addItem("refunded");

    // 发药状态
    ui->cmbDispensingStatus->clear();
    ui->cmbDispensingStatus->addItem("pending");
    ui->cmbDispensingStatus->addItem("dispensed");
    ui->cmbDispensingStatus->addItem("partial");
}

void PrescriptionEditView::loadPrescriptionDetails()
{
    if (!currentPrescriptionId.isEmpty()) {
        if (IDatabase::getInstance().initPrescriptionDetailModel(currentPrescriptionId)) {
            ui->tableViewDetails->setModel(IDatabase::getInstance().prescriptionDetailTabModel);
            ui->tableViewDetails->setSelectionModel(IDatabase::getInstance().thePrescriptionDetailSelection);

            // 设置列宽
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("medicine_id"), 150);
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("quantity"), 80);
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("dosage"), 120);
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("unit_price"), 80);
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("total_price"), 80);
            ui->tableViewDetails->setColumnWidth(IDatabase::getInstance().prescriptionDetailTabModel->fieldIndex("dispensing_quantity"), 80);

            updateTotalAmount();
        }
    }
}

bool PrescriptionEditView::validateInput()
{
    clearError();

    // 验证患者ID
    if (ui->txtPatientId->text().trimmed().isEmpty()) {
        showError("患者信息不能为空");
        ui->txtPatientId->setFocus();
        return false;
    }

    // 验证医生ID
    if (ui->txtDoctorId->text().trimmed().isEmpty()) {
        showError("医生信息不能为空");
        ui->txtDoctorId->setFocus();
        return false;
    }

    // 验证诊断
    if (ui->txtDiagnosis->text().trimmed().isEmpty()) {
        showError("诊断信息不能为空");
        ui->txtDiagnosis->setFocus();
        return false;
    }

    return true;
}

void PrescriptionEditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void PrescriptionEditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}



void PrescriptionEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().prescriptionTabModel;
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());

    // 设置状态
    rec.setValue("payment_status", ui->cmbPaymentStatus->currentText());
    rec.setValue("dispensing_status", ui->cmbDispensingStatus->currentText());

    // 更新记录
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 提交更改
    if (IDatabase::getInstance().submitPrescriptionEdit()) {
        QMessageBox::information(this, "成功",
                                 isNewPrescription ? "处方开具成功" : "处方信息更新成功");
        emit goPreviousView();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请检查数据是否正确");
    }
}

void PrescriptionEditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertPrescriptionEdit();
    emit goPreviousView();
}



void PrescriptionEditView::on_btnDeleteMedicine_clicked()
{
    if (currentPrescriptionId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先保存处方");
        return;
    }

    QModelIndex currentIndex = IDatabase::getInstance().thePrescriptionDetailSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的药品");
        return;
    }

    QString detailId = IDatabase::getInstance().prescriptionDetailTabModel
                           ->record(currentIndex.row()).value("id").toString();
    QString medicineName = IDatabase::getInstance().prescriptionDetailTabModel
                               ->record(currentIndex.row()).value("medicine_id").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除药品 '%1' 吗？").arg(medicineName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deletePrescriptionDetail(detailId)) {
            loadPrescriptionDetails(); // 重新加载
            QMessageBox::information(this, "成功", "药品删除成功");
        } else {
            QMessageBox::warning(this, "错误", "删除药品失败");
        }
    }
}

void PrescriptionEditView::on_btnSelectPatient_clicked()
{
    QMessageBox::information(this, "选择患者", "患者选择功能待实现");
}

void PrescriptionEditView::on_btnSelectDoctor_clicked()
{
    QMessageBox::information(this, "选择医生", "医生选择功能待实现");
}

void PrescriptionEditView::on_btnAddMedicine_clicked()
{
    if (currentPrescriptionId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先保存处方基本信息");
        return;
    }

    // 创建药品选择对话框
    MedicineSelectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString medicineId = dialog.getSelectedMedicineId();
        QString medicineName = dialog.getSelectedMedicineName();
        int quantity = dialog.getQuantity();
        QString dosage = dialog.getDosage();

        if (medicineId.isEmpty()) {
            QMessageBox::warning(this, "错误", "未选择药品");
            return;
        }

        // 获取药品单价
        QSqlQuery query;
        query.prepare("SELECT price FROM medicine WHERE id = ?");
        query.addBindValue(medicineId);

        if (query.exec() && query.next()) {
            double unitPrice = query.value(0).toDouble();

            // 添加到处方详情
            if (IDatabase::getInstance().addPrescriptionDetail(
                    currentPrescriptionId, medicineId, quantity, dosage)) {
                // 刷新处方详情列表
                loadPrescriptionDetails();
                // 更新总金额显示
                updateTotalAmount();
                QMessageBox::information(this, "成功", "药品添加成功");
            } else {
                QMessageBox::warning(this, "错误", "添加药品失败");
            }
        } else {
            QMessageBox::warning(this, "错误", "获取药品信息失败");
        }
    }
}

// 新增：更新总金额函数
void PrescriptionEditView::updateTotalAmount()
{
    if (currentPrescriptionId.isEmpty()) {
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT total_amount FROM prescription WHERE id = ?");
    query.addBindValue(currentPrescriptionId);

    if (query.exec() && query.next()) {
        double totalAmount = query.value(0).toDouble();
        // 更新UI显示总金额
        ui->lblTotalAmount->setText(QString("¥%1").arg(totalAmount, 0, 'f', 2));
    }
}
