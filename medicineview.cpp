#include "medicineview.h"
#include "ui_medicineview.h"
#include "idatabase.h"
#include "stockdialog.h"
#include "medicineeditview.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QStyledItemDelegate>

// 自定义委托，用于高亮显示
class HighlightDelegate : public QStyledItemDelegate
{
public:
    HighlightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        // 获取行数据
        QSqlTableModel *model = qobject_cast<QSqlTableModel*>(const_cast<QAbstractItemModel*>(index.model()));
        if (model) {
            QSqlRecord rec = model->record(index.row());
            int stock = rec.value("stock").toInt();
            int minStock = rec.value("min_stock").toInt();
            QDate expiryDate = rec.value("expiry_date").toDate();

            // 低库存高亮（红色）
            if (stock <= minStock) {
                option->backgroundBrush = QBrush(QColor(255, 200, 200));
            }
            // 近效期高亮（黄色，30天内过期）
            else if (expiryDate.isValid() && expiryDate <= QDate::currentDate().addDays(30)) {
                option->backgroundBrush = QBrush(QColor(255, 255, 200));
            }
        }
    }
};

medicineview::medicineview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::medicineview)
{
    ui->setupUi(this);

    // 初始化界面
    initTableView();
    populateCategoryFilter();
    updateStats();

    // 连接信号槽
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &medicineview::on_tableView_doubleClicked);

    // 设置回车键搜索
    connect(ui->txtSearch, &QLineEdit::returnPressed,
            this, &medicineview::on_txtSearch_returnPressed);
}

medicineview::~medicineview()
{
    delete ui;
}

void medicineview::initTableView()
{
    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);

    // 设置自定义委托
    ui->tableView->setItemDelegate(new HighlightDelegate(this));

    // 初始化模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initMedicineModel()) {
        ui->tableView->setModel(iDatabase.medicineTabModel);
        ui->tableView->setSelectionModel(iDatabase.theMedicineSelection);

        // 隐藏不必要显示的列
        ui->tableView->hideColumn(0); // id
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("created_time"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("cost"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("max_stock"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("storage_location"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("dosage_form"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("indications"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("contraindications"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("side_effects"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("usage_dosage"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("expiration_days"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("batch_number"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("production_date"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("generic_name"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("brand"));
        ui->tableView->hideColumn(iDatabase.medicineTabModel->fieldIndex("approval_number"));

        // 设置列宽
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("code"), 120);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("name"), 150);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("category"), 80);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("specification"), 120);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("unit"), 60);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("manufacturer"), 150);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("price"), 80);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("stock"), 80);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("min_stock"), 80);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("expiry_date"), 100);
        ui->tableView->setColumnWidth(iDatabase.medicineTabModel->fieldIndex("status"), 80);
    } else {
        QMessageBox::warning(this, "错误", "初始化药品数据失败");
    }
}

void medicineview::populateCategoryFilter()
{
    ui->cmbCategory->clear();
    ui->cmbCategory->addItem("所有分类", "");

    QStringList categories = IDatabase::getInstance().getMedicineCategories();
    for (const QString &category : categories) {
        ui->cmbCategory->addItem(category, category);
    }
}

void medicineview::updateStats()
{
    IDatabase &iDatabase = IDatabase::getInstance();

    // 获取低库存药品数量
    QList<QString> lowStockMedicines = iDatabase.getLowStockMedicines();
    ui->lblStockWarning->setText(QString("库存预警：%1种药品").arg(lowStockMedicines.size()));

    // 获取近效期药品数量
    QList<QString> nearExpiryMedicines = iDatabase.getNearExpiryMedicines();
    ui->lblExpiryWarning->setText(QString("近效期：%1种药品").arg(nearExpiryMedicines.size()));

    // 获取库存总值
    double totalValue = iDatabase.getTotalInventoryValue();
    ui->lblTotalValue->setText(QString("库存总值：%1元").arg(totalValue, 0, 'f', 2));
}

void medicineview::refreshData()
{
    IDatabase::getInstance().medicineTabModel->select();
    updateStats();
    highlightLowStock();
    highlightNearExpiry();
}

void medicineview::highlightLowStock()
{
    // 表格委托会自动高亮，这里主要是为了更新统计信息
    updateStats();
}

void medicineview::highlightNearExpiry()
{
    // 表格委托会自动高亮，这里主要是为了更新统计信息
    updateStats();
}

void medicineview::on_btSearch_clicked()
{
    QString searchText = ui->txtSearch->text().trimmed();
    if (IDatabase::getInstance().searchMedicine(searchText)) {
        updateStats();
    } else {
        QMessageBox::warning(this, "搜索失败", "搜索药品信息失败");
    }
}

void medicineview::on_btAdd_clicked()
{
    // 新增药品
    int currow = IDatabase::getInstance().addNewMedicine();
    qDebug() << "新增药品，行号：" << currow;

    // 发出信号，通知主视图跳转到编辑界面
    emit goMedicineEditView(currow);
}

void medicineview::on_btDelete_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theMedicineSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的药品");
        return;
    }

    QString medicineName = IDatabase::getInstance().medicineTabModel
                               ->record(currentIndex.row()).value("name").toString();
    QString medicineCode = IDatabase::getInstance().medicineTabModel
                               ->record(currentIndex.row()).value("code").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除药品 '%1 (编码: %2)' 吗？")
                                      .arg(medicineName).arg(medicineCode),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (IDatabase::getInstance().deleteCurrentMedicine()) {
            refreshData();
            QMessageBox::information(this, "成功", "药品删除成功");
        } else {
            QMessageBox::warning(this, "错误",
                                 "无法删除药品，该药品可能库存不为零或有处方使用记录。");
        }
    }
}

