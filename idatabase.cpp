#include "idatabase.h"
#include <QUuid>

void IDatabase::ininDatabase()
{
    database=QSqlDatabase::addDatabase("QSQLITE");
    QString aFile="C:/Users/白夜/Documents/Lab4.db";
    database.setDatabaseName(aFile);


    if(!database.open()){
        qDebug() <<"failed to open database";
    }else {
        qDebug()<<"open database is ok"<<database.connectionName();
    }
}

bool IDatabase::initPatientModel()
{
    patientTabModel =new QSqlTableModel(this,database);
    patientTabModel->setTable("patient");
    patientTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    patientTabModel->setSort(patientTabModel->fieldIndex("name"),Qt::AscendingOrder);
    if(!(patientTabModel->select()))
        return false;

    thePatientSelection=new QItemSelectionModel(patientTabModel);
    return true;
}

int IDatabase::addNewPatient()
{
    patientTabModel->insertRow(patientTabModel->rowCount(),QModelIndex());
    QModelIndex curIndex=patientTabModel->index(patientTabModel->rowCount() - 1,1);
    int curRecNo=curIndex.row();
    QSqlRecord curRec=patientTabModel->record(curRecNo);
    curRec.setValue("CREATEDTIMESTAMP", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    curRec.setValue("ID",QUuid::createUuid().toString(QUuid::WithoutBraces));

    patientTabModel->setRecord(curRecNo,curRec);

    return curIndex.row();
}

bool IDatabase::searchPatient(QString filter)
{
    patientTabModel->setFilter(filter);
    return patientTabModel->select();
}

bool IDatabase::deleteCurrentPatient()
{
    QModelIndex curIndex = thePatientSelection->currentIndex();
    patientTabModel->removeRow(curIndex.row());
    patientTabModel->submitAll();
    patientTabModel->select();
}

bool IDatabase::submitPatientEdit()
{
    return patientTabModel->submitAll();
}

void IDatabase::revertPatientEdit()
{
    patientTabModel->revertAll();
}

QString IDatabase::userLogin(QString userName, QString password)
{
    //return "loginOK";
    QSqlQuery query;
    query.prepare("select username,password from user where username= :USER");
    query.bindValue(":USER",userName);
    query.exec();
    qDebug()<<query.lastQuery()<<query.first();

    if(query.first()&&query.value("username").isValid()){
        QString passwd=query.value("password").toString();
        if(passwd==password){
            qDebug()<<"login ok";
            return "loginOK";
        }
        else{
            qDebug()<<"wrong password";
            return "wrongPassword";
        }
    }else{
        qDebug()<<"no such user";
        return "loginFailed";
    }
}

IDatabase::IDatabase(QObject *parent)
    : QObject{parent}
{
    ininDatabase();
}

// 医生管理方法实现
bool IDatabase::initDoctorModel()
{
    doctorTabModel = new QSqlTableModel(this, database);
    doctorTabModel->setTable("doctor");
    doctorTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("employee_id"), Qt::Horizontal, "工号");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("name"), Qt::Horizontal, "姓名");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("gender"), Qt::Horizontal, "性别");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("department_id"), Qt::Horizontal, "科室");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("title"), Qt::Horizontal, "职称");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("specialization"), Qt::Horizontal, "专业方向");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("phone"), Qt::Horizontal, "联系电话");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("email"), Qt::Horizontal, "邮箱");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("work_years"), Qt::Horizontal, "工作年限");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("consultation_fee"), Qt::Horizontal, "诊费");
    doctorTabModel->setHeaderData(doctorTabModel->fieldIndex("status"), Qt::Horizontal, "状态");

    // 按姓名字段排序
    doctorTabModel->setSort(doctorTabModel->fieldIndex("name"), Qt::AscendingOrder);

    if (!doctorTabModel->select()) {
        qDebug() << "初始化医生模型失败：" << doctorTabModel->lastError();
        return false;
    }

    theDoctorSelection = new QItemSelectionModel(doctorTabModel);
    qDebug() << "医生模型初始化成功，记录数：" << doctorTabModel->rowCount();
    return true;
}

int IDatabase::addNewDoctor()
{
    // 生成医生ID - 使用UUID
    QString doctorId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 获取当前最大工号（独立于ID）
    QSqlQuery query("SELECT MAX(employee_id) FROM doctor");
    int nextEmployeeId = 10001; // 初始值
    if (query.exec() && query.next()) {
        QString maxId = query.value(0).toString();
        if (!maxId.isEmpty()) {
            nextEmployeeId = maxId.toInt() + 1;
        }
    }

    doctorTabModel->insertRow(doctorTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = doctorTabModel->index(doctorTabModel->rowCount() - 1, 1);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = doctorTabModel->record(curRecNo);

    // 设置医生ID和默认值
    curRec.setValue("id", doctorId);  // 医生唯一ID
    curRec.setValue("employee_id", QString::number(nextEmployeeId));  // 工号
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("status", "active");
    curRec.setValue("title", "主治医师"); // 默认职称
    curRec.setValue("gender", "男");      // 默认性别
    curRec.setValue("work_years", 0);
    curRec.setValue("consultation_fee", 15.0);

    doctorTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增医生，ID：" << doctorId << "，工号：" << curRec.value("employee_id").toString();
    return curRecNo;
}

bool IDatabase::searchDoctor(const QString &filter)
{
    if (filter.isEmpty()) {
        doctorTabModel->setFilter("");
    } else {
        QString whereClause = QString("name LIKE '%%1%' OR employee_id LIKE '%%1%' OR phone LIKE '%%1%'")
        .arg(filter);
        doctorTabModel->setFilter(whereClause);
    }

    bool success = doctorTabModel->select();
    if (!success) {
        qDebug() << "搜索医生失败：" << doctorTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentDoctor()
{
    QModelIndex curIndex = theDoctorSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择医生";
        return false;
    }

    // 获取医生信息
    QString doctorId = doctorTabModel->record(curIndex.row()).value("id").toString();
    QString doctorName = doctorTabModel->record(curIndex.row()).value("name").toString();

    // 检查该医生是否有未完成的预约
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM appointment WHERE doctor_id = ? AND status IN ('scheduled', 'confirmed')");
    query.addBindValue(doctorId);
    query.exec();

    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "无法删除医生：" << doctorName << "，存在未完成的预约";
        return false;
    }

    // 执行删除
    if (doctorTabModel->removeRow(curIndex.row())) {
        bool success = doctorTabModel->submitAll();
        if (success) {
            doctorTabModel->select(); // 刷新数据
            qDebug() << "删除医生成功：" << doctorName;
        } else {
            qDebug() << "删除医生失败：" << doctorTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitDoctorEdit()
{
    bool success = doctorTabModel->submitAll();
    if (!success) {
        qDebug() << "提交医生编辑失败：" << doctorTabModel->lastError();
    }
    return success;
}

void IDatabase::revertDoctorEdit()
{
    doctorTabModel->revertAll();
    qDebug() << "撤销医生编辑";
}

QList<QString> IDatabase::getDepartmentsForCombo()
{
    QList<QString> departments;

    // 如果department表不存在，返回测试数据
    QSqlQuery checkTable("SELECT name FROM sqlite_master WHERE type='table' AND name='department'");
    if (!checkTable.next()) {
        // 表不存在，返回模拟数据
        departments << "内科 (dept001)" << "外科 (dept002)" << "儿科 (dept003)"
                    << "妇产科 (dept004)" << "骨科 (dept005)" << "眼科 (dept006)";
        return departments;
    }

    QSqlQuery query("SELECT id, name FROM department WHERE status = 'active' ORDER BY name");

    while (query.next()) {
        QString displayText = QString("%1 (%2)")
        .arg(query.value("name").toString())
            .arg(query.value("id").toString());
        departments.append(displayText);
    }

    // 如果没有数据，添加默认选项
    if (departments.isEmpty()) {
        departments << "未分配科室";
    }

    return departments;
}

QStringList IDatabase::getDoctorTitles()
{
    return QStringList() << "住院医师" << "主治医师" << "副主任医师" << "主任医师";
}

QStringList IDatabase::getDoctorStatuses()
{
    return QStringList() << "active" << "leave" << "retired";
}

// 科室管理方法实现
bool IDatabase::initDepartmentModel()
{
    departmentTabModel = new QSqlTableModel(this, database);
    departmentTabModel->setTable("department");
    departmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("name"), Qt::Horizontal, "科室名称");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("location"), Qt::Horizontal, "位置/楼层");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("phone"), Qt::Horizontal, "科室电话");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("description"), Qt::Horizontal, "描述");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("director_id"), Qt::Horizontal, "科室主任");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("established_date"), Qt::Horizontal, "成立日期");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("bed_count"), Qt::Horizontal, "床位数量");
    departmentTabModel->setHeaderData(departmentTabModel->fieldIndex("status"), Qt::Horizontal, "状态");

    // 按科室名称排序
    departmentTabModel->setSort(departmentTabModel->fieldIndex("name"), Qt::AscendingOrder);

    if (!departmentTabModel->select()) {
        qDebug() << "初始化科室模型失败：" << departmentTabModel->lastError();
        return false;
    }

    theDepartmentSelection = new QItemSelectionModel(departmentTabModel);
    qDebug() << "科室模型初始化成功，记录数：" << departmentTabModel->rowCount();
    return true;
}

