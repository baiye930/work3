#include "doctoreditview.h"
#include "ui_doctoreditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QDebug>
#include <QCompleter>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

DoctoreditView::DoctoreditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::DoctoreditView)
    , dataMapper(nullptr)
    , isNewDoctor(false)
{
    ui->setupUi(this);
    initUI();
    populateComboBoxes();

    // 判断是否是新增（如果index指向的是最后一行，则是新增）
    QSqlTableModel *tabModel = IDatabase::getInstance().doctorTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        // 检查该行是否是新添加的行（检查工号是否为空）
        QSqlRecord rec = tabModel->record(index);
        QString employeeId = rec.value("employee_id").toString();
        if (employeeId.isEmpty()) {
            isNewDoctor = true;
        }
    } else {
        // 如果索引无效，默认不是新增
        isNewDoctor = false;
    }

    // 设置窗口标题
    if (isNewDoctor) {
        this->setWindowTitle("新增医生");
    } else {
        this->setWindowTitle("编辑医生信息");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射
    dataMapper->addMapping(ui->txtEmployeeId, tabModel->fieldIndex("employee_id"));
    dataMapper->addMapping(ui->txtName, tabModel->fieldIndex("name"));
    dataMapper->addMapping(ui->cmbGender, tabModel->fieldIndex("gender"));
    dataMapper->addMapping(ui->dateBirth, tabModel->fieldIndex("birth_date"));
    dataMapper->addMapping(ui->txtSpecialization, tabModel->fieldIndex("specialization"));
    dataMapper->addMapping(ui->txtQualification, tabModel->fieldIndex("qualification"));
    dataMapper->addMapping(ui->txtLicense, tabModel->fieldIndex("license_number"));
    dataMapper->addMapping(ui->spinWorkYears, tabModel->fieldIndex("work_years"));
    dataMapper->addMapping(ui->txtPhone, tabModel->fieldIndex("phone"));
    dataMapper->addMapping(ui->txtEmail, tabModel->fieldIndex("email"));
    dataMapper->addMapping(ui->spinConsultationFee, tabModel->fieldIndex("consultation_fee"));
    dataMapper->addMapping(ui->txtSchedule, tabModel->fieldIndex("schedule"));
    dataMapper->addMapping(ui->cmbStatus, tabModel->fieldIndex("status"));

    // 设置当前索引
    if (isNewDoctor) {
        // 如果是新增，应该设置到最后一行
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        // 设置默认值
        ui->cmbTitle->setCurrentText("主治医师");
        ui->cmbStatus->setCurrentText("active");
        ui->cmbGender->setCurrentText("男");
        ui->dateBirth->setDate(QDate::currentDate().addYears(-30));
        originalDepartmentText = "";
    } else {
        // 如果是编辑，设置到指定行
        dataMapper->setCurrentIndex(index);

        // 加载科室数据
        loadDepartmentData();

        // 获取当前记录的职称和状态
        QSqlRecord rec = tabModel->record(index);
        QString title = rec.value("title").toString();
        QString status = rec.value("status").toString();
        QString gender = rec.value("gender").toString();

        if (!title.isEmpty()) {
            int titleIndex = ui->cmbTitle->findText(title);
            if (titleIndex >= 0) ui->cmbTitle->setCurrentIndex(titleIndex);
        }

        if (!status.isEmpty()) {
            int statusIndex = ui->cmbStatus->findText(status);
            if (statusIndex >= 0) ui->cmbStatus->setCurrentIndex(statusIndex);
        }

        if (!gender.isEmpty()) {
            int genderIndex = ui->cmbGender->findText(gender);
            if (genderIndex >= 0) ui->cmbGender->setCurrentIndex(genderIndex);
        }
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &DoctoreditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &DoctoreditView::on_btnCancel_clicked);
}

DoctoreditView::~DoctoreditView()
{
    delete ui;
}

void DoctoreditView::initUI()
{
    // 设置日期范围
    ui->dateBirth->setDate(QDate::currentDate().addYears(-30));
    ui->dateBirth->setMaximumDate(QDate::currentDate());
    ui->dateBirth->setMinimumDate(QDate::currentDate().addYears(-100));

    // 设置输入验证
    ui->txtPhone->setInputMask("000-0000-0000;_");

    // 使用 QRegularExpressionValidator 替代 QRegExpValidator
    QRegularExpression emailRegex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}\\b",
                                  QRegularExpression::CaseInsensitiveOption);
    ui->txtEmail->setValidator(new QRegularExpressionValidator(emailRegex, this));

    // 隐藏错误标签
    clearError();
}

