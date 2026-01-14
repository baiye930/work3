#include "departmenteditview.h"
#include "ui_departmenteditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QDebug>

// 如果ui文件不存在，需要创建
#ifndef DEPARTMENTEDITVIEW_H
#include "ui_departmenteditview.h"
#endif

DepartmentEditView::DepartmentEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::DepartmentEditView)
    , dataMapper(nullptr)
    , isNewDepartment(false)
{
    ui->setupUi(this);
    initUI();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().departmentTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString deptName = rec.value("name").toString();
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
    } else {
        dataMapper->setCurrentIndex(index);

        // 获取当前记录的状态
        QSqlRecord rec = tabModel->record(index);
        QString status = rec.value("status").toString();

        if (!status.isEmpty()) {
            int statusIndex = ui->cmbStatus->findText(status);
            if (statusIndex >= 0) ui->cmbStatus->setCurrentIndex(statusIndex);
        }
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

    // 隐藏错误标签
    clearError();
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
