#include "statisticsworker.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

StatisticsWorker::StatisticsWorker(QObject *parent)
    : QObject(parent)
    , workerThread(nullptr)
    , isRunning(false)
    , isPaused(false)
    , checkInterval(30) // 默认30分钟检查一次
    , enableAutoReports(true)
    , dailyReportTime(QTime(23, 0)) // 默认晚上11点生成日报
{
    // 设置数据库路径（与主程序相同）
    QString appDir = QCoreApplication::applicationDirPath();
    dbPath = appDir + "/statistics.db";

    // 创建工作线程
    workerThread = new QThread();
    this->moveToThread(workerThread);
    connect(workerThread, &QThread::started, this, &StatisticsWorker::startWork);
    connect(workerThread, &QThread::finished, this, &StatisticsWorker::deleteLater);
}

StatisticsWorker::~StatisticsWorker()
{
    stopWork();
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
    cleanupDatabase();
}

void StatisticsWorker::startWork()
{
    qDebug() << "统计工作线程开始运行";

    isRunning = true;
    initDatabase();

    // 检查数据库连接是否成功
    if (!db.isOpen()) {
        emit errorOccurred("无法连接到统计数据库");
        return;
    }

    emit workStatusChanged("统计服务已启动");

    // 主工作循环
    while (isRunning) {
        QMutexLocker locker(&mutex);

        if (isPaused) {
            waitCondition.wait(&mutex);
        }

        if (!isRunning) break;

        try {
            // 执行各种统计任务
            emit workStatusChanged("开始数据统计...");

            // 1. 检查预警
            checkLowStock();
            checkNearExpiry();
            checkPendingAppointments();
            checkOverduePrescriptions();

            // 2. 生成统计报告
            QTime currentTime = QTime::currentTime();
            QDate currentDate = QDate::currentDate();

            // 每天生成一次日报
            if (enableAutoReports && currentTime >= dailyReportTime) {
                QVariantMap dailyStats = generateDailyStatistics();
                emit dailyStatsReady(dailyStats);
                emit statisticsGenerated("daily", "每日统计报告已生成");
            }

            // 每周一生成周报
            if (enableAutoReports && currentDate.dayOfWeek() == 1) {
                QVariantMap weeklyStats = generateWeeklyStatistics();
                emit weeklyStatsReady(weeklyStats);
                emit statisticsGenerated("weekly", "每周统计报告已生成");
            }

            // 每月1号生成月报
            if (enableAutoReports && currentDate.day() == 1) {
                QVariantMap monthlyStats = generateMonthlyStatistics();
                emit monthlyStatsReady(monthlyStats);
                emit statisticsGenerated("monthly", "每月统计报告已生成");
            }

            // 3. 系统健康检查
            performSystemHealthCheck();

            emit workStatusChanged("数据统计完成");

        } catch (const std::exception &e) {
            emit errorOccurred(QString("统计任务执行异常: %1").arg(e.what()));
        } catch (...) {
            emit errorOccurred("统计任务执行时发生未知异常");
        }

        // 等待指定间隔
        for (int i = 0; i < checkInterval * 60 && isRunning; i += 5) {
            if (isPaused) {
                waitCondition.wait(&mutex);
            }
            QThread::sleep(5);
        }
    }

    emit workStatusChanged("统计服务已停止");
}

void StatisticsWorker::stopWork()
{
    isRunning = false;
    isPaused = false;
    waitCondition.wakeAll();
    emit workStatusChanged("正在停止统计服务...");
}

void StatisticsWorker::pauseWork()
{
    isPaused = true;
    emit workStatusChanged("统计服务已暂停");
}

void StatisticsWorker::resumeWork()
{
    isPaused = false;
    waitCondition.wakeAll();
    emit workStatusChanged("统计服务已恢复");
}

