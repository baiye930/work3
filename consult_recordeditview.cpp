#include "consult_recordeditview.h"
#include "ui_consult_recordeditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QInputDialog>

ConsultRecordEditView::ConsultRecordEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::ConsultRecordEditView)
    , dataMapper(nullptr)
    , isNewConsultRecord(false)
{
    ui->setupUi(this);
    initUI();
    populateComboBoxes();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().consultRecordTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString patientId = rec.value("patient_id").toString();
        if (patientId.isEmpty()) {
            isNewConsultRecord = true;
        } else {
            currentRecordId = rec.value("id").toString();
        }
    } else {
        isNewConsultRecord = false;
    }

    // 设置窗口标题
    if (isNewConsultRecord) {
        this->setWindowTitle("新建就诊记录");
    } else {
        this->setWindowTitle("编辑就诊记录");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射 - 将 consult_date 改为 visit_date
    dataMapper->addMapping(ui->txtPatientId, tabModel->fieldIndex("patient_id"));
    dataMapper->addMapping(ui->txtDoctorId, tabModel->fieldIndex("doctor_id"));
    dataMapper->addMapping(ui->dateConsultDate, tabModel->fieldIndex("visit_date"));  // 改为 visit_date
    dataMapper->addMapping(ui->txtDiagnosis, tabModel->fieldIndex("diagnosis"));
    dataMapper->addMapping(ui->txtSymptoms, tabModel->fieldIndex("symptoms"));
    dataMapper->addMapping(ui->txtTreatment, tabModel->fieldIndex("treatment"));
    dataMapper->addMapping(ui->txtConsultFee, tabModel->fieldIndex("consult_fee"));
    dataMapper->addMapping(ui->txtNotes, tabModel->fieldIndex("notes"));
    dataMapper->addMapping(ui->dateNextVisit, tabModel->fieldIndex("next_visit_date"));
    dataMapper->addMapping(ui->txtPrescriptionId, tabModel->fieldIndex("prescription_id"));

    // 设置当前索引
    if (isNewConsultRecord) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->dateConsultDate->setDateTime(QDateTime::currentDateTime());
        ui->spinConsultFee->setValue(0.0);
    } else {
        dataMapper->setCurrentIndex(index);

        // 获取当前记录的科室
        QSqlRecord rec = tabModel->record(index);
        QString departmentId = rec.value("department_id").toString();

        if (!departmentId.isEmpty()) {
            // 查找对应的科室显示文本
            for (int i = 0; i < ui->cmbDepartment->count(); i++) {
                QString itemText = ui->cmbDepartment->itemText(i);
                if (itemText.contains(departmentId)) {
                    ui->cmbDepartment->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &ConsultRecordEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ConsultRecordEditView::on_btnCancel_clicked);
    connect(ui->spinConsultFee, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ConsultRecordEditView::updateConsultFee);
}

ConsultRecordEditView::~ConsultRecordEditView()
{
    delete ui;
}

void ConsultRecordEditView::initUI()
{
    // 设置日期时间
    ui->dateConsultDate->setDateTime(QDateTime::currentDateTime());
    ui->dateNextVisit->setDate(QDate::currentDate().addDays(7));

    // 设置诊费范围
    ui->spinConsultFee->setMinimum(0.0);
    ui->spinConsultFee->setMaximum(10000.0);
    ui->spinConsultFee->setDecimals(2);

    // 隐藏错误标签
    clearError();
}

void ConsultRecordEditView::populateComboBoxes()
{
    // 加载科室列表
    ui->cmbDepartment->clear();
    ui->cmbDepartment->addItem("请选择科室", "");

    QList<QString> departments = IDatabase::getInstance().getDepartmentsForCombo();
    for (const QString &dept : departments) {
        ui->cmbDepartment->addItem(dept);
    }
}

bool ConsultRecordEditView::validateInput()
{
    clearError();

    // 验证患者ID
    if (ui->txtPatientId->text().trimmed().isEmpty()) {
        showError("患者信息不能为空");
        return false;
    }

    // 验证医生ID
    if (ui->txtDoctorId->text().trimmed().isEmpty()) {
        showError("医生信息不能为空");
        return false;
    }

    // 验证诊断信息
    if (ui->txtDiagnosis->text().trimmed().isEmpty()) {
        showError("诊断信息不能为空");
        return false;
    }

    // 验证症状描述
    if (ui->txtSymptoms->toPlainText().trimmed().isEmpty()) {
        showError("症状描述不能为空");
        return false;
    }

    // 验证治疗方案
    if (ui->txtTreatment->toPlainText().trimmed().isEmpty()) {
        showError("治疗方案不能为空");
        return false;
    }

    return true;
}

void ConsultRecordEditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void ConsultRecordEditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}

void ConsultRecordEditView::updateConsultFee()
{
    double fee = ui->spinConsultFee->value();
    ui->txtConsultFee->setText(QString::number(fee, 'f', 2));
}

void ConsultRecordEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 保存科室信息
    QString departmentText = ui->cmbDepartment->currentText();
    QString departmentId = "";

    if (!departmentText.isEmpty() && departmentText != "请选择科室") {
        // 从格式 "科室名称 (dept001)" 中提取科室ID
        QRegularExpression rx("\\((.*)\\)");
        QRegularExpressionMatch match = rx.match(departmentText);
        if (match.hasMatch()) {
            departmentId = match.captured(1);
        }
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().consultRecordTabModel;
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());

    // 设置科室信息
    rec.setValue("department_id", departmentId);

    // 更新记录
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 提交更改
    if (IDatabase::getInstance().submitConsultRecordEdit()) {
        QMessageBox::information(this, "成功",
                                 isNewConsultRecord ? "就诊记录创建成功" : "就诊记录更新成功");
        emit goPreviousView();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请检查数据是否正确");
    }
}

void ConsultRecordEditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertConsultRecordEdit();
    emit goPreviousView();
}

void ConsultRecordEditView::on_btnSelectPatient_clicked()
{
    bool ok;
    QString patientId = QInputDialog::getText(this, "选择患者",
                                              "请输入患者ID：", QLineEdit::Normal, "", &ok);

    if (ok && !patientId.isEmpty()) {
        ui->txtPatientId->setText(patientId);
    }
}

void ConsultRecordEditView::on_btnSelectDoctor_clicked()
{
    bool ok;
    QString doctorId = QInputDialog::getText(this, "选择医生",
                                             "请输入医生ID：", QLineEdit::Normal, "", &ok);

    if (ok && !doctorId.isEmpty()) {
        ui->txtDoctorId->setText(doctorId);
    }
}

void ConsultRecordEditView::on_btnSelectPrescription_clicked()
{
    QMessageBox::information(this, "选择处方", "处方选择功能待实现");
}
