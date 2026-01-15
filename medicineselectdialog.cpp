#include "medicineselectdialog.h"
#include "ui_medicineselectdialog.h"
#include "idatabase.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

MedicineSelectDialog::MedicineSelectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MedicineSelectDialog)
    , medicineModel(nullptr)
    , unitPrice(0.0)
{
    ui->setupUi(this);
    this->setWindowTitle("选择药品");

    initTableView();

    // 默认数量为1
    ui->spinQuantity->setValue(1);
    ui->spinQuantity->setMinimum(1);
    ui->spinQuantity->setMaximum(999);

    // 连接信号槽
    connect(ui->tableView, &QTableView::clicked,
            this, &MedicineSelectDialog::updateMedicineInfo);
    connect(ui->spinQuantity, SIGNAL(valueChanged(int)),
            this, SLOT(on_spinQuantity_valueChanged(int)));
}

MedicineSelectDialog::~MedicineSelectDialog()
{
    delete ui;
}

void MedicineSelectDialog::initTableView()
{
    // 初始化药品模型
    medicineModel = new QSqlTableModel(this);
    medicineModel->setTable("medicine");
    medicineModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    medicineModel->setFilter("status = 'active' AND stock > 0");
    medicineModel->setSort(medicineModel->fieldIndex("name"), Qt::AscendingOrder);

    if (medicineModel->select()) {
        // 设置表格属性
        ui->tableView->setModel(medicineModel);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->setAlternatingRowColors(true);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(medicineModel->fieldIndex("generic_name"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("category"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("manufacturer"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("cost"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("min_stock"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("max_stock"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("expiry_date"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("status"));
        ui->tableView->hideColumn(medicineModel->fieldIndex("created_time"));

        // 设置列宽
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("code"), 100);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("name"), 150);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("specification"), 100);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("unit"), 60);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("price"), 80);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("stock"), 80);
        ui->tableView->setColumnWidth(medicineModel->fieldIndex("dosage_form"), 100);
    } else {
        QMessageBox::warning(this, "错误", "加载药品列表失败");
    }
}

void MedicineSelectDialog::updateMedicineInfo()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        return;
    }

    QSqlRecord rec = medicineModel->record(currentIndex.row());
    selectedMedicineId = rec.value("id").toString();
    selectedMedicineName = rec.value("name").toString();
    unitPrice = rec.value("price").toDouble();
    int stock = rec.value("stock").toInt();

    // 更新界面显示
    ui->lblMedicineName->setText(selectedMedicineName);
    ui->lblSpecification->setText(rec.value("specification").toString());
    ui->lblUnit->setText(rec.value("unit").toString());
    ui->lblPrice->setText(QString("¥%1").arg(unitPrice, 0, 'f', 2));
    ui->lblStock->setText(QString("%1").arg(stock));

    // 更新最大可购买数量
    ui->spinQuantity->setMaximum(stock);

    // 计算总价
    calculateTotal();
}

void MedicineSelectDialog::calculateTotal()
{
    int quantity = ui->spinQuantity->value();
    double total = unitPrice * quantity;
    ui->lblTotal->setText(QString("¥%1").arg(total, 0, 'f', 2));
}

QString MedicineSelectDialog::getSelectedMedicineId() const
{
    return selectedMedicineId;
}

QString MedicineSelectDialog::getSelectedMedicineName() const
{
    return selectedMedicineName;
}

int MedicineSelectDialog::getQuantity() const
{
    return ui->spinQuantity->value();
}

QString MedicineSelectDialog::getDosage() const
{
    return ui->txtDosage->text().trimmed();
}

void MedicineSelectDialog::on_btnSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (searchText.isEmpty()) {
        medicineModel->setFilter("status = 'active' AND stock > 0");
    } else {
        QString filter = QString("(name LIKE '%%1%' OR code LIKE '%%1%' OR generic_name LIKE '%%1%') AND status = 'active' AND stock > 0")
        .arg(searchText);
        medicineModel->setFilter(filter);
    }

    if (!medicineModel->select()) {
        QMessageBox::warning(this, "搜索失败", "搜索药品失败");
    }
}

void MedicineSelectDialog::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}

void MedicineSelectDialog::on_spinQuantity_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    calculateTotal();
}

void MedicineSelectDialog::on_btnSelect_clicked()
{
    if (selectedMedicineId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择药品");
        return;
    }

    QString dosage = ui->txtDosage->text().trimmed();
    if (dosage.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用法用量");
        ui->txtDosage->setFocus();
        return;
    }

    int quantity = ui->spinQuantity->value();
    if (quantity <= 0) {
        QMessageBox::warning(this, "错误", "数量必须大于0");
        return;
    }

    // 检查库存
    QSqlQuery query;
    query.prepare("SELECT stock FROM medicine WHERE id = ?");
    query.addBindValue(selectedMedicineId);
    if (query.exec() && query.next()) {
        int stock = query.value(0).toInt();
        if (quantity > stock) {
            QMessageBox::warning(this, "库存不足",
                                 QString("药品库存不足，当前库存：%1").arg(stock));
            return;
        }
    }

    this->accept(); // 关闭对话框并返回Accepted
}

void MedicineSelectDialog::on_btnCancel_clicked()
{
    this->reject(); // 关闭对话框并返回Rejected
}

void MedicineSelectDialog::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    // 双击直接选中当前药品并填入默认用法用量
    if (!selectedMedicineId.isEmpty()) {
        // 设置默认用法用量
        if (ui->txtDosage->text().isEmpty()) {
            ui->txtDosage->setText("每日三次，每次一片");
        }
        on_btnSelect_clicked();
    }
}
