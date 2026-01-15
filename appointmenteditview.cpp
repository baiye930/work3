#include "appointmenteditview.h"
#include "ui_appointmenteditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QRandomGenerator>
#include <QSqlTableModel>

AppointmentEditView::AppointmentEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::AppointmentEditView)
    , dataMapper(nullptr)
    , isNewAppointment(false)
    , hasTimeConflict(false)
{
    ui->setupUi(this);
    initUI();
    populateComboBoxes();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().appointmentTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString appointmentNumber = rec.value("appointment_number").toString();
        if (appointmentNumber.isEmpty()) {
            isNewAppointment = true;
        } else {
            currentAppointmentId = rec.value("id").toString();
        }
    } else {
        isNewAppointment = false;
    }

    // 设置窗口标题
    if (isNewAppointment) {
        this->setWindowTitle("新建预约");
    } else {
        this->setWindowTitle("编辑预约");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射 - 只映射可以直接存储的字段
    dataMapper->addMapping(ui->txtAppointmentId, tabModel->fieldIndex("id"));
    dataMapper->addMapping(ui->txtPatientId, tabModel->fieldIndex("patient_id"));
    dataMapper->addMapping(ui->txtDoctorId, tabModel->fieldIndex("doctor_id"));
    dataMapper->addMapping(ui->dateAppointmentDate, tabModel->fieldIndex("appointment_date"));
    dataMapper->addMapping(ui->txtReason, tabModel->fieldIndex("reason"));
    dataMapper->addMapping(ui->txtSymptoms, tabModel->fieldIndex("symptoms"));
    dataMapper->addMapping(ui->txtNotes, tabModel->fieldIndex("notes"));
    // 下拉框手动处理

    // 设置当前索引
    if (isNewAppointment) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->dateAppointmentDate->setDate(QDate::currentDate());
        ui->timeAppointmentTime->setTime(QTime(9, 0));
        ui->cmbStatus->setCurrentText("scheduled");
        ui->cmbPriority->setCurrentIndex(0);
    } else {
        dataMapper->setCurrentIndex(index);
        // 加载已有的预约数据
        loadAppointmentData();
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &AppointmentEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &AppointmentEditView::on_btnCancel_clicked);
    connect(ui->txtPatientId, &QLineEdit::editingFinished, this, &AppointmentEditView::on_txtPatientId_editingFinished);
    connect(ui->txtDoctorId, &QLineEdit::editingFinished, this, &AppointmentEditView::on_txtDoctorId_editingFinished);
    connect(ui->btnSearchPatient, &QPushButton::clicked, this, &AppointmentEditView::on_btnSearchPatient_clicked);
    connect(ui->btnSearchDoctor, &QPushButton::clicked, this, &AppointmentEditView::on_btnSearchDoctor_clicked);
    connect(ui->dateAppointmentDate, &QDateEdit::dateChanged, this, &AppointmentEditView::on_dateAppointmentDate_dateChanged);
    connect(ui->timeAppointmentTime, &QTimeEdit::timeChanged, this, &AppointmentEditView::on_timeAppointmentTime_timeChanged);
}

AppointmentEditView::~AppointmentEditView()
{
    delete ui;
}

void AppointmentEditView::initUI()
{
    // 设置日期范围
    ui->dateAppointmentDate->setDate(QDate::currentDate());
    ui->dateAppointmentDate->setMinimumDate(QDate::currentDate());
    ui->dateAppointmentDate->setMaximumDate(QDate::currentDate().addYears(1));

    // 设置时间显示格式
    ui->timeAppointmentTime->setDisplayFormat("HH:mm");
    ui->timeAppointmentTime->setTime(QTime(9, 0));

    // 设置优先级下拉框
    ui->cmbPriority->clear();
    ui->cmbPriority->addItem("普通 (0)", "0");
    ui->cmbPriority->addItem("紧急 (1)", "1");

    // 隐藏错误标签
    clearError();
}

void AppointmentEditView::populateComboBoxes()
{
    // 状态
    ui->cmbStatus->clear();
    QStringList statuses = IDatabase::getInstance().getAppointmentStatuses();
    for (const QString &status : statuses) {
        ui->cmbStatus->addItem(status, status);
    }
}