int IDatabase::addNewDepartment()
{
    departmentTabModel->insertRow(departmentTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = departmentTabModel->index(departmentTabModel->rowCount() - 1, 0);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = departmentTabModel->record(curRecNo);

    // 设置默认值
    curRec.setValue("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("status", "active");
    curRec.setValue("bed_count", 0);
    curRec.setValue("established_date", QDate::currentDate());

    departmentTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增科室，ID：" << curRec.value("id").toString();
    return curRecNo;
}

bool IDatabase::searchDepartment(const QString &filter)
{
    if (filter.isEmpty()) {
        departmentTabModel->setFilter("");
    } else {
        QString whereClause = QString("name LIKE '%%1%' OR location LIKE '%%1%' OR phone LIKE '%%1%'")
        .arg(filter);
        departmentTabModel->setFilter(whereClause);
    }

    bool success = departmentTabModel->select();
    if (!success) {
        qDebug() << "搜索科室失败：" << departmentTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentDepartment()
{
    QModelIndex curIndex = theDepartmentSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择科室";
        return false;
    }

    // 获取科室信息
    QString departmentId = departmentTabModel->record(curIndex.row()).value("id").toString();
    QString departmentName = departmentTabModel->record(curIndex.row()).value("name").toString();

    // 检查该科室是否有医生
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM doctor WHERE department_id = ? AND status = 'active'");
    query.addBindValue(departmentId);
    query.exec();

    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "无法删除科室：" << departmentName << "，该科室下有在职医生";
        return false;
    }

    // 检查该科室是否有预约
    query.prepare("SELECT COUNT(*) FROM appointment WHERE department_id = ? AND status IN ('scheduled', 'confirmed')");
    query.addBindValue(departmentId);
    query.exec();

    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "无法删除科室：" << departmentName << "，该科室下有未完成的预约";
        return false;
    }

    // 执行删除
    if (departmentTabModel->removeRow(curIndex.row())) {
        bool success = departmentTabModel->submitAll();
        if (success) {
            departmentTabModel->select(); // 刷新数据
            qDebug() << "删除科室成功：" << departmentName;
        } else {
            qDebug() << "删除科室失败：" << departmentTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitDepartmentEdit()
{
    bool success = departmentTabModel->submitAll();
    if (!success) {
        qDebug() << "提交科室编辑失败：" << departmentTabModel->lastError();
    }
    return success;
}

void IDatabase::revertDepartmentEdit()
{
    departmentTabModel->revertAll();
    qDebug() << "撤销科室编辑";
}

QStringList IDatabase::getDepartmentStatuses()
{
    return QStringList() << "active" << "inactive" << "under_construction";
}

// 药品管理方法实现
bool IDatabase::initMedicineModel()
{
    medicineTabModel = new QSqlTableModel(this, database);
    medicineTabModel->setTable("medicine");
    medicineTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("code"), Qt::Horizontal, "药品编码");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("name"), Qt::Horizontal, "药品名称");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("generic_name"), Qt::Horizontal, "通用名");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("category"), Qt::Horizontal, "分类");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("specification"), Qt::Horizontal, "规格");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("unit"), Qt::Horizontal, "单位");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("manufacturer"), Qt::Horizontal, "生产厂家");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("price"), Qt::Horizontal, "单价");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("stock"), Qt::Horizontal, "库存");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("min_stock"), Qt::Horizontal, "最小库存");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("expiry_date"), Qt::Horizontal, "过期日期");
    medicineTabModel->setHeaderData(medicineTabModel->fieldIndex("status"), Qt::Horizontal, "状态");

    // 按药品名称排序
    medicineTabModel->setSort(medicineTabModel->fieldIndex("name"), Qt::AscendingOrder);

    if (!medicineTabModel->select()) {
        qDebug() << "初始化药品模型失败：" << medicineTabModel->lastError();
        return false;
    }

    theMedicineSelection = new QItemSelectionModel(medicineTabModel);
    qDebug() << "药品模型初始化成功，记录数：" << medicineTabModel->rowCount();
    return true;
}