void DoctoreditView::populateComboBoxes()
{
    // 性别
    ui->cmbGender->clear();
    ui->cmbGender->addItem("男");
    ui->cmbGender->addItem("女");

    // 职称
    ui->cmbTitle->clear();
    QStringList titles = IDatabase::getInstance().getDoctorTitles();
    for (const QString &title : titles) {
        ui->cmbTitle->addItem(title);
    }

    // 状态
    ui->cmbStatus->clear();
    QStringList statuses = IDatabase::getInstance().getDoctorStatuses();
    for (const QString &status : statuses) {
        ui->cmbStatus->addItem(status);
    }

    // 科室（异步加载）
    loadDepartmentData();
}

void DoctoreditView::loadDepartmentData()
{
    ui->cmbDepartment->clear();
    QList<QString> departments = IDatabase::getInstance().getDepartmentsForCombo();
    for (const QString &dept : departments) {
        ui->cmbDepartment->addItem(dept);
    }

    // 如果是编辑模式，设置当前科室
    if (!isNewDoctor && dataMapper) {
        QSqlTableModel *tabModel = IDatabase::getInstance().doctorTabModel;
        QSqlRecord rec = tabModel->record(dataMapper->currentIndex());
        QString departmentId = rec.value("department_id").toString();

        if (!departmentId.isEmpty()) {
            // 查找对应的科室显示文本
            for (int i = 0; i < ui->cmbDepartment->count(); i++) {
                QString itemText = ui->cmbDepartment->itemText(i);
                if (itemText.contains(departmentId)) {
                    ui->cmbDepartment->setCurrentIndex(i);
                    originalDepartmentText = itemText;
                    break;
                }
            }
        }
    }
}

bool DoctoreditView::validateInput()
{
    clearError();

    // 验证姓名
    if (ui->txtName->text().trimmed().isEmpty()) {
        showError("姓名不能为空");
        ui->txtName->setFocus();
        return false;
    }

    // 验证联系电话格式
    QString phone = ui->txtPhone->text();
    if (!phone.isEmpty() && phone.contains('_')) {
        showError("联系电话格式不正确");
        ui->txtPhone->setFocus();
        return false;
    }

    // 验证邮箱格式
    QString email = ui->txtEmail->text();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}\\b",
                                      QRegularExpression::CaseInsensitiveOption);
        if (!emailRegex.match(email).hasMatch()) {
            showError("邮箱格式不正确");
            ui->txtEmail->setFocus();
            return false;
        }
    }

    // 验证诊费
    if (ui->spinConsultationFee->value() < 0) {
        showError("诊费不能为负数");
        ui->spinConsultationFee->setFocus();
        return false;
    }

    return true;
}

void DoctoreditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void DoctoreditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}

void DoctoreditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 保存科室信息（从组合框中提取科室ID）
    QString departmentText = ui->cmbDepartment->currentText();
    QString departmentId = "";

    if (!departmentText.isEmpty() && departmentText != originalDepartmentText) {
        // 从格式 "科室名称 (dept001)" 中提取科室ID
        QRegularExpression rx("\\((.*)\\)");
        QRegularExpressionMatch match = rx.match(departmentText);
        if (match.hasMatch()) {
            departmentId = match.captured(1);
        }
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().doctorTabModel;
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());

    // 设置职称和科室
    rec.setValue("title", ui->cmbTitle->currentText());
    rec.setValue("department_id", departmentId);
    rec.setValue("gender", ui->cmbGender->currentText());
    rec.setValue("status", ui->cmbStatus->currentText());

    // 更新记录
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 提交更改
    if (IDatabase::getInstance().submitDoctorEdit()) {
        QMessageBox::information(this, "成功",
                                 isNewDoctor ? "医生添加成功" : "医生信息更新成功");
        emit goPreviousView();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请检查数据是否正确");
    }
}

void DoctoreditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertDoctorEdit();
    emit goPreviousView();
}
