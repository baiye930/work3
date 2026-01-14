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
    // 获取当前最大工号
    QSqlQuery query("SELECT MAX(employee_id) FROM doctor");
    int nextId = 10001; // 初始值
    if (query.exec() && query.next()) {
        QString maxId = query.value(0).toString();
        if (!maxId.isEmpty()) {
            nextId = maxId.toInt() + 1;
        }
    }

    doctorTabModel->insertRow(doctorTabModel->rowCount(), QModelIndex());

    QModelIndex curIndex = doctorTabModel->index(doctorTabModel->rowCount() - 1, 1);
    int curRecNo = curIndex.row();
    QSqlRecord curRec = doctorTabModel->record(curRecNo);

    // 设置默认值
    curRec.setValue("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    curRec.setValue("employee_id", QString::number(nextId));
    curRec.setValue("created_time", QDateTime::currentDateTime());
    curRec.setValue("status", "active");
    curRec.setValue("title", "主治医师"); // 默认职称
    curRec.setValue("gender", "男");      // 默认性别
    curRec.setValue("work_years", 0);
    curRec.setValue("consultation_fee", 15.0);

    doctorTabModel->setRecord(curRecNo, curRec);

    qDebug() << "新增医生，ID：" << curRec.value("id").toString() << "，工号：" << curRec.value("employee_id").toString();
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