int IDatabase::addNewMedicine()
{
    medicineTabModel->insertRow(medicineTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = medicineTabModel->index(medicineTabModel->rowCount() - 1, 0);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = medicineTabModel->record(curRecNo);

    // 生成药品编码（规则：MED + 年月日 + 4位随机数）
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    QString randomStr = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    QString medicineCode = "MED" + dateStr + randomStr;

    // 设置默认值
    curRec.setValue("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    curRec.setValue("code", medicineCode);
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("status", "active");
    curRec.setValue("category", "西药");
    curRec.setValue("unit", "盒");
    curRec.setValue("dosage_form", "片剂");
    curRec.setValue("price", 0.0);
    curRec.setValue("cost", 0.0);
    curRec.setValue("stock", 0);
    curRec.setValue("min_stock", 10);
    curRec.setValue("max_stock", 1000);
    curRec.setValue("expiration_days", 365 * 2); // 默认有效期2年

    medicineTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增药品，ID：" << curRec.value("id").toString()
             << "，编码：" << curRec.value("code").toString();
    return curRecNo;
}

bool IDatabase::searchMedicine(const QString &filter)
{
    if (filter.isEmpty()) {
        medicineTabModel->setFilter("");
    } else {
        QString whereClause = QString("name LIKE '%%1%' OR code LIKE '%%1%' OR manufacturer LIKE '%%1%' OR generic_name LIKE '%%1%'")
        .arg(filter);
        medicineTabModel->setFilter(whereClause);
    }

    bool success = medicineTabModel->select();
    if (!success) {
        qDebug() << "搜索药品失败：" << medicineTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentMedicine()
{
    QModelIndex curIndex = theMedicineSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择药品";
        return false;
    }

    // 获取药品信息
    QString medicineId = medicineTabModel->record(curIndex.row()).value("id").toString();
    QString medicineName = medicineTabModel->record(curIndex.row()).value("name").toString();

    // 检查该药品是否有库存
    int stock = medicineTabModel->record(curIndex.row()).value("stock").toInt();
    if (stock > 0) {
        qDebug() << "无法删除药品：" << medicineName << "，库存不为零";
        return false;
    }

    // 检查该药品是否有处方使用记录
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM prescription_detail WHERE medicine_id = ?");
    query.addBindValue(medicineId);
    query.exec();

    if (query.next() && query.value(0).toInt() > 0) {
        qDebug() << "无法删除药品：" << medicineName << "，该药品有处方使用记录";
        return false;
    }

    // 执行删除
    if (medicineTabModel->removeRow(curIndex.row())) {
        bool success = medicineTabModel->submitAll();
        if (success) {
            medicineTabModel->select(); // 刷新数据
            qDebug() << "删除药品成功：" << medicineName;
        } else {
            qDebug() << "删除药品失败：" << medicineTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitMedicineEdit()
{
    bool success = medicineTabModel->submitAll();
    if (!success) {
        qDebug() << "提交药品编辑失败：" << medicineTabModel->lastError();
    }
    return success;
}

void IDatabase::revertMedicineEdit()
{
    medicineTabModel->revertAll();
    qDebug() << "撤销药品编辑";
}

bool IDatabase::stockIn(const QString &medicineId, int quantity, const QString &batchNumber, const QDate &expiryDate)
{
    if (quantity <= 0) {
        qDebug() << "入库数量必须大于0";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT stock, name FROM medicine WHERE id = ?");
    query.addBindValue(medicineId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取药品信息失败";
        return false;
    }

    int oldStock = query.value("stock").toInt();
    QString medicineName = query.value("name").toString();
    int newStock = oldStock + quantity;

    // 更新库存
    query.prepare("UPDATE medicine SET stock = ? WHERE id = ?");
    query.addBindValue(newStock);
    query.addBindValue(medicineId);

    if (!query.exec()) {
        qDebug() << "更新库存失败：" << query.lastError();
        return false;
    }

    // 记录库存变更
    QString recordId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    query.prepare("INSERT INTO inventory_record (id, medicine_id, transaction_type, quantity_change, before_quantity, after_quantity, reference_id, reference_type, operator, transaction_time, batch_number, expiry_date) "
                  "VALUES (?, ?, 'purchase', ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(recordId);
    query.addBindValue(medicineId);
    query.addBindValue(quantity);
    query.addBindValue(oldStock);
    query.addBindValue(newStock);
    query.addBindValue("");
    query.addBindValue("manual");
    query.addBindValue("system");
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(batchNumber);
    query.addBindValue(expiryDate.isValid() ? expiryDate : QDate());

    if (!query.exec()) {
        qDebug() << "记录库存变更失败：" << query.lastError();
        return false;
    }

    qDebug() << "药品入库成功：" << medicineName << "，数量：" << quantity
             << "，新库存：" << newStock;
    return true;
}

bool IDatabase::stockOut(const QString &medicineId, int quantity, const QString &referenceId)
{
    if (quantity <= 0) {
        qDebug() << "出库数量必须大于0";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT stock, name FROM medicine WHERE id = ?");
    query.addBindValue(medicineId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取药品信息失败";
        return false;
    }

    int oldStock = query.value("stock").toInt();
    QString medicineName = query.value("name").toString();

    if (oldStock < quantity) {
        qDebug() << "库存不足：" << medicineName << "，库存：" << oldStock << "，需求：" << quantity;
        return false;
    }

    int newStock = oldStock - quantity;

    // 更新库存
    query.prepare("UPDATE medicine SET stock = ? WHERE id = ?");
    query.addBindValue(newStock);
    query.addBindValue(medicineId);

    if (!query.exec()) {
        qDebug() << "更新库存失败：" << query.lastError();
        return false;
    }

    // 记录库存变更
    QString recordId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    query.prepare("INSERT INTO inventory_record (id, medicine_id, transaction_type, quantity_change, before_quantity, after_quantity, reference_id, reference_type, operator, transaction_time) "
                  "VALUES (?, ?, 'dispense', ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(recordId);
    query.addBindValue(medicineId);
    query.addBindValue(-quantity);
    query.addBindValue(oldStock);
    query.addBindValue(newStock);
    query.addBindValue(referenceId);
    query.addBindValue("prescription");
    query.addBindValue("system");
    query.addBindValue(QDateTime::currentDateTime());

    if (!query.exec()) {
        qDebug() << "记录库存变更失败：" << query.lastError();
        return false;
    }

    qDebug() << "药品出库成功：" << medicineName << "，数量：" << quantity
             << "，新库存：" << newStock;
    return true;
}

bool IDatabase::adjustStock(const QString &medicineId, int newQuantity)
{
    if (newQuantity < 0) {
        qDebug() << "库存数量不能为负数";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT stock, name FROM medicine WHERE id = ?");
    query.addBindValue(medicineId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取药品信息失败";
        return false;
    }

    int oldStock = query.value("stock").toInt();
    QString medicineName = query.value("name").toString();
    int quantityChange = newQuantity - oldStock;

    // 更新库存
    query.prepare("UPDATE medicine SET stock = ? WHERE id = ?");
    query.addBindValue(newQuantity);
    query.addBindValue(medicineId);

    if (!query.exec()) {
        qDebug() << "更新库存失败：" << query.lastError();
        return false;
    }

    // 记录库存调整
    QString recordId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    query.prepare("INSERT INTO inventory_record (id, medicine_id, transaction_type, quantity_change, before_quantity, after_quantity, reference_id, reference_type, operator, transaction_time, notes) "
                  "VALUES (?, ?, 'adjust', ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(recordId);
    query.addBindValue(medicineId);
    query.addBindValue(quantityChange);
    query.addBindValue(oldStock);
    query.addBindValue(newQuantity);
    query.addBindValue("");
    query.addBindValue("adjustment");
    query.addBindValue("system");
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue("库存调整");

    if (!query.exec()) {
        qDebug() << "记录库存调整失败：" << query.lastError();
        return false;
    }

    qDebug() << "库存调整成功：" << medicineName << "，调整：" << quantityChange
             << "，新库存：" << newQuantity;
    return true;
}

QList<QString> IDatabase::getLowStockMedicines(int threshold)
{
    QList<QString> lowStockMedicines;
    QSqlQuery query;
    query.prepare("SELECT name, stock, min_stock FROM medicine WHERE stock <= ? AND status = 'active' ORDER BY stock ASC");
    query.addBindValue(threshold);

    if (query.exec()) {
        while (query.next()) {
            QString medicineName = query.value("name").toString();
            int stock = query.value("stock").toInt();
            int minStock = query.value("min_stock").toInt();
            lowStockMedicines.append(QString("%1 (库存:%2, 最低:%3)").arg(medicineName).arg(stock).arg(minStock));
        }
    }

    return lowStockMedicines;
}

QList<QString> IDatabase::getNearExpiryMedicines(int days)
{
    QList<QString> nearExpiryMedicines;
    QDate checkDate = QDate::currentDate().addDays(days);

    QSqlQuery query;
    query.prepare("SELECT name, expiry_date, stock FROM medicine WHERE expiry_date <= ? AND expiry_date >= ? AND status = 'active' ORDER BY expiry_date ASC");
    query.addBindValue(checkDate);
    query.addBindValue(QDate::currentDate());

    if (query.exec()) {
        while (query.next()) {
            QString medicineName = query.value("name").toString();
            QDate expiryDate = query.value("expiry_date").toDate();
            int stock = query.value("stock").toInt();
            int daysToExpire = QDate::currentDate().daysTo(expiryDate);
            nearExpiryMedicines.append(QString("%1 (到期:%2, 剩余:%3天, 库存:%4)")
                                           .arg(medicineName)
                                           .arg(expiryDate.toString("yyyy-MM-dd"))
                                           .arg(daysToExpire)
                                           .arg(stock));
        }
    }

    return nearExpiryMedicines;
}

double IDatabase::getTotalInventoryValue()
{
    double totalValue = 0.0;
    QSqlQuery query("SELECT SUM(stock * cost) as total FROM medicine WHERE status = 'active'");

    if (query.exec() && query.next()) {
        totalValue = query.value("total").toDouble();
    }

    return totalValue;
}

QStringList IDatabase::getMedicineCategories()
{
    // 实际项目中应该从数据库查询，这里返回常见分类
    return QStringList() << "西药" << "中药" << "中成药" << "处方药" << "非处方药"
                         << "抗生素" << "激素类" << "心脑血管" << "消化系统" << "呼吸系统";
}

QStringList IDatabase::getDosageForms()
{
    // 常见剂型
    return QStringList() << "片剂" << "胶囊" << "注射剂" << "颗粒剂" << "口服液"
                         << "软膏" << "栓剂" << "滴眼液" << "喷雾剂" << "贴剂";
}

// 处方管理方法实现
bool IDatabase::initPrescriptionModel()
{
    prescriptionTabModel = new QSqlTableModel(this, database);
    prescriptionTabModel->setTable("prescription");
    prescriptionTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("prescription_number"), Qt::Horizontal, "处方号");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("patient_id"), Qt::Horizontal, "患者");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("doctor_id"), Qt::Horizontal, "医生");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("prescription_date"), Qt::Horizontal, "开方日期");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("diagnosis"), Qt::Horizontal, "诊断");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("total_amount"), Qt::Horizontal, "总金额");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("payment_status"), Qt::Horizontal, "支付状态");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("dispensing_status"), Qt::Horizontal, "发药状态");
    prescriptionTabModel->setHeaderData(prescriptionTabModel->fieldIndex("status"), Qt::Horizontal, "状态");

    // 按开方日期倒序排序（最新在前）
    prescriptionTabModel->setSort(prescriptionTabModel->fieldIndex("prescription_date"), Qt::DescendingOrder);

    if (!prescriptionTabModel->select()) {
        qDebug() << "初始化处方模型失败：" << prescriptionTabModel->lastError();
        return false;
    }

    thePrescriptionSelection = new QItemSelectionModel(prescriptionTabModel);
    qDebug() << "处方模型初始化成功，记录数：" << prescriptionTabModel->rowCount();
    return true;
}

int IDatabase::addNewPrescription()
{
    // 生成处方号（规则：RX + 年月日 + 4位随机数）
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    QString randomStr = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    QString prescriptionNumber = "RX" + dateStr + randomStr;

    prescriptionTabModel->insertRow(prescriptionTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = prescriptionTabModel->index(prescriptionTabModel->rowCount() - 1, 0);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = prescriptionTabModel->record(curRecNo);

    // 设置默认值
    curRec.setValue("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    curRec.setValue("prescription_number", prescriptionNumber);
    curRec.setValue("prescription_date", QDateTime::currentDateTime());
    curRec.setValue("payment_status", "unpaid");
    curRec.setValue("dispensing_status", "pending");
    curRec.setValue("total_amount", 0.0);
    curRec.setValue("created_time", QDateTime::currentDateTime());

    prescriptionTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增处方，ID：" << curRec.value("id").toString()
             << "，处方号：" << curRec.value("prescription_number").toString();
    return curRecNo;
}

bool IDatabase::searchPrescription(const QString &filter)
{
    if (filter.isEmpty()) {
        prescriptionTabModel->setFilter("");
    } else {
        // 构建复杂的查询条件，关联患者和医生表
        QString whereClause = QString(
                                  "prescription_number LIKE '%%1%' OR "
                                  "id IN (SELECT id FROM prescription WHERE "
                                  "patient_id IN (SELECT ID FROM patient WHERE name LIKE '%%1%') OR "
                                  "doctor_id IN (SELECT id FROM doctor WHERE name LIKE '%%1%'))")
                                  .arg(filter);
        prescriptionTabModel->setFilter(whereClause);
    }

    bool success = prescriptionTabModel->select();
    if (!success) {
        qDebug() << "搜索处方失败：" << prescriptionTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentPrescription()
{
    QModelIndex curIndex = thePrescriptionSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择处方";
        return false;
    }

    // 获取处方信息
    QString prescriptionId = prescriptionTabModel->record(curIndex.row()).value("id").toString();
    QString prescriptionNumber = prescriptionTabModel->record(curIndex.row()).value("prescription_number").toString();
    QString dispensingStatus = prescriptionTabModel->record(curIndex.row()).value("dispensing_status").toString();
    QString paymentStatus = prescriptionTabModel->record(curIndex.row()).value("payment_status").toString();

    // 检查处方状态
    if (dispensingStatus == "dispensed") {
        qDebug() << "无法删除处方：" << prescriptionNumber << "，已发药";
        return false;
    }

    if (paymentStatus == "paid") {
        qDebug() << "无法删除处方：" << prescriptionNumber << "，已收费";
        return false;
    }

    // 先删除处方详情
    QSqlQuery query;
    query.prepare("DELETE FROM prescription_detail WHERE prescription_id = ?");
    query.addBindValue(prescriptionId);
    if (!query.exec()) {
        qDebug() << "删除处方详情失败：" << query.lastError();
        return false;
    }

    // 再删除处方
    if (prescriptionTabModel->removeRow(curIndex.row())) {
        bool success = prescriptionTabModel->submitAll();
        if (success) {
            prescriptionTabModel->select(); // 刷新数据
            qDebug() << "删除处方成功：" << prescriptionNumber;
        } else {
            qDebug() << "删除处方失败：" << prescriptionTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitPrescriptionEdit()
{
    bool success = prescriptionTabModel->submitAll();
    if (!success) {
        qDebug() << "提交处方编辑失败：" << prescriptionTabModel->lastError();
    }
    return success;
}

void IDatabase::revertPrescriptionEdit()
{
    prescriptionTabModel->revertAll();
    qDebug() << "撤销处方编辑";
}

bool IDatabase::initPrescriptionDetailModel(const QString &prescriptionId)
{
    prescriptionDetailTabModel = new QSqlTableModel(this, database);
    prescriptionDetailTabModel->setTable("prescription_detail");
    prescriptionDetailTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    prescriptionDetailTabModel->setFilter(QString("prescription_id = '%1'").arg(prescriptionId));

    // 设置表头显示名称
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("medicine_id"), Qt::Horizontal, "药品");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("quantity"), Qt::Horizontal, "数量");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("dosage"), Qt::Horizontal, "用法用量");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("unit_price"), Qt::Horizontal, "单价");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("total_price"), Qt::Horizontal, "小计");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("dispensing_quantity"), Qt::Horizontal, "已发数量");
    prescriptionDetailTabModel->setHeaderData(prescriptionDetailTabModel->fieldIndex("dispensing_status"), Qt::Horizontal, "发药状态");

    if (!prescriptionDetailTabModel->select()) {
        qDebug() << "初始化处方详情模型失败：" << prescriptionDetailTabModel->lastError();
        return false;
    }

    thePrescriptionDetailSelection = new QItemSelectionModel(prescriptionDetailTabModel);
    qDebug() << "处方详情模型初始化成功，记录数：" << prescriptionDetailTabModel->rowCount();
    return true;
}

bool IDatabase::addPrescriptionDetail(const QString &prescriptionId, const QString &medicineId,
                                      int quantity, const QString &dosage)
{
    if (quantity <= 0) {
        qDebug() << "药品数量必须大于0";
        return false;
    }

    // 获取药品信息
    QSqlQuery query;
    query.prepare("SELECT name, price, stock FROM medicine WHERE id = ?");
    query.addBindValue(medicineId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取药品信息失败";
        return false;
    }

    QString medicineName = query.value("name").toString();
    double unitPrice = query.value("price").toDouble();
    int stock = query.value("stock").toInt();
    double totalPrice = unitPrice * quantity;

    // 检查库存
    if (stock < quantity) {
        qDebug() << "药品库存不足：" << medicineName << "，库存：" << stock << "，需求：" << quantity;
        return false;
    }

    // 插入处方详情
    QString detailId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    query.prepare("INSERT INTO prescription_detail (id, prescription_id, medicine_id, quantity, dosage, unit_price, total_price) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(detailId);
    query.addBindValue(prescriptionId);
    query.addBindValue(medicineId);
    query.addBindValue(quantity);
    query.addBindValue(dosage);
    query.addBindValue(unitPrice);
    query.addBindValue(totalPrice);

    if (!query.exec()) {
        qDebug() << "添加处方详情失败：" << query.lastError();
        return false;
    }

    // 更新处方总金额
    query.prepare("UPDATE prescription SET total_amount = total_amount + ? WHERE id = ?");
    query.addBindValue(totalPrice);
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "更新处方总金额失败：" << query.lastError();
        return false;
    }

    qDebug() << "添加处方详情成功：" << medicineName << "，数量：" << quantity
             << "，金额：" << totalPrice;
    return true;
}

bool IDatabase::deletePrescriptionDetail(const QString &detailId)
{
    // 先获取详情信息
    QSqlQuery query;
    query.prepare("SELECT prescription_id, total_price FROM prescription_detail WHERE id = ?");
    query.addBindValue(detailId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取处方详情失败";
        return false;
    }

    QString prescriptionId = query.value("prescription_id").toString();
    double totalPrice = query.value("total_price").toDouble();

    // 删除详情
    query.prepare("DELETE FROM prescription_detail WHERE id = ?");
    query.addBindValue(detailId);

    if (!query.exec()) {
        qDebug() << "删除处方详情失败：" << query.lastError();
        return false;
    }

    // 更新处方总金额
    query.prepare("UPDATE prescription SET total_amount = total_amount - ? WHERE id = ?");
    query.addBindValue(totalPrice);
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "更新处方总金额失败：" << query.lastError();
        return false;
    }

    qDebug() << "删除处方详情成功，ID：" << detailId;
    return true;
}

bool IDatabase::dispensePrescription(const QString &prescriptionId)
{
    // 检查处方状态
    QSqlQuery query;
    query.prepare("SELECT dispensing_status, total_amount FROM prescription WHERE id = ?");
    query.addBindValue(prescriptionId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取处方信息失败";
        return false;
    }

    QString dispensingStatus = query.value("dispensing_status").toString();
    double totalAmount = query.value("total_amount").toDouble();

    if (dispensingStatus == "dispensed") {
        qDebug() << "处方已发药";
        return false;
    }

    if (totalAmount <= 0) {
        qDebug() << "处方金额为0，无需发药";
        return false;
    }

    // 获取处方详情
    query.prepare("SELECT id, medicine_id, quantity FROM prescription_detail WHERE prescription_id = ?");
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "获取处方详情失败";
        return false;
    }

    bool allSuccess = true;
    QStringList errors;

    while (query.next()) {
        QString detailId = query.value("id").toString();
        QString medicineId = query.value("medicine_id").toString();
        int quantity = query.value("quantity").toInt();

        // 检查库存并发药
        QSqlQuery medicineQuery;
        medicineQuery.prepare("SELECT stock, name FROM medicine WHERE id = ?");
        medicineQuery.addBindValue(medicineId);

        if (medicineQuery.exec() && medicineQuery.next()) {
            int stock = medicineQuery.value("stock").toInt();
            QString medicineName = medicineQuery.value("name").toString();

            if (stock >= quantity) {
                // 更新药品库存
                medicineQuery.prepare("UPDATE medicine SET stock = stock - ? WHERE id = ?");
                medicineQuery.addBindValue(quantity);
                medicineQuery.addBindValue(medicineId);

                if (!medicineQuery.exec()) {
                    errors.append(QString("%1: 更新库存失败").arg(medicineName));
                    allSuccess = false;
                    continue;
                }

                // 更新处方详情发药状态
                medicineQuery.prepare("UPDATE prescription_detail SET dispensing_quantity = ?, dispensing_time = ?, dispensing_by = ? WHERE id = ?");
                medicineQuery.addBindValue(quantity);
                medicineQuery.addBindValue(QDateTime::currentDateTime());
                medicineQuery.addBindValue("system");
                medicineQuery.addBindValue(detailId);

                if (!medicineQuery.exec()) {
                    errors.append(QString("%1: 更新发药状态失败").arg(medicineName));
                    allSuccess = false;
                }
            } else {
                errors.append(QString("%1: 库存不足（库存:%2, 需求:%3）").arg(medicineName).arg(stock).arg(quantity));
                allSuccess = false;
            }
        }
    }

    if (allSuccess) {
        // 更新处方发药状态
        query.prepare("UPDATE prescription SET dispensing_status = 'dispensed' WHERE id = ?");
        query.addBindValue(prescriptionId);

        if (!query.exec()) {
            qDebug() << "更新处方发药状态失败：" << query.lastError();
            return false;
        }

        qDebug() << "处方发药成功：" << prescriptionId;
        return true;
    } else {
        qDebug() << "发药失败，错误：" << errors.join(", ");
        return false;
    }
}

bool IDatabase::processPayment(const QString &prescriptionId, const QString &paymentMethod)
{
    // 检查处方状态
    QSqlQuery query;
    query.prepare("SELECT payment_status, total_amount, dispensing_status FROM prescription WHERE id = ?");
    query.addBindValue(prescriptionId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取处方信息失败";
        return false;
    }

    QString paymentStatus = query.value("payment_status").toString();
    double totalAmount = query.value("total_amount").toDouble();
    QString dispensingStatus = query.value("dispensing_status").toString();

    if (paymentStatus == "paid") {
        qDebug() << "处方已收费";
        return false;
    }

    if (totalAmount <= 0) {
        qDebug() << "处方金额为0，无需收费";
        return false;
    }

    // 更新处方支付状态
    query.prepare("UPDATE prescription SET payment_status = 'paid', payment_method = ? WHERE id = ?");
    query.addBindValue(paymentMethod);
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "更新处方支付状态失败：" << query.lastError();
        return false;
    }

    // 创建费用记录
    QString feeId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    query.prepare("INSERT INTO fee (id, patient_id, prescription_id, fee_type, item_name, quantity, unit_price, total_amount, payment_method, payment_time, status) "
                  "SELECT ?, patient_id, ?, 'medicine', '处方费', 1, total_amount, total_amount, ?, ?, 'paid' "
                  "FROM prescription WHERE id = ?");
    query.addBindValue(feeId);
    query.addBindValue(prescriptionId);
    query.addBindValue(paymentMethod);
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "创建费用记录失败：" << query.lastError();
        return false;
    }

    qDebug() << "处方收费成功：" << prescriptionId << "，金额：" << totalAmount;
    return true;
}

