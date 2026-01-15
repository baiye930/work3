#include "departmenteditview.h"
#include "ui_departmenteditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QDebug>

DepartmentEditView::DepartmentEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::DepartmentEditView)
    , dataMapper(nullptr)
    , isNewDepartment(false)
    , currentDepartmentId("")
{
    ui->setupUi(this);
    initUI();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().departmentTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString deptName = rec.value("name").toString();
        currentDepartmentId = rec.value("id").toString();  // 获取ID

        if (deptName.isEmpty()) {
            isNewDepartment = true;
        }
    } else {
        isNewDepartment = false;
    }

    // 设置窗口标题
    if (isNewDepartment) {
        this->setWindowTitle("新增科室");
    } else {
        this->setWindowTitle("编辑科室信息");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射
    dataMapper->addMapping(ui->txtID, tabModel->fieldIndex("id"));  // 映射ID字段
    dataMapper->addMapping(ui->txtName, tabModel->fieldIndex("name"));
    dataMapper->addMapping(ui->txtLocation, tabModel->fieldIndex("location"));
    dataMapper->addMapping(ui->txtPhone, tabModel->fieldIndex("phone"));
    dataMapper->addMapping(ui->txtDescription, tabModel->fieldIndex("description"));
    dataMapper->addMapping(ui->txtDirector, tabModel->fieldIndex("director_id"));
    dataMapper->addMapping(ui->dateEstablished, tabModel->fieldIndex("established_date"));
    dataMapper->addMapping(ui->spinBedCount, tabModel->fieldIndex("bed_count"));

    // 设置当前索引
    if (isNewDepartment) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->cmbStatus->setCurrentText("active");
        ui->dateEstablished->setDate(QDate::currentDate());

        // 如果是新增，显示生成的ID
        loadDepartmentData();
    } else {
        dataMapper->setCurrentIndex(index);

        // 获取当前记录的状态
        QSqlRecord rec = tabModel->record(index);
        QString status = rec.value("status").toString();

        if (!status.isEmpty()) {
            int statusIndex = ui->cmbStatus->findText(status);
            if (statusIndex >= 0) ui->cmbStatus->setCurrentIndex(statusIndex);
        }

        // 加载当前科室数据
        loadDepartmentData();
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &DepartmentEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &DepartmentEditView::on_btnCancel_clicked);
}

DepartmentEditView::~DepartmentEditView()
{
    delete ui;
}

void DepartmentEditView::initUI()
{
    // 设置日期范围
    ui->dateEstablished->setDate(QDate::currentDate());
    ui->dateEstablished->setMaximumDate(QDate::currentDate());
    ui->dateEstablished->setMinimumDate(QDate(1900, 1, 1));

    // 设置状态选项
    ui->cmbStatus->clear();
    QStringList statuses = IDatabase::getInstance().getDepartmentStatuses();
    for (const QString &status : statuses) {
        ui->cmbStatus->addItem(status);
    }

    // 设置床位数量范围
    ui->spinBedCount->setMinimum(0);
    ui->spinBedCount->setMaximum(1000);

    // 设置ID字段为只读
    ui->txtID->setReadOnly(true);

    // 隐藏错误标签
    clearError();
}

void DepartmentEditView::loadDepartmentData()
{
    QSqlTableModel *tabModel = IDatabase::getInstance().departmentTabModel;
    int currentRow = dataMapper->currentIndex();

    if (currentRow >= 0 && currentRow < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(currentRow);
        QString id = rec.value("id").toString();
        QString status = rec.value("status").toString();

        // 显示ID
        ui->txtID->setText(id);

        // 显示状态
        if (!status.isEmpty()) {
            int statusIndex = ui->cmbStatus->findText(status);
            if (statusIndex >= 0) ui->cmbStatus->setCurrentIndex(statusIndex);
        }
    }
}

bool DepartmentEditView::validateInput()
{
    clearError();

    // 验证科室名称
    if (ui->txtName->text().trimmed().isEmpty()) {
        showError("科室名称不能为空");
        ui->txtName->setFocus();
        return false;
    }

    // 验证联系电话格式
    QString phone = ui->txtPhone->text();
    if (!phone.isEmpty()) {
        // 简单的电话格式验证
        if (!phone.contains(QRegularExpression("^[0-9\\-]+$"))) {
            showError("联系电话只能包含数字和横线");
            ui->txtPhone->setFocus();
            return false;
        }
    }

    // 验证床位数量
    if (ui->spinBedCount->value() < 0) {
        showError("床位数量不能为负数");
        ui->spinBedCount->setFocus();
        return false;
    }

    return true;
}

void DepartmentEditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void DepartmentEditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}

QString DepartmentEditView::getCurrentDepartmentId() const
{
    return currentDepartmentId;
}

void DepartmentEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 获取当前记录
    QSqlTableModel *tabModel = IDatabase::getInstance().departmentTabModel;
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());

    // 设置状态
    rec.setValue("status", ui->cmbStatus->currentText());

    // 如果ID为空，生成新的ID（针对新增情况）
    QString id = ui->txtID->text().trimmed();
    if (id.isEmpty() && isNewDepartment) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        rec.setValue("id", id);
    }

    // 更新记录
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 提交更改
    if (IDatabase::getInstance().submitDepartmentEdit()) {
        QMessageBox::information(this, "成功",
                                 isNewDepartment ? "科室添加成功" : "科室信息更新成功");
        emit goPreviousView();
    } else {
        QMessageBox::warning(this, "错误", "保存失败，请检查数据是否正确");
    }
}

void DepartmentEditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertDepartmentEdit();
    emit goPreviousView();
}