void StatisticsWorker::initDatabase()
{
    // 创建独立的数据库连接用于统计
    if (QSqlDatabase::contains("statistics_connection")) {
        db = QSqlDatabase::database("statistics_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "statistics_connection");
        db.setDatabaseName(dbPath);
    }

    if (!db.open()) {
        emit errorOccurred(QString("无法打开统计数据库: %1").arg(db.lastError().text()));
        return;
    }

    // 创建统计相关表
    QSqlQuery query(db);

    // 创建统计结果表
    QString createStatsTable =
        "CREATE TABLE IF NOT EXISTS statistics_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "stat_type TEXT NOT NULL,"
        "period_start DATE NOT NULL,"
        "period_end DATE NOT NULL,"
        "stats_data TEXT NOT NULL,"
        "generated_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "notes TEXT)";

    if (!query.exec(createStatsTable)) {
        emit errorOccurred(QString("创建统计历史表失败: %1").arg(query.lastError().text()));
    }

    // 创建预警记录表
    QString createWarningsTable =
        "CREATE TABLE IF NOT EXISTS warnings_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "warning_type TEXT NOT NULL,"
        "warning_level TEXT NOT NULL,"
        "message TEXT NOT NULL,"
        "data_json TEXT,"
        "created_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "is_resolved BOOLEAN DEFAULT 0,"
        "resolved_time DATETIME,"
        "resolved_by TEXT)";

    if (!query.exec(createWarningsTable)) {
        emit errorOccurred(QString("创建预警记录表失败: %1").arg(query.lastError().text()));
    }
}

void StatisticsWorker::cleanupDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}

QVariantMap StatisticsWorker::generateDailyStatistics()
{
    QVariantMap stats;
    QDate today = QDate::currentDate();

    try {
        // 1. 预约统计
        QSqlQuery query(db);
        query.prepare(
            "SELECT "
            "COUNT(*) as total, "
            "SUM(CASE WHEN status = 'scheduled' THEN 1 ELSE 0 END) as scheduled, "
            "SUM(CASE WHEN status = 'confirmed' THEN 1 ELSE 0 END) as confirmed, "
            "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed, "
            "SUM(CASE WHEN status = 'cancelled' THEN 1 ELSE 0 END) as cancelled, "
            "SUM(CASE WHEN status = 'no_show' THEN 1 ELSE 0 END) as no_show "
            "FROM appointment WHERE DATE(appointment_date) = ?"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["appointments_total"] = query.value("total").toInt();
            stats["appointments_scheduled"] = query.value("scheduled").toInt();
            stats["appointments_confirmed"] = query.value("confirmed").toInt();
            stats["appointments_completed"] = query.value("completed").toInt();
            stats["appointments_cancelled"] = query.value("cancelled").toInt();
            stats["appointments_no_show"] = query.value("no_show").toInt();
        }

        // 2. 就诊统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(consult_fee) as total_fee "
            "FROM consult_record WHERE DATE(visit_date) = ?"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["consults_count"] = query.value("count").toInt();
            stats["consults_revenue"] = query.value("total_fee").toDouble();
        }

        // 3. 处方统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(total_amount) as total_amount "
            "FROM prescription WHERE DATE(prescription_date) = ? AND payment_status = 'paid'"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["prescriptions_count"] = query.value("count").toInt();
            stats["prescriptions_revenue"] = query.value("total_amount").toDouble();
        }

        // 4. 新增患者统计
        query.prepare(
            "SELECT COUNT(*) as count "
            "FROM patient WHERE DATE(CREATEDTIMESTAMP) = ?"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["new_patients"] = query.value("count").toInt();
        }

        // 5. 计算总收入
        double totalRevenue = stats["consults_revenue"].toDouble() +
                              stats["prescriptions_revenue"].toDouble();
        stats["daily_revenue"] = totalRevenue;

        // 6. 保存统计结果到数据库
        QJsonObject statsJson = QJsonObject::fromVariantMap(stats);
        QJsonDocument doc(statsJson);
        QString statsData = doc.toJson(QJsonDocument::Compact);

        query.prepare(
            "INSERT INTO statistics_history (stat_type, period_start, period_end, stats_data) "
            "VALUES ('daily', ?, ?, ?)"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));
        query.addBindValue(statsData);

        query.exec();

    } catch (const std::exception &e) {
        emit errorOccurred(QString("生成日报统计失败: %1").arg(e.what()));
    }

    return stats;
}