bool IDatabase::auditPrescription(const QString &prescriptionId)
{
    QSqlQuery query;
    query.prepare("UPDATE prescription SET status = 'audited' WHERE id = ?");
    query.addBindValue(prescriptionId);

    if (!query.exec()) {
        qDebug() << "审核处方失败：" << query.lastError();
        return false;
    }

    qDebug() << "处方审核成功：" << prescriptionId;
    return true;
}

QMap<QString, QVariant> IDatabase::getTodayPrescriptionStats()
{
    QMap<QString, QVariant> stats;

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count, SUM(total_amount) as amount FROM prescription WHERE DATE(prescription_date) = DATE('now')");

    if (query.exec() && query.next()) {
        stats["today_count"] = query.value("count").toInt();
        stats["today_amount"] = query.value("amount").toDouble();
    } else {
        stats["today_count"] = 0;
        stats["today_amount"] = 0.0;
    }

    return stats;
}

QMap<QString, QVariant> IDatabase::getMonthPrescriptionStats()
{
    QMap<QString, QVariant> stats;

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count, SUM(total_amount) as amount FROM prescription WHERE strftime('%Y-%m', prescription_date) = strftime('%Y-%m', 'now')");

    if (query.exec() && query.next()) {
        stats["month_count"] = query.value("count").toInt();
        stats["month_amount"] = query.value("amount").toDouble();
    } else {
        stats["month_count"] = 0;
        stats["month_amount"] = 0.0;
    }

    return stats;
}

