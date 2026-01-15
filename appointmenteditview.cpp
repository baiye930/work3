#include "appointmenteditview.h"
#include "ui_appointmenteditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QInputDialog>

AppointmentEditView::AppointmentEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::AppointmentEditView)
    , dataMapper(nullptr)
    , isNewAppointment(false)
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

    // 添加映射
    dataMapper->addMapping(ui->txtPatientId, tabModel->fieldIndex("patient_id"));
    dataMapper->addMapping(ui->txtDoctorId, tabModel->fieldIndex("doctor_id"));
    dataMapper->addMapping(ui->dateAppointmentDate, tabModel->fieldIndex("appointment_date"));
    dataMapper->addMapping(ui->timeAppointmentTime, tabModel->fieldIndex("appointment_time"));
    dataMapper->addMapping(ui->txtReason, tabModel->fieldIndex("reason"));
    dataMapper->addMapping(ui->txtNotes, tabModel->fieldIndex("notes"));
    dataMapper->addMapping(ui->cmbStatus, tabModel->fieldIndex("status"));

    // 设置当前索引
    if (isNewAppointment) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->dateAppointmentDate->setDate(QDate::currentDate());
        ui->timeAppointmentTime->setTime(QTime(9, 0)); // 默认上午9点
        ui->cmbStatus->setCurrentText("scheduled");
    } else {
        dataMapper->setCurrentIndex(index);
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &AppointmentEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &AppointmentEditView::on_btnCancel_clicked);
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

    // 设置时间范围（工作时间为8:00-18:00）
    ui->timeAppointmentTime->setTime(QTime(9, 0));

    // 隐藏错误标签
    clearError();
}

void AppointmentEditView::populateComboBoxes()
{
    // 状态
    ui->cmbStatus->clear();
    QStringList statuses = IDatabase::getInstance().getAppointmentStatuses();
    for (const QString &status : statuses) {
        ui->cmbStatus->addItem(status);
    }

    // 加载医生列表
    QList<QString> doctors = IDatabase::getInstance().getDoctorsForAppointmentCombo();
    ui->cmbDoctor->clear();
    ui->cmbDoctor->addItem("请选择医生", "");
    for (const QString &doctor : doctors) {
        ui->cmbDoctor->addItem(doctor);
    }
}

bool AppointmentEditView::validateInput()
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

    // 验证预约事由
    if (ui->txtReason->text().trimmed().isEmpty()) {
        showError("预约事由不能为空");
        return false;
    }

    // 检查时间冲突
    checkTimeConflict();

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

void AppointmentEditView::loadDoctorSchedule()
{
    // 加载医生的排班信息（简化实现）
    QString doctorText = ui->cmbDoctor->currentText();
    if (doctorText != "请选择医生" && !doctorText.isEmpty()) {
        // 提取医生ID
        QRegularExpression rx("\\((.*)\\)");
        QRegularExpressionMatch match = rx.match(doctorText);
        if (match.hasMatch()) {
            QString doctorId = match.captured(1);
            ui->txtDoctorId->setText(doctorId);

            // 可以在这里加载医生的具体排班信息
        }
    }
}

void AppointmentEditView::checkTimeConflict()
{
    QString doctorId = ui->txtDoctorId->text().trimmed();
    QDateTime appointmentTime(ui->dateAppointmentDate->date(), ui->timeAppointmentTime->time());

    if (!doctorId.isEmpty()) {
        bool hasConflict = IDatabase::getInstance().checkTimeConflict(
            doctorId, appointmentTime, currentAppointmentId);

        if (hasConflict) {
            ui->lblTimeConflict->setText("警告：该时间段可能已有预约");
            ui->lblTimeConflict->setStyleSheet("color: orange;");
        } else {
            ui->lblTimeConflict->clear();
        }
    }
}

void AppointmentEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().appointmentTabModel;
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());

    // 更新记录
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 提交更改
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

void AppointmentEditView::on_btnSelectPatient_clicked()
{
    // 简化实现：弹出输入框让用户输入患者ID
    bool ok;
    QString patientId = QInputDialog::getText(this, "选择患者",
                                              "请输入患者ID：", QLineEdit::Normal, "", &ok);

    if (ok && !patientId.isEmpty()) {
        ui->txtPatientId->setText(patientId);
    }
}

void AppointmentEditView::on_btnSelectDoctor_clicked()
{
    // 简化实现：使用下拉框选择
    // 这里已经通过cmbDoctor实现了
}

void AppointmentEditView::on_dateAppointmentDate_dateChanged(const QDate &date)
{
    Q_UNUSED(date);
    checkTimeConflict();
}

void AppointmentEditView::on_cmbDoctor_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    loadDoctorSchedule();
    checkTimeConflict();
}