QVariantMap StatisticsWorker::generateWeeklyStatistics()
{
    QVariantMap stats;
    QDate today = QDate::currentDate();
    QDate weekStart = today.addDays(-today.dayOfWeek() + 1); // 本周一

    try {
        // 1. 预约统计（本周）
        QSqlQuery query(db);
        query.prepare(
            "SELECT "
            "COUNT(*) as total, "
            "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed "
            "FROM appointment "
            "WHERE appointment_date BETWEEN ? AND ?"
            );
        query.addBindValue(weekStart.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["weekly_appointments_total"] = query.value("total").toInt();
            stats["weekly_appointments_completed"] = query.value("completed").toInt();
        }

        // 2. 就诊统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(consult_fee) as total_fee "
            "FROM consult_record "
            "WHERE DATE(visit_date) BETWEEN ? AND ?"
            );
        query.addBindValue(weekStart.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["weekly_consults_count"] = query.value("count").toInt();
            stats["weekly_consults_revenue"] = query.value("total_fee").toDouble();
        }

        // 3. 处方统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(total_amount) as total_amount "
            "FROM prescription "
            "WHERE DATE(prescription_date) BETWEEN ? AND ? AND payment_status = 'paid'"
            );
        query.addBindValue(weekStart.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["weekly_prescriptions_count"] = query.value("count").toInt();
            stats["weekly_prescriptions_revenue"] = query.value("total_amount").toDouble();
        }

        // 4. 计算周总收入
        double weeklyRevenue = stats["weekly_consults_revenue"].toDouble() +
                               stats["weekly_prescriptions_revenue"].toDouble();
        stats["weekly_revenue"] = weeklyRevenue;

        // 5. 保存统计结果
        QJsonObject statsJson = QJsonObject::fromVariantMap(stats);
        QJsonDocument doc(statsJson);
        QString statsData = doc.toJson(QJsonDocument::Compact);

        query.prepare(
            "INSERT INTO statistics_history (stat_type, period_start, period_end, stats_data) "
            "VALUES ('weekly', ?, ?, ?)"
            );
        query.addBindValue(weekStart.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));
        query.addBindValue(statsData);

        query.exec();

    } catch (const std::exception &e) {
        emit errorOccurred(QString("生成周报统计失败: %1").arg(e.what()));
    }

    return stats;
}

QVariantMap StatisticsWorker::generateMonthlyStatistics()
{
    QVariantMap stats;
    QDate today = QDate::currentDate();
    QDate monthStart(today.year(), today.month(), 1);

    try {
        // 1. 预约统计（本月）
        QSqlQuery query(db);
        query.prepare(
            "SELECT "
            "COUNT(*) as total, "
            "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed "
            "FROM appointment "
            "WHERE strftime('%Y-%m', appointment_date) = strftime('%Y-%m', ?)"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["monthly_appointments_total"] = query.value("total").toInt();
            stats["monthly_appointments_completed"] = query.value("completed").toInt();
        }

        // 2. 就诊统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(consult_fee) as total_fee "
            "FROM consult_record "
            "WHERE strftime('%Y-%m', visit_date) = strftime('%Y-%m', ?)"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["monthly_consults_count"] = query.value("count").toInt();
            stats["monthly_consults_revenue"] = query.value("total_fee").toDouble();
        }

        // 3. 处方统计
        query.prepare(
            "SELECT COUNT(*) as count, SUM(total_amount) as total_amount "
            "FROM prescription "
            "WHERE strftime('%Y-%m', prescription_date) = strftime('%Y-%m', ?) "
            "AND payment_status = 'paid'"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["monthly_prescriptions_count"] = query.value("count").toInt();
            stats["monthly_prescriptions_revenue"] = query.value("total_amount").toDouble();
        }

        // 4. 新增患者统计
        query.prepare(
            "SELECT COUNT(*) as count "
            "FROM patient "
            "WHERE strftime('%Y-%m', CREATEDTIMESTAMP) = strftime('%Y-%m', ?)"
            );
        query.addBindValue(today.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            stats["monthly_new_patients"] = query.value("count").toInt();
        }

        // 5. 计算月总收入
        double monthlyRevenue = stats["monthly_consults_revenue"].toDouble() +
                                stats["monthly_prescriptions_revenue"].toDouble();
        stats["monthly_revenue"] = monthlyRevenue;

        // 6. 保存统计结果
        QJsonObject statsJson = QJsonObject::fromVariantMap(stats);
        QJsonDocument doc(statsJson);
        QString statsData = doc.toJson(QJsonDocument::Compact);

        query.prepare(
            "INSERT INTO statistics_history (stat_type, period_start, period_end, stats_data) "
            "VALUES ('monthly', ?, ?, ?)"
            );
        query.addBindValue(monthStart.toString("yyyy-MM-dd"));
        query.addBindValue(today.toString("yyyy-MM-dd"));
        query.addBindValue(statsData);

        query.exec();

    } catch (const std::exception &e) {
        emit errorOccurred(QString("生成月报统计失败: %1").arg(e.what()));
    }

    return stats;
}