int IDatabase::getPendingDispenseCount()
{
    int count = 0;
    QSqlQuery query("SELECT COUNT(*) FROM prescription WHERE dispensing_status = 'pending' AND payment_status = 'paid'");

    if (query.exec() && query.next()) {
        count = query.value(0).toInt();
    }

    return count;
}

int IDatabase::getUnpaidCount()
{
    int count = 0;
    QSqlQuery query("SELECT COUNT(*) FROM prescription WHERE payment_status = 'unpaid'");

    if (query.exec() && query.next()) {
        count = query.value(0).toInt();
    }

    return count;
}

QList<QString> IDatabase::getDoctorsForCombo()
{
    QList<QString> doctors;

    // 如果doctor表不存在，返回测试数据
    QSqlQuery checkTable("SELECT name FROM sqlite_master WHERE type='table' AND name='doctor'");
    if (!checkTable.next()) {
        // 表不存在，返回模拟数据
        doctors << "张医生 (d001)" << "李医生 (d002)" << "王医生 (d003)";
        return doctors;
    }

    QSqlQuery query("SELECT id, name FROM doctor WHERE status = 'active' ORDER BY name");

    while (query.next()) {
        QString displayText = QString("%1 (%2)")
        .arg(query.value("name").toString())
            .arg(query.value("id").toString());
        doctors.append(displayText);
    }

    // 如果没有数据，添加默认选项
    if (doctors.isEmpty()) {
        doctors << "未指定医生";
    }

    return doctors;
}
// 预约管理方法实现
bool IDatabase::initAppointmentModel()
{
    appointmentTabModel = new QSqlTableModel(this, database);
    appointmentTabModel->setTable("appointment");
    appointmentTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("patient_id"), Qt::Horizontal, "患者");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("doctor_id"), Qt::Horizontal, "医生");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("appointment_date"), Qt::Horizontal, "预约日期");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("appointment_time"), Qt::Horizontal, "预约时间");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("department_id"), Qt::Horizontal, "科室");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("status"), Qt::Horizontal, "状态");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("reason"), Qt::Horizontal, "事由");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("notes"), Qt::Horizontal, "备注");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("check_in_time"), Qt::Horizontal, "到诊时间");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("check_out_time"), Qt::Horizontal, "离开时间");
    appointmentTabModel->setHeaderData(appointmentTabModel->fieldIndex("created_time"), Qt::Horizontal, "创建时间");

    // 按预约日期和时间排序（最新的在前）
    appointmentTabModel->setSort(appointmentTabModel->fieldIndex("appointment_date"), Qt::DescendingOrder);
    appointmentTabModel->setSort(appointmentTabModel->fieldIndex("appointment_time"), Qt::DescendingOrder);

    if (!appointmentTabModel->select()) {
        qDebug() << "初始化预约模型失败：" << appointmentTabModel->lastError();
        return false;
    }

    theAppointmentSelection = new QItemSelectionModel(appointmentTabModel);
    qDebug() << "预约模型初始化成功，记录数：" << appointmentTabModel->rowCount();
    return true;
}

