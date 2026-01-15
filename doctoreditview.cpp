#include "doctoreditview.h"
#include "ui_doctoreditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlTableModel>

DoctoreditView::DoctoreditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::DoctoreditView)
    , dataMapper(nullptr)
{
    ui->setupUi(this);

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    QSqlTableModel *tabModel = IDatabase::getInstance().doctorTabModel;
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射
    dataMapper->addMapping(ui->txtDoctorId, tabModel->fieldIndex("id"));  // 医生ID
    dataMapper->addMapping(ui->txtEmployeeId, tabModel->fieldIndex("employee_id"));
    dataMapper->addMapping(ui->txtName, tabModel->fieldIndex("name"));
    dataMapper->addMapping(ui->cmbGender, tabModel->fieldIndex("gender"));
    dataMapper->addMapping(ui->dateBirth, tabModel->fieldIndex("birth_date"));
    dataMapper->addMapping(ui->cmbDepartment, tabModel->fieldIndex("department_id"));
    dataMapper->addMapping(ui->cmbTitle, tabModel->fieldIndex("title"));
    dataMapper->addMapping(ui->txtSpecialization, tabModel->fieldIndex("specialization"));
    dataMapper->addMapping(ui->txtPhone, tabModel->fieldIndex("phone"));
    dataMapper->addMapping(ui->txtEmail, tabModel->fieldIndex("email"));
    dataMapper->addMapping(ui->spinWorkYears, tabModel->fieldIndex("work_years"));
    dataMapper->addMapping(ui->spinFee, tabModel->fieldIndex("consultation_fee"));
    dataMapper->addMapping(ui->cmbStatus, tabModel->fieldIndex("status"));

    dataMapper->setCurrentIndex(index);

    // 设置医生ID为只读
    ui->txtDoctorId->setReadOnly(true);

    // 初始化下拉框
    initComboBoxes();

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &DoctoreditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &DoctoreditView::on_btnCancel_clicked);
}

DoctoreditView::~DoctoreditView()
{
    delete ui;
}

void DoctoreditView::initComboBoxes()
{
    // 性别
    ui->cmbGender->clear();
    ui->cmbGender->addItem("男");
    ui->cmbGender->addItem("女");

    // 科室
    ui->cmbDepartment->clear();
    ui->cmbDepartment->addItem("请选择科室", "");
    QList<QString> departments = IDatabase::getInstance().getDepartmentsForCombo();
    for (const QString &dept : departments) {
        ui->cmbDepartment->addItem(dept);
    }

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
}

void DoctoreditView::on_btnSave_clicked()
{
    if (validateInput()) {
        IDatabase::getInstance().submitDoctorEdit();
        emit goPreviousView();
    }
}

void DoctoreditView::on_btnCancel_clicked()
{
    IDatabase::getInstance().revertDoctorEdit();
    emit goPreviousView();
}

bool DoctoreditView::validateInput()
{
    // 验证姓名
    if (ui->txtName->text().trimmed().isEmpty()) {
        ui->lblError->setText("错误：医生姓名不能为空");
        ui->lblError->setVisible(true);
        return false;
    }

    // 验证工号
    if (ui->txtEmployeeId->text().trimmed().isEmpty()) {
        ui->lblError->setText("错误：工号不能为空");
        ui->lblError->setVisible(true);
        return false;
    }

    // 验证联系电话
    QString phone = ui->txtPhone->text();
    if (!phone.isEmpty()) {
        // 简单的电话格式验证
        if (!phone.contains(QRegularExpression("^[0-9\\-]+$"))) {
            ui->lblError->setText("错误：联系电话只能包含数字和横线");
            ui->lblError->setVisible(true);
            return false;
        }
    }

    ui->lblError->clear();
    ui->lblError->setVisible(false);
    return true;
}