void medicineview::on_btEdit_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theMedicineSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的药品");
        return;
    }

    // 发出信号，通知主视图跳转到编辑界面
    emit goMedicineEditView(currentIndex.row());
}

void medicineview::on_btnLowStock_clicked()
{
    // 显示低库存药品
    IDatabase &iDatabase = IDatabase::getInstance();
    QList<QString> lowStockMedicines = iDatabase.getLowStockMedicines();

    if (lowStockMedicines.isEmpty()) {
        QMessageBox::information(this, "库存预警", "没有低库存药品。");
    } else {
        QString message = "低库存药品列表：\n\n";
        for (const QString &medicine : lowStockMedicines) {
            message += "• " + medicine + "\n";
        }
        QMessageBox::information(this, "库存预警", message);
    }
}

void medicineview::on_btnNearExpiry_clicked()
{
    // 显示近效期药品
    IDatabase &iDatabase = IDatabase::getInstance();
    QList<QString> nearExpiryMedicines = iDatabase.getNearExpiryMedicines();

    if (nearExpiryMedicines.isEmpty()) {
        QMessageBox::information(this, "近效期预警", "没有近效期药品。");
    } else {
        QString message = "近效期药品列表（30天内过期）：\n\n";
        for (const QString &medicine : nearExpiryMedicines) {
            message += "• " + medicine + "\n";
        }
        QMessageBox::information(this, "近效期预警", message);
    }
}

void medicineview::on_btnStockIn_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theMedicineSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要入库的药品");
        return;
    }

    QString medicineId = IDatabase::getInstance().medicineTabModel
                             ->record(currentIndex.row()).value("id").toString();
    QString medicineName = IDatabase::getInstance().medicineTabModel
                               ->record(currentIndex.row()).value("name").toString();

    StockDialog dialog(this, StockDialog::StockIn, medicineName, medicineId);
    if (dialog.exec() == QDialog::Accepted) {
        int quantity = dialog.getQuantity();
        QString batchNumber = dialog.getBatchNumber();
        QDate expiryDate = dialog.getExpiryDate();

        if (IDatabase::getInstance().stockIn(medicineId, quantity, batchNumber, expiryDate)) {
            refreshData();
            QMessageBox::information(this, "成功", QString("药品入库成功，数量：%1").arg(quantity));
        } else {
            QMessageBox::warning(this, "错误", "药品入库失败");
        }
    }
}

void medicineview::on_btnStockOut_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theMedicineSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要出库的药品");
        return;
    }

    QString medicineId = IDatabase::getInstance().medicineTabModel
                             ->record(currentIndex.row()).value("id").toString();
    QString medicineName = IDatabase::getInstance().medicineTabModel
                               ->record(currentIndex.row()).value("name").toString();
    int currentStock = IDatabase::getInstance().medicineTabModel
                           ->record(currentIndex.row()).value("stock").toInt();

    if (currentStock <= 0) {
        QMessageBox::warning(this, "错误", "该药品库存为零，无法出库");
        return;
    }

    StockDialog dialog(this, StockDialog::StockOut, medicineName, medicineId);
    if (dialog.exec() == QDialog::Accepted) {
        int quantity = dialog.getQuantity();

        if (quantity > currentStock) {
            QMessageBox::warning(this, "错误",
                                 QString("出库数量不能超过当前库存。当前库存：%1").arg(currentStock));
            return;
        }

        if (IDatabase::getInstance().stockOut(medicineId, quantity)) {
            refreshData();
            QMessageBox::information(this, "成功", QString("药品出库成功，数量：%1").arg(quantity));
        } else {
            QMessageBox::warning(this, "错误", "药品出库失败");
        }
    }
}

void medicineview::on_btnAdjustStock_clicked()
{
    QModelIndex currentIndex = IDatabase::getInstance().theMedicineSelection->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要调整库存的药品");
        return;
    }

    QString medicineId = IDatabase::getInstance().medicineTabModel
                             ->record(currentIndex.row()).value("id").toString();
    QString medicineName = IDatabase::getInstance().medicineTabModel
                               ->record(currentIndex.row()).value("name").toString();
    int currentStock = IDatabase::getInstance().medicineTabModel
                           ->record(currentIndex.row()).value("stock").toInt();

    StockDialog dialog(this, StockDialog::StockAdjust, medicineName, medicineId);
    if (dialog.exec() == QDialog::Accepted) {
        int newQuantity = dialog.getQuantity();

        if (IDatabase::getInstance().adjustStock(medicineId, newQuantity)) {
            refreshData();
            QMessageBox::information(this, "成功",
                                     QString("库存调整成功，新库存：%1").arg(newQuantity));
        } else {
            QMessageBox::warning(this, "错误", "库存调整失败");
        }
    }
}

void medicineview::on_cmbCategory_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString category = ui->cmbCategory->currentData().toString();

    if (!category.isEmpty()) {
        IDatabase::getInstance().medicineTabModel->setFilter(
            QString("category = '%1'").arg(category));
        IDatabase::getInstance().medicineTabModel->select();
        updateStats();
    } else {
        IDatabase::getInstance().medicineTabModel->setFilter("");
        IDatabase::getInstance().medicineTabModel->select();
        updateStats();
    }
}

void medicineview::on_txtSearch_returnPressed()
{
    on_btSearch_clicked();
}

void medicineview::on_tableView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_btEdit_clicked();
}