int IDatabase::addNewAppointment()
{
    // 生成预约号（规则：APT + 年月日 + 4位随机数）
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    QString randomStr = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    QString appointmentNumber = "APT" + dateStr + randomStr;

    appointmentTabModel->insertRow(appointmentTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = appointmentTabModel->index(appointmentTabModel->rowCount() - 1, 0);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = appointmentTabModel->record(curRecNo);

    // 设置默认值
    QString appointmentId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    curRec.setValue("id", appointmentId);
    curRec.setValue("appointment_number", appointmentNumber);
    curRec.setValue("appointment_date", QDate::currentDate());
    curRec.setValue("time_slot", QTime::currentTime().toString("HH:mm"));
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("updated_time", QDateTime::currentDateTime());
    curRec.setValue("status", "scheduled");
    curRec.setValue("priority", 0);
    curRec.setValue("reason", "");
    curRec.setValue("symptoms", "");
    curRec.setValue("notes", "");

    // patient_id 和 doctor_id 留空，让用户选择

    qDebug() << "新增预约，ID：" << appointmentId
             << "，预约号：" << appointmentNumber;

    appointmentTabModel->setRecord(curRecNo, curRec);
    return curRecNo;
}

bool IDatabase::searchAppointment(const QString &filter)
{
    if (filter.isEmpty()) {
        appointmentTabModel->setFilter("");
    } else {
        // 构建复杂的查询条件，关联患者和医生表
        QString whereClause = QString(
                                  "appointment_number LIKE '%%1%' OR "
                                  "id IN (SELECT id FROM appointment WHERE "
                                  "patient_id IN (SELECT ID FROM patient WHERE name LIKE '%%1%') OR "
                                  "doctor_id IN (SELECT id FROM doctor WHERE name LIKE '%%1%') OR "
                                  "reason LIKE '%%1%')")
                                  .arg(filter);
        appointmentTabModel->setFilter(whereClause);
    }

    bool success = appointmentTabModel->select();
    if (!success) {
        qDebug() << "搜索预约失败：" << appointmentTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentAppointment()
{
    QModelIndex curIndex = theAppointmentSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择预约";
        return false;
    }

    // 获取预约信息
    QString appointmentId = appointmentTabModel->record(curIndex.row()).value("id").toString();
    QString appointmentNumber = appointmentTabModel->record(curIndex.row()).value("appointment_number").toString();
    QString status = appointmentTabModel->record(curIndex.row()).value("status").toString();

    // 检查预约状态，已确认或已完成的预约不能删除
    if (status == "confirmed" || status == "completed") {
        qDebug() << "无法删除预约：" << appointmentNumber << "，状态为" << status;
        return false;
    }

    // 执行删除
    if (appointmentTabModel->removeRow(curIndex.row())) {
        bool success = appointmentTabModel->submitAll();
        if (success) {
            appointmentTabModel->select(); // 刷新数据
            qDebug() << "删除预约成功：" << appointmentNumber;
        } else {
            qDebug() << "删除预约失败：" << appointmentTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitAppointmentEdit()
{
    bool success = appointmentTabModel->submitAll();
    if (!success) {
        qDebug() << "提交预约编辑失败：" << appointmentTabModel->lastError();
    }
    return success;
}

void IDatabase::revertAppointmentEdit()
{
    appointmentTabModel->revertAll();
    qDebug() << "撤销预约编辑";
}

QStringList IDatabase::getAppointmentStatuses()
{
    return QStringList() << "scheduled" << "confirmed" << "checked_in"
                         << "completed" << "cancelled" << "no_show";
}

QMap<QString, QVariant> IDatabase::getTodayAppointmentStats()
{
    QMap<QString, QVariant> stats;

    // 获取今日预约统计
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM appointment WHERE DATE(appointment_date) = DATE('now')");

    if (query.exec() && query.next()) {
        stats["today_count"] = query.value("count").toInt();
    } else {
        stats["today_count"] = 0;
    }

    return stats;
}

QMap<QString, QVariant> IDatabase::getTomorrowAppointmentStats()
{
    QMap<QString, QVariant> stats;

    // 获取明日预约统计
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM appointment WHERE DATE(appointment_date) = DATE('now', '+1 day')");

    if (query.exec() && query.next()) {
        stats["tomorrow_count"] = query.value("count").toInt();
    } else {
        stats["tomorrow_count"] = 0;
    }

    return stats;
}

QMap<QString, QVariant> IDatabase::getAppointmentSummaryStats()
{
    QMap<QString, QVariant> stats;

    QSqlQuery query;
    query.prepare("SELECT "
                  "COUNT(*) as total, "
                  "SUM(CASE WHEN status = 'scheduled' THEN 1 ELSE 0 END) as scheduled, "
                  "SUM(CASE WHEN status = 'confirmed' THEN 1 ELSE 0 END) as confirmed, "
                  "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed, "
                  "SUM(CASE WHEN status = 'cancelled' THEN 1 ELSE 0 END) as cancelled, "
                  "SUM(CASE WHEN status = 'no_show' THEN 1 ELSE 0 END) as no_show "
                  "FROM appointment");

    if (query.exec() && query.next()) {
        stats["total"] = query.value("total").toInt();
        stats["scheduled"] = query.value("scheduled").toInt();
        stats["confirmed"] = query.value("confirmed").toInt();
        stats["completed"] = query.value("completed").toInt();
        stats["cancelled"] = query.value("cancelled").toInt();
        stats["no_show"] = query.value("no_show").toInt();
    } else {
        stats["total"] = 0;
        stats["scheduled"] = 0;
        stats["confirmed"] = 0;
        stats["completed"] = 0;
        stats["cancelled"] = 0;
        stats["no_show"] = 0;
    }

    return stats;
}

bool IDatabase::updateAppointmentStatus(const QString &appointmentId, const QString &newStatus)
{
    if (appointmentId.isEmpty() || newStatus.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE appointment SET status = ? WHERE id = ?");
    query.addBindValue(newStatus);
    query.addBindValue(appointmentId);

    if (!query.exec()) {
        qDebug() << "更新预约状态失败：" << query.lastError();
        return false;
    }

    // 如果是到诊状态，记录到诊时间
    if (newStatus == "checked_in") {
        query.prepare("UPDATE appointment SET check_in_time = ? WHERE id = ?");
        query.addBindValue(QDateTime::currentDateTime());
        query.addBindValue(appointmentId);
        query.exec();
    }
    // 如果是完成状态，记录离开时间
    else if (newStatus == "completed") {
        query.prepare("UPDATE appointment SET check_out_time = ?, status = ? WHERE id = ?");
        query.addBindValue(QDateTime::currentDateTime());
        query.addBindValue("completed");
        query.addBindValue(appointmentId);
        query.exec();
    }

    qDebug() << "更新预约状态成功，ID：" << appointmentId << "，新状态：" << newStatus;
    return true;
}

QList<QString> IDatabase::getPatientsForCombo()
{
    QList<QString> patients;

    // 如果patient表不存在，返回测试数据
    QSqlQuery checkTable("SELECT name FROM sqlite_master WHERE type='table' AND name='patient'");
    if (!checkTable.next()) {
        // 表不存在，返回模拟数据
        patients << "张三 (p001)" << "李四 (p002)" << "王五 (p003)"
                 << "赵六 (p004)" << "钱七 (p005)" << "孙八 (p006)";
        return patients;
    }

    QSqlQuery query("SELECT ID, name FROM patient ORDER BY name");

    while (query.next()) {
        QString displayText = QString("%1 (%2)")
        .arg(query.value("name").toString())
            .arg(query.value("ID").toString());
        patients.append(displayText);
    }

    // 如果没有数据，添加默认选项
    if (patients.isEmpty()) {
        patients << "未选择患者";
    }

    return patients;
}

QList<QString> IDatabase::getDoctorsForAppointmentCombo()
{
    QList<QString> doctors;

    // 如果doctor表不存在，返回测试数据
    QSqlQuery checkTable("SELECT name FROM sqlite_master WHERE type='table' AND name='doctor'");
    if (!checkTable.next()) {
        // 表不存在，返回模拟数据
        doctors << "张医生 (d001)" << "李医生 (d002)" << "王医生 (d003)";
        return doctors;
    }

    QSqlQuery query("SELECT id, name FROM doctor WHERE status = 'active' ORDER BY name");

    while (query.next()) {
        QString displayText = QString("%1 (%2)")
        .arg(query.value("name").toString())
            .arg(query.value("id").toString());
        doctors.append(displayText);
    }

    // 如果没有数据，添加默认选项
    if (doctors.isEmpty()) {
        doctors << "未指定医生";
    }

    return doctors;
}

bool IDatabase::checkTimeConflict(const QString &doctorId, const QDateTime &appointmentTime,
                                  const QString &excludeAppointmentId)
{
    if (doctorId.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    QString queryStr = "SELECT COUNT(*) FROM appointment WHERE doctor_id = ? AND "
                       "appointment_date = ? AND appointment_time BETWEEN ? AND ? "
                       "AND status NOT IN ('cancelled', 'no_show')";

    if (!excludeAppointmentId.isEmpty()) {
        queryStr += " AND id != ?";
    }

    query.prepare(queryStr);
    query.addBindValue(doctorId);
    query.addBindValue(appointmentTime.date());

    // 检查前后30分钟的时间段
    QTime startTime = appointmentTime.time().addSecs(-30 * 60);
    QTime endTime = appointmentTime.time().addSecs(30 * 60);
    query.addBindValue(startTime);
    query.addBindValue(endTime);

    if (!excludeAppointmentId.isEmpty()) {
        query.addBindValue(excludeAppointmentId);
    }

    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
}
// idatabase.cpp - 在文件末尾添加以下方法实现

// 就诊记录管理方法实现
bool IDatabase::initConsultRecordModel()
{
    consultRecordTabModel = new QSqlTableModel(this, database);
    consultRecordTabModel->setTable("consult_record");
    consultRecordTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 设置表头显示名称 - 将 "consult_date" 改为 "visit_date"
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("patient_id"), Qt::Horizontal, "患者");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("doctor_id"), Qt::Horizontal, "医生");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("visit_date"), Qt::Horizontal, "就诊日期");  // 改为 visit_date
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("department_id"), Qt::Horizontal, "科室");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("diagnosis"), Qt::Horizontal, "诊断");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("symptoms"), Qt::Horizontal, "症状");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("treatment"), Qt::Horizontal, "治疗方案");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("prescription_id"), Qt::Horizontal, "处方");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("consult_fee"), Qt::Horizontal, "诊费");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("notes"), Qt::Horizontal, "备注");
    consultRecordTabModel->setHeaderData(consultRecordTabModel->fieldIndex("next_visit_date"), Qt::Horizontal, "复诊日期");

    // 按就诊日期倒序排序（最新的在前）- 改为 visit_date
    consultRecordTabModel->setSort(consultRecordTabModel->fieldIndex("visit_date"), Qt::DescendingOrder);

    if (!consultRecordTabModel->select()) {
        qDebug() << "初始化就诊记录模型失败：" << consultRecordTabModel->lastError();
        return false;
    }

    theConsultRecordSelection = new QItemSelectionModel(consultRecordTabModel);
    qDebug() << "就诊记录模型初始化成功，记录数：" << consultRecordTabModel->rowCount();
    return true;
}