void StatisticsWorker::checkLowStock()
{
    try {
        QSqlQuery query(db);
        query.prepare(
            "SELECT name, stock, min_stock "
            "FROM medicine "
            "WHERE stock <= min_stock AND status = 'active'"
            );

        if (query.exec()) {
            int warningCount = 0;
            while (query.next()) {
                QString medicineName = query.value("name").toString();
                int stock = query.value("stock").toInt();
                int minStock = query.value("min_stock").toInt();

                warningCount++;

                // 发送预警信号
                QJsonObject warningData;
                warningData["medicine_name"] = medicineName;
                warningData["current_stock"] = stock;
                warningData["min_stock"] = minStock;
                warningData["warning_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

                QJsonDocument doc(warningData);
                QString dataJson = doc.toJson(QJsonDocument::Compact);

                emit warningGenerated(
                    "low_stock",
                    QString("药品 %1 库存不足: 当前 %2，最低要求 %3")
                        .arg(medicineName).arg(stock).arg(minStock)
                    );

                // 记录到预警历史
                QSqlQuery insertQuery(db);
                insertQuery.prepare(
                    "INSERT INTO warnings_history (warning_type, warning_level, message, data_json) "
                    "VALUES ('low_stock', 'warning', ?, ?)"
                    );
                insertQuery.addBindValue(
                    QString("药品 %1 库存不足").arg(medicineName)
                    );
                insertQuery.addBindValue(dataJson);
                insertQuery.exec();
            }

            if (warningCount > 0) {
                emit statisticsGenerated("inventory",
                                         QString("发现 %1 种药品库存不足").arg(warningCount));
            }
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("检查低库存失败: %1").arg(e.what()));
    }
}

void StatisticsWorker::checkNearExpiry()
{
    try {
        QDate checkDate = QDate::currentDate().addDays(30); // 检查30天内过期的药品

        QSqlQuery query(db);
        query.prepare(
            "SELECT name, expiry_date, stock "
            "FROM medicine "
            "WHERE expiry_date <= ? AND expiry_date >= ? AND status = 'active' "
            "ORDER BY expiry_date ASC"
            );
        query.addBindValue(checkDate.toString("yyyy-MM-dd"));
        query.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

        if (query.exec()) {
            int warningCount = 0;
            while (query.next()) {
                QString medicineName = query.value("name").toString();
                QDate expiryDate = query.value("expiry_date").toDate();
                int stock = query.value("stock").toInt();
                int daysToExpire = QDate::currentDate().daysTo(expiryDate);

                warningCount++;

                // 发送预警信号
                QJsonObject warningData;
                warningData["medicine_name"] = medicineName;
                warningData["expiry_date"] = expiryDate.toString("yyyy-MM-dd");
                warningData["days_to_expire"] = daysToExpire;
                warningData["current_stock"] = stock;
                warningData["warning_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

                QJsonDocument doc(warningData);
                QString dataJson = doc.toJson(QJsonDocument::Compact);

                emit warningGenerated(
                    "near_expiry",
                    QString("药品 %1 即将过期: 过期日期 %2，剩余 %3 天，库存 %4")
                        .arg(medicineName)
                        .arg(expiryDate.toString("yyyy-MM-dd"))
                        .arg(daysToExpire)
                        .arg(stock)
                    );

                // 记录到预警历史
                QSqlQuery insertQuery(db);
                insertQuery.prepare(
                    "INSERT INTO warnings_history (warning_type, warning_level, message, data_json) "
                    "VALUES ('near_expiry', 'warning', ?, ?)"
                    );
                insertQuery.addBindValue(
                    QString("药品 %1 即将过期").arg(medicineName)
                    );
                insertQuery.addBindValue(dataJson);
                insertQuery.exec();
            }

            if (warningCount > 0) {
                emit statisticsGenerated("expiry",
                                         QString("发现 %1 种药品即将过期").arg(warningCount));
            }
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("检查临近过期药品失败: %1").arg(e.what()));
    }
}

void StatisticsWorker::checkPendingAppointments()
{
    try {
        QSqlQuery query(db);
        query.prepare(
            "SELECT COUNT(*) as count "
            "FROM appointment "
            "WHERE status IN ('scheduled', 'confirmed') "
            "AND appointment_date <= DATE('now', '+1 day')"
            );

        if (query.exec() && query.next()) {
            int pendingCount = query.value("count").toInt();

            if (pendingCount > 0) {
                emit statisticsGenerated("appointments",
                                         QString("有 %1 个预约即将到期").arg(pendingCount));
            }
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("检查待处理预约失败: %1").arg(e.what()));
    }
}

void StatisticsWorker::checkOverduePrescriptions()
{
    try {
        QDate threeDaysAgo = QDate::currentDate().addDays(-3);

        QSqlQuery query(db);
        query.prepare(
            "SELECT COUNT(*) as count "
            "FROM prescription "
            "WHERE payment_status = 'unpaid' "
            "AND DATE(prescription_date) < ?"
            );
        query.addBindValue(threeDaysAgo.toString("yyyy-MM-dd"));

        if (query.exec() && query.next()) {
            int overdueCount = query.value("count").toInt();

            if (overdueCount > 0) {
                emit warningGenerated(
                    "overdue_prescriptions",
                    QString("有 %1 张处方已逾期未缴费").arg(overdueCount)
                    );
            }
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("检查逾期处方失败: %1").arg(e.what()));
    }
}

void StatisticsWorker::performSystemHealthCheck()
{
    try {
        QVariantMap healthStatus;

        // 检查数据库连接
        healthStatus["database_connected"] = db.isOpen();

        // 检查关键表是否存在
        QStringList requiredTables = {"patient", "doctor", "appointment", "consult_record", "prescription", "medicine"};
        int missingTables = 0;

        foreach (const QString &table, requiredTables) {
            QSqlQuery query(db);
            query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
            query.addBindValue(table);

            if (query.exec() && !query.next()) {
                missingTables++;
                healthStatus[table + "_exists"] = false;
            } else {
                healthStatus[table + "_exists"] = true;
            }
        }

        healthStatus["missing_table_count"] = missingTables;

        // 检查数据完整性
        healthStatus["total_patients"] = 0;
        healthStatus["total_doctors"] = 0;
        healthStatus["total_appointments"] = 0;

        QSqlQuery countQuery(db);

        if (countQuery.exec("SELECT COUNT(*) FROM patient") && countQuery.next()) {
            healthStatus["total_patients"] = countQuery.value(0).toInt();
        }

        if (countQuery.exec("SELECT COUNT(*) FROM doctor WHERE status='active'") && countQuery.next()) {
            healthStatus["total_doctors"] = countQuery.value(0).toInt();
        }

        if (countQuery.exec("SELECT COUNT(*) FROM appointment WHERE status='scheduled'") && countQuery.next()) {
            healthStatus["pending_appointments"] = countQuery.value(0).toInt();
        }

        // 生成健康报告
        QJsonObject healthJson = QJsonObject::fromVariantMap(healthStatus);
        QJsonDocument doc(healthJson);
        QString healthData = doc.toJson(QJsonDocument::Compact);

        emit statisticsGenerated("health_check", "系统健康检查完成");

    } catch (const std::exception &e) {
        emit errorOccurred(QString("系统健康检查失败: %1").arg(e.what()));
    }
}

void StatisticsWorker::generateDailyReport()
{
    QVariantMap dailyStats = generateDailyStatistics();
    emit dailyStatsReady(dailyStats);
}

void StatisticsWorker::generateWeeklyReport()
{
    QVariantMap weeklyStats = generateWeeklyStatistics();
    emit weeklyStatsReady(weeklyStats);
}

void StatisticsWorker::generateMonthlyReport()
{
    QVariantMap monthlyStats = generateMonthlyStatistics();
    emit monthlyStatsReady(monthlyStats);
}

void StatisticsWorker::checkSystemHealth()
{
    performSystemHealthCheck();
}
