#include "medicineeditview.h"
#include "ui_medicineeditview.h"
#include "idatabase.h"
#include <QMessageBox>
#include <QSqlTableModel>
#include <QDebug>
#include <QRegularExpressionValidator>

MedicineEditView::MedicineEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::MedicineEditView)
    , dataMapper(nullptr)
    , isNewMedicine(false)
{
    ui->setupUi(this);
    initUI();
    populateComboBoxes();

    // 判断是否是新增
    QSqlTableModel *tabModel = IDatabase::getInstance().medicineTabModel;
    if (index >= 0 && index < tabModel->rowCount()) {
        QSqlRecord rec = tabModel->record(index);
        QString medicineCode = rec.value("code").toString();
        if (medicineCode.isEmpty()) {
            isNewMedicine = true;
        }
    } else {
        isNewMedicine = false;
    }

    // 设置窗口标题
    if (isNewMedicine) {
        this->setWindowTitle("新增药品");
    } else {
        this->setWindowTitle("编辑药品信息");
    }

    // 初始化数据映射器
    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 添加映射
    dataMapper->addMapping(ui->txtCode, tabModel->fieldIndex("code"));
    dataMapper->addMapping(ui->txtName, tabModel->fieldIndex("name"));
    dataMapper->addMapping(ui->txtGenericName, tabModel->fieldIndex("generic_name"));
    dataMapper->addMapping(ui->txtBrand, tabModel->fieldIndex("brand"));
    dataMapper->addMapping(ui->txtSpecification, tabModel->fieldIndex("specification"));
    dataMapper->addMapping(ui->txtUnit, tabModel->fieldIndex("unit"));
    dataMapper->addMapping(ui->txtManufacturer, tabModel->fieldIndex("manufacturer"));
    dataMapper->addMapping(ui->txtApprovalNumber, tabModel->fieldIndex("approval_number"));
    dataMapper->addMapping(ui->spinPrice, tabModel->fieldIndex("price"));
    dataMapper->addMapping(ui->spinCost, tabModel->fieldIndex("cost"));
    dataMapper->addMapping(ui->spinStock, tabModel->fieldIndex("stock"));
    dataMapper->addMapping(ui->spinMinStock, tabModel->fieldIndex("min_stock"));
    dataMapper->addMapping(ui->spinMaxStock, tabModel->fieldIndex("max_stock"));
    dataMapper->addMapping(ui->txtStorageLocation, tabModel->fieldIndex("storage_location"));
    dataMapper->addMapping(ui->txtIndications, tabModel->fieldIndex("indications"));
    dataMapper->addMapping(ui->txtContraindications, tabModel->fieldIndex("contraindications"));
    dataMapper->addMapping(ui->txtSideEffects, tabModel->fieldIndex("side_effects"));
    dataMapper->addMapping(ui->txtUsageDosage, tabModel->fieldIndex("usage_dosage"));
    dataMapper->addMapping(ui->txtBatchNumber, tabModel->fieldIndex("batch_number"));
    dataMapper->addMapping(ui->dateProduction, tabModel->fieldIndex("production_date"));
    dataMapper->addMapping(ui->dateExpiry, tabModel->fieldIndex("expiry_date"));

    // 设置当前索引
    if (isNewMedicine) {
        dataMapper->setCurrentIndex(tabModel->rowCount() - 1);
        ui->cmbCategory->setCurrentText("西药");
        ui->cmbDosageForm->setCurrentText("片剂");
        ui->cmbStatus->setCurrentText("active");
        ui->spinExpirationDays->setValue(730); // 2年
        ui->dateProduction->setDate(QDate::currentDate());
        calculateExpiryDate();
    } else {
        dataMapper->setCurrentIndex(index);

        // 获取当前记录的分类、剂型和状态
        QSqlRecord rec = tabModel->record(index);
        QString category = rec.value("category").toString();
        QString dosageForm = rec.value("dosage_form").toString();
        QString status = rec.value("status").toString();

        if (!category.isEmpty()) {
            int categoryIndex = ui->cmbCategory->findText(category);
            if (categoryIndex >= 0) ui->cmbCategory->setCurrentIndex(categoryIndex);
        }

        if (!dosageForm.isEmpty()) {
            int dosageFormIndex = ui->cmbDosageForm->findText(dosageForm);
            if (dosageFormIndex >= 0) ui->cmbDosageForm->setCurrentIndex(dosageFormIndex);
        }

        if (!status.isEmpty()) {
            int statusIndex = ui->cmbStatus->findText(status);
            if (statusIndex >= 0) ui->cmbStatus->setCurrentIndex(statusIndex);
        }
    }

    // 连接信号槽
    connect(ui->btnSave, &QPushButton::clicked, this, &MedicineEditView::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &MedicineEditView::on_btnCancel_clicked);
    connect(ui->spinExpirationDays, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MedicineEditView::calculateExpiryDate);
    connect(ui->dateProduction, &QDateEdit::dateChanged,
            this, &MedicineEditView::calculateExpiryDate);
}

MedicineEditView::~MedicineEditView()
{
    delete ui;
}