int IDatabase::addNewConsultRecord()
{
    consultRecordTabModel->insertRow(consultRecordTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = consultRecordTabModel->index(consultRecordTabModel->rowCount() - 1, 0);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = consultRecordTabModel->record(curRecNo);

    // 设置默认值 - 将 "consult_date" 改为 "visit_date"
    curRec.setValue("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    curRec.setValue("visit_date", QDateTime::currentDateTime());  // 改为 visit_date
    curRec.setValue("consult_fee", 0.0);
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("status", "active");

    consultRecordTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增就诊记录，ID：" << curRec.value("id").toString();
    return curRecNo;
}

bool IDatabase::searchConsultRecord(const QString &filter)
{
    if (filter.isEmpty()) {
        consultRecordTabModel->setFilter("");
    } else {
        // 构建复杂的查询条件，关联患者和医生表
        QString whereClause = QString(
                                  "diagnosis LIKE '%%1%' OR symptoms LIKE '%%1%' OR treatment LIKE '%%1%' OR "
                                  "id IN (SELECT id FROM consult_record WHERE "
                                  "patient_id IN (SELECT ID FROM patient WHERE name LIKE '%%1%') OR "
                                  "doctor_id IN (SELECT id FROM doctor WHERE name LIKE '%%1%'))")
                                  .arg(filter);
        consultRecordTabModel->setFilter(whereClause);
    }

    bool success = consultRecordTabModel->select();
    if (!success) {
        qDebug() << "搜索就诊记录失败：" << consultRecordTabModel->lastError();
    }
    return success;
}

bool IDatabase::deleteCurrentConsultRecord()
{
    QModelIndex curIndex = theConsultRecordSelection->currentIndex();
    if (!curIndex.isValid()) {
        qDebug() << "删除失败：未选择就诊记录";
        return false;
    }

    // 获取就诊记录信息
    QString recordId = consultRecordTabModel->record(curIndex.row()).value("id").toString();
    QString patientName = consultRecordTabModel->record(curIndex.row()).value("patient_id").toString();

    // 检查是否有关联的处方
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM prescription WHERE consult_record_id = ?");
    query.addBindValue(recordId);

    if (query.exec() && query.next()) {
        int prescriptionCount = query.value(0).toInt();
        if (prescriptionCount > 0) {
            qDebug() << "无法删除就诊记录，有处方关联";
            return false;
        }
    }

    // 执行删除
    if (consultRecordTabModel->removeRow(curIndex.row())) {
        bool success = consultRecordTabModel->submitAll();
        if (success) {
            consultRecordTabModel->select(); // 刷新数据
            qDebug() << "删除就诊记录成功，患者：" << patientName;
        } else {
            qDebug() << "删除就诊记录失败：" << consultRecordTabModel->lastError();
        }
        return success;
    }

    return false;
}

bool IDatabase::submitConsultRecordEdit()
{
    bool success = consultRecordTabModel->submitAll();
    if (!success) {
        qDebug() << "提交就诊记录编辑失败：" << consultRecordTabModel->lastError();
    }
    return success;
}

void IDatabase::revertConsultRecordEdit()
{
    consultRecordTabModel->revertAll();
    qDebug() << "撤销就诊记录编辑";
}

int IDatabase::getTodayConsultCount()
{
    int count = 0;
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM consult_record WHERE DATE(visit_date) = DATE('now')");  // 改为 visit_date

    if (query.exec() && query.next()) {
        count = query.value(0).toInt();
    }

    return count;
}

int IDatabase::getMonthConsultCount()
{
    int count = 0;
    QSqlQuery query;
   query.prepare("SELECT COUNT(*) FROM consult_record WHERE strftime('%Y-%m', visit_date) = strftime('%Y-%m', 'now')");  // 改为 visit_date

    if (query.exec() && query.next()) {
        count = query.value(0).toInt();
    }

    return count;
}

QMap<QString, QVariant> IDatabase::getConsultSummaryStats()
{
    QMap<QString, QVariant> stats;

    // 今日诊费总额
    QSqlQuery todayFeeQuery;
    todayFeeQuery.prepare("SELECT SUM(consult_fee) as total_fee FROM consult_record WHERE DATE(visit_date) = DATE('now')");
    if (todayFeeQuery.exec() && todayFeeQuery.next()) {
        stats["today_fee"] = todayFeeQuery.value("total_fee").toDouble();
    }

    // 本月诊费总额
    QSqlQuery monthFeeQuery;
    monthFeeQuery.prepare("SELECT SUM(consult_fee) as total_fee FROM consult_record WHERE strftime('%Y-%m', visit_date) = strftime('%Y-%m', 'now')");
    if (monthFeeQuery.exec() && monthFeeQuery.next()) {
        stats["month_fee"] = monthFeeQuery.value("total_fee").toDouble();
    }

    // 各科室就诊量
    QSqlQuery deptQuery;
    deptQuery.prepare("SELECT d.name as department, COUNT(*) as count "
                      "FROM consult_record cr "
                      "LEFT JOIN department d ON cr.department_id = d.id "
                      "WHERE DATE(cr.visit_date) = DATE('now') "  // 改为 visit_date
                      "GROUP BY cr.department_id "
                      "ORDER BY count DESC LIMIT 5");

    QStringList topDepartments;
    if (deptQuery.exec()) {
        while (deptQuery.next()) {
            topDepartments.append(QString("%1: %2人次").arg(deptQuery.value("department").toString()).arg(deptQuery.value("count").toInt()));
        }
        stats["top_departments"] = topDepartments.join(" | ");
    }

    return stats;
}

QList<QString> IDatabase::getCommonDiagnoses(int limit)
{
    QList<QString> commonDiagnoses;
    QSqlQuery query;
    query.prepare("SELECT diagnosis, COUNT(*) as count FROM consult_record "
                  "WHERE diagnosis IS NOT NULL AND diagnosis != '' "
                  "GROUP BY diagnosis ORDER BY count DESC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            commonDiagnoses.append(QString("%1 (%2次)").arg(query.value("diagnosis").toString()).arg(query.value("count").toInt()));
        }
    }

    // 如果没有数据，添加默认信息
    if (commonDiagnoses.isEmpty()) {
        commonDiagnoses.append("暂无数据");
    }

    return commonDiagnoses;
}