void AppointmentEditView::loadAppointmentData()
{
    if (isNewAppointment || !dataMapper) return;

    QSqlTableModel *tabModel = IDatabase::getInstance().appointmentTabModel;
    int currentRow = dataMapper->currentIndex();

    if (currentRow >= 0 && currentRow < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(currentRow);

        // 显示预约ID
        QString appointmentId = rec.value("id").toString();
        ui->txtAppointmentId->setText(appointmentId);

        // 获取数据
        QString patientId = rec.value("patient_id").toString();
        QString doctorId = rec.value("doctor_id").toString();
        QString priority = rec.value("priority").toString();
        QString status = rec.value("status").toString();
        QString timeSlot = rec.value("time_slot").toString();

        // 设置患者ID
        if (!patientId.isEmpty()) {
            ui->txtPatientId->setText(patientId);
            // 查询患者信息
            searchPatientInfo(patientId);
        }

        // 设置医生ID
        if (!doctorId.isEmpty()) {
            ui->txtDoctorId->setText(doctorId);
            // 查询医生信息
            searchDoctorInfo(doctorId);
        }

        // 设置时间
        if (!timeSlot.isEmpty()) {
            QTime time = QTime::fromString(timeSlot, "HH:mm");
            if (time.isValid()) {
                ui->timeAppointmentTime->setTime(time);
            }
        }

        // 设置优先级
        int priorityIndex = ui->cmbPriority->findData(priority);
        if (priorityIndex >= 0) {
            ui->cmbPriority->setCurrentIndex(priorityIndex);
        }

        // 设置状态
        int statusIndex = ui->cmbStatus->findText(status);
        if (statusIndex >= 0) {
            ui->cmbStatus->setCurrentIndex(statusIndex);
        }

        // 检查时间冲突
        checkTimeConflict();
    }
}

void AppointmentEditView::searchPatientInfo(const QString &patientId)
{
    if (patientId.isEmpty()) {
        ui->txtPatientName->clear();
        ui->txtPatientName->setPlaceholderText("输入患者ID后显示");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT name FROM patient WHERE ID = ?");
    query.addBindValue(patientId);

    if (query.exec() && query.next()) {
        QString name = query.value("name").toString();
        showPatientInfo(name, true);
        // 存储映射
        patientIdMap[patientId] = name;
    } else {
        showPatientInfo("", false);
        ui->lblError->setText("未找到患者ID：" + patientId);
        ui->lblError->setVisible(true);
    }
}

void AppointmentEditView::searchDoctorInfo(const QString &doctorId)
{
    if (doctorId.isEmpty()) {
        ui->txtDoctorName->clear();
        ui->txtDoctorName->setPlaceholderText("输入医生ID后显示");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT name FROM doctor WHERE id = ?");
    query.addBindValue(doctorId);

    if (query.exec() && query.next()) {
        QString name = query.value("name").toString();
        showDoctorInfo(name, true);
        // 存储映射
        doctorIdMap[doctorId] = name;
    } else {
        showDoctorInfo("", false);
        ui->lblError->setText("未找到医生ID：" + doctorId);
        ui->lblError->setVisible(true);
    }
}

void AppointmentEditView::showPatientInfo(const QString &name, bool found)
{
    if (found && !name.isEmpty()) {
        ui->txtPatientName->setText(name);
        ui->txtPatientName->setStyleSheet("");
    } else {
        ui->txtPatientName->clear();
        ui->txtPatientName->setPlaceholderText("患者ID无效");
        ui->txtPatientName->setStyleSheet("color: red;");
    }
}

void AppointmentEditView::showDoctorInfo(const QString &name, bool found)
{
    if (found && !name.isEmpty()) {
        ui->txtDoctorName->setText(name);
        ui->txtDoctorName->setStyleSheet("");
    } else {
        ui->txtDoctorName->clear();
        ui->txtDoctorName->setPlaceholderText("医生ID无效");
        ui->txtDoctorName->setStyleSheet("color: red;");
    }
}

bool AppointmentEditView::validateInput()
{
    clearError();

    // 验证医生ID
    QString doctorId = ui->txtDoctorId->text().trimmed();
    if (doctorId.isEmpty()) {
        showError("医生ID不能为空");
        ui->txtDoctorId->setFocus();
        return false;
    }

    // 验证医生是否存在
    if (!doctorIdMap.contains(doctorId) || doctorIdMap[doctorId].isEmpty()) {
        showError("医生ID无效，请检查或重新查询");
        ui->txtDoctorId->setFocus();
        return false;
    }

    // 验证预约事由
    if (ui->txtReason->text().trimmed().isEmpty()) {
        showError("预约事由不能为空");
        ui->txtReason->setFocus();
        return false;
    }

    // 检查时间冲突
    if (hasTimeConflict) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, "时间冲突",
                                     "该医生在此时间段已有预约，是否继续保存？",
                                     QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return false;
        }
    }

    return true;
}

void AppointmentEditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void AppointmentEditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}

void AppointmentEditView::checkTimeConflict()
{
    QString doctorId = ui->txtDoctorId->text().trimmed();
    if (doctorId.isEmpty()) {
        hasTimeConflict = false;
        updateTimeConflictWarning();
        return;
    }

    QDateTime appointmentTime(ui->dateAppointmentDate->date(), ui->timeAppointmentTime->time());

    hasTimeConflict = IDatabase::getInstance().checkTimeConflict(
        doctorId, appointmentTime, currentAppointmentId);

    updateTimeConflictWarning();
}

void AppointmentEditView::updateTimeConflictWarning()
{
    if (hasTimeConflict) {
        ui->lblTimeConflict->setText("⚠️ 该时间段可能已有预约，建议选择其他时间");
        ui->lblTimeConflict->setStyleSheet("color: orange; font-weight: bold;");
    } else {
        ui->lblTimeConflict->clear();
    }
}

void AppointmentEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().appointmentTabModel;
    int currentRow = dataMapper->currentIndex();
    QSqlRecord rec = tabModel->record(currentRow);

    // 1. 设置患者ID（可以为空）
    QString patientId = ui->txtPatientId->text().trimmed();
    if (patientId.isEmpty()) {
        rec.setValue("patient_id", QVariant()); // 使用QVariant()表示NULL
    } else {
        rec.setValue("patient_id", patientId);
    }

    // 2. 设置医生ID
    QString doctorId = ui->txtDoctorId->text().trimmed();
    rec.setValue("doctor_id", doctorId);

    // 3. 设置优先级
    QString priority = ui->cmbPriority->currentData().toString();
    rec.setValue("priority", priority);

    // 4. 设置状态
    QString status = ui->cmbStatus->currentText();
    rec.setValue("status", status);

    // 5. 设置时间字符串
    QString timeStr = ui->timeAppointmentTime->time().toString("HH:mm");
    rec.setValue("time_slot", timeStr);

    // 6. 更新时间戳
    rec.setValue("updated_time", QDateTime::currentDateTime());

    // 7. 如果是新增，生成ID和预约号
    if (isNewAppointment) {
        QString appointmentId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        rec.setValue("id", appointmentId);

        // 生成预约号
        QString dateStr = QDate::currentDate().toString("yyyyMMdd");
        QString randomStr = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
        QString appointmentNumber = "APT" + dateStr + randomStr;
        rec.setValue("appointment_number", appointmentNumber);

        // 设置创建时间
        rec.setValue("created_time", QDateTime::currentDateTime());
    }

    // 8. 更新记录
    tabModel->setRecord(currentRow, rec);

    // 9. 提交更改
    if (IDatabase::getInstance().submitAppointmentEdit()) {
        QMessageBox::information(this, "成功",
                                 isNewAppointment ? "预约创建成功" : "预约信息更新成功");
        emit goPreviousView();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请检查数据是否正确");
    }
}

void AppointmentEditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertAppointmentEdit();
    emit goPreviousView();
}

void AppointmentEditView::on_txtPatientId_editingFinished()
{
    QString patientId = ui->txtPatientId->text().trimmed();
    if (!patientId.isEmpty()) {
        searchPatientInfo(patientId);
    } else {
        ui->txtPatientName->clear();
        ui->txtPatientName->setPlaceholderText("输入患者ID后显示");
    }
}

void AppointmentEditView::on_txtDoctorId_editingFinished()
{
    QString doctorId = ui->txtDoctorId->text().trimmed();
    if (!doctorId.isEmpty()) {
        searchDoctorInfo(doctorId);
        // 检查时间冲突
        checkTimeConflict();
    } else {
        ui->txtDoctorName->clear();
        ui->txtDoctorName->setPlaceholderText("输入医生ID后显示");
        hasTimeConflict = false;
        updateTimeConflictWarning();
    }
}

void AppointmentEditView::on_btnSearchPatient_clicked()
{
    on_txtPatientId_editingFinished();
}

void AppointmentEditView::on_btnSearchDoctor_clicked()
{
    on_txtDoctorId_editingFinished();
}

void AppointmentEditView::on_dateAppointmentDate_dateChanged(const QDate &date)
{
    Q_UNUSED(date);
    checkTimeConflict();
}

void AppointmentEditView::on_timeAppointmentTime_timeChanged(const QTime &time)
{
    Q_UNUSED(time);
    checkTimeConflict();
}