void MedicineEditView::initUI()
{
    // 设置日期范围
    ui->dateProduction->setDate(QDate::currentDate());
    ui->dateProduction->setMaximumDate(QDate::currentDate());
    ui->dateProduction->setMinimumDate(QDate(2000, 1, 1));

    ui->dateExpiry->setMinimumDate(QDate::currentDate());

    // 设置数值范围
    ui->spinPrice->setMinimum(0.0);
    ui->spinPrice->setMaximum(10000.0);
    ui->spinPrice->setDecimals(2);

    ui->spinCost->setMinimum(0.0);
    ui->spinCost->setMaximum(10000.0);
    ui->spinCost->setDecimals(2);

    ui->spinStock->setMinimum(0);
    ui->spinStock->setMaximum(100000);

    ui->spinMinStock->setMinimum(0);
    ui->spinMinStock->setMaximum(10000);

    ui->spinMaxStock->setMinimum(0);
    ui->spinMaxStock->setMaximum(100000);

    ui->spinExpirationDays->setMinimum(1);
    ui->spinExpirationDays->setMaximum(365 * 5); // 最长5年

    // 隐藏错误标签
    clearError();
}

void MedicineEditView::populateComboBoxes()
{
    // 药品分类
    ui->cmbCategory->clear();
    QStringList categories = IDatabase::getInstance().getMedicineCategories();
    for (const QString &category : categories) {
        ui->cmbCategory->addItem(category);
    }

    // 剂型
    ui->cmbDosageForm->clear();
    QStringList dosageForms = IDatabase::getInstance().getDosageForms();
    for (const QString &dosageForm : dosageForms) {
        ui->cmbDosageForm->addItem(dosageForm);
    }

    // 状态
    ui->cmbStatus->clear();
    ui->cmbStatus->addItem("active");
    ui->cmbStatus->addItem("discontinued");
}

bool MedicineEditView::validateInput()
{
    clearError();

    // 验证药品名称
    if (ui->txtName->text().trimmed().isEmpty()) {
        showError("药品名称不能为空");
        ui->txtName->setFocus();
        return false;
    }

    // 验证规格
    if (ui->txtSpecification->text().trimmed().isEmpty()) {
        showError("药品规格不能为空");
        ui->txtSpecification->setFocus();
        return false;
    }

    // 验证单位
    if (ui->txtUnit->text().trimmed().isEmpty()) {
        showError("药品单位不能为空");
        ui->txtUnit->setFocus();
        return false;
    }

    // 验证价格
    if (ui->spinPrice->value() < 0) {
        showError("价格不能为负数");
        ui->spinPrice->setFocus();
        return false;
    }

    // 验证成本
    if (ui->spinCost->value() < 0) {
        showError("成本不能为负数");
        ui->spinCost->setFocus();
        return false;
    }

    // 验证库存
    if (ui->spinStock->value() < 0) {
        showError("库存不能为负数");
        ui->spinStock->setFocus();
        return false;
    }

    // 验证最小库存
    if (ui->spinMinStock->value() < 0) {
        showError("最小库存不能为负数");
        ui->spinMinStock->setFocus();
        return false;
    }

    // 验证最大库存
    if (ui->spinMaxStock->value() < 0) {
        showError("最大库存不能为负数");
        ui->spinMaxStock->setFocus();
        return false;
    }

    // 验证最小库存不能大于最大库存
    if (ui->spinMinStock->value() > ui->spinMaxStock->value()) {
        showError("最小库存不能大于最大库存");
        ui->spinMinStock->setFocus();
        return false;
    }

    // 验证生产日期不能大于过期日期
    if (ui->dateProduction->date() >= ui->dateExpiry->date()) {
        showError("生产日期必须早于过期日期");
        ui->dateProduction->setFocus();
        return false;
    }

    return true;
}

void MedicineEditView::showError(const QString &message)
{
    ui->lblError->setText("错误: " + message);
    ui->lblError->setVisible(true);
}

void MedicineEditView::clearError()
{
    ui->lblError->clear();
    ui->lblError->setVisible(false);
}

void MedicineEditView::calculateExpiryDate()
{
    if (ui->dateProduction->date().isValid()) {
        QDate expiryDate = ui->dateProduction->date().addDays(ui->spinExpirationDays->value());
        ui->dateExpiry->setDate(expiryDate);
    }
}

void MedicineEditView::on_btnSave_clicked()
{
    if (!validateInput()) {
        return;
    }

    // 获取当前模型
    QSqlTableModel *tabModel = IDatabase::getInstance().medicineTabModel;

    // 1. 确保当前 Mapper 的数据写入模型缓存
    dataMapper->submit();

    // 2. 手动更新下拉框的值 (因为 Mapper 可能没映射下拉框)
    QSqlRecord rec = tabModel->record(dataMapper->currentIndex());
    rec.setValue("category", ui->cmbCategory->currentText());
    rec.setValue("dosage_form", ui->cmbDosageForm->currentText());
    rec.setValue("status", ui->cmbStatus->currentText());
    rec.setValue("expiration_days", ui->spinExpirationDays->value());

    // 把记录写回模型
    tabModel->setRecord(dataMapper->currentIndex(), rec);

    // 3. 提交到数据库并检查错误
    if (IDatabase::getInstance().submitMedicineEdit()) {
        QMessageBox::information(this, "成功", isNewMedicine ? "药品添加成功" : "更新成功");
        emit goPreviousView();
    } else {
        // --- 重点：显示具体的数据库错误 ---
        QString err = tabModel->lastError().text();
        qDebug() << "提交失败原因：" << err;
        QMessageBox::critical(this, "保存失败", "数据库错误：" + err);
    }
}

void MedicineEditView::on_btnCancel_clicked()
{
    // 撤销未保存的更改
    IDatabase::getInstance().revertMedicineEdit();
    emit goPreviousView();
}
