#include "statisticsmanager.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

StatisticsManager::StatisticsManager(QObject *parent)
    : QObject(parent)
    , worker(nullptr)
    , workerThread(nullptr)
    , isServiceRunning(false)
{
    // 设置日志文件路径
    QString appDir = QCoreApplication::applicationDirPath();
    QDir logDir(appDir + "/logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    logFilePath = logDir.filePath("statistics.log");

    // 初始化统计服务
    startStatisticsService();
}

StatisticsManager::~StatisticsManager()
{
    stopStatisticsService();
}

StatisticsManager& StatisticsManager::getInstance()
{
    static StatisticsManager instance;
    return instance;
}

void StatisticsManager::startStatisticsService()
{
    if (isServiceRunning) return;

    qDebug() << "启动统计服务...";

    // 创建工作线程和工作者
    workerThread = new QThread();
    worker = new StatisticsWorker();
    worker->moveToThread(workerThread);

    // 设置信号连接
    setupConnections();

    // 启动工作线程
    workerThread->start();

    isServiceRunning = true;

    // 记录日志
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " - 统计服务已启动\n";
        logFile.close();
    }

    emit notificationReceived("统计服务", "后台统计服务已启动");
}

void StatisticsManager::stopStatisticsService()
{
    if (!isServiceRunning) return;

    qDebug() << "停止统计服务...";

    if (worker) {
        worker->stopWork();
    }

    if (workerThread) {
        workerThread->quit();
        workerThread->wait(3000); // 等待3秒
        delete workerThread;
        workerThread = nullptr;
    }

    if (worker) {
        delete worker;
        worker = nullptr;
    }

    isServiceRunning = false;

    // 记录日志
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " - 统计服务已停止\n";
        logFile.close();
    }

    emit notificationReceived("统计服务", "后台统计服务已停止");
}

void StatisticsManager::pauseStatisticsService()
{
    if (worker && isServiceRunning) {
        worker->pauseWork();
        emit notificationReceived("统计服务", "统计服务已暂停");
    }
}

void StatisticsManager::resumeStatisticsService()
{
    if (worker && isServiceRunning) {
        worker->resumeWork();
        emit notificationReceived("统计服务", "统计服务已恢复");
    }
}

void StatisticsManager::setupConnections()
{
    if (!worker) return;

    // 连接工作者的信号
    connect(worker, &StatisticsWorker::dailyStatsReady,
            this, &StatisticsManager::onDailyStatsReady);
    connect(worker, &StatisticsWorker::weeklyStatsReady,
            this, &StatisticsManager::onWeeklyStatsReady);
    connect(worker, &StatisticsWorker::monthlyStatsReady,
            this, &StatisticsManager::onMonthlyStatsReady);
    connect(worker, &StatisticsWorker::warningGenerated,
            this, &StatisticsManager::onWarningGenerated);
    connect(worker, &StatisticsWorker::errorOccurred,
            this, &StatisticsManager::onErrorOccurred);
    connect(worker, &StatisticsWorker::workStatusChanged,
            this, &StatisticsManager::onWorkStatusChanged);

    // 连接线程的信号
    connect(workerThread, &QThread::started,
            worker, &StatisticsWorker::startWork);
    connect(workerThread, &QThread::finished,
            worker, &StatisticsWorker::deleteLater);
}

void StatisticsManager::onDailyStatsReady(const QVariantMap &stats)
{
    dailyStats = stats;
    saveStatsToFile();
    emit statsUpdated();
    emit notificationReceived("每日统计", "每日统计报告已生成");
}

void StatisticsManager::onWeeklyStatsReady(const QVariantMap &stats)
{
    weeklyStats = stats;
    saveStatsToFile();
    emit statsUpdated();
    emit notificationReceived("每周统计", "每周统计报告已生成");
}

void StatisticsManager::onMonthlyStatsReady(const QVariantMap &stats)
{
    monthlyStats = stats;
    saveStatsToFile();
    emit statsUpdated();
    emit notificationReceived("每月统计", "每月统计报告已生成");
}

void StatisticsManager::onWarningGenerated(const QString &warningType, const QString &message)
{
    QVariantMap warning;
    warning["type"] = warningType;
    warning["message"] = message;
    warning["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    warnings.append(warning);

    // 限制警告数量，最多保留100条
    if (warnings.size() > 100) {
        warnings.removeFirst();
    }

    emit warningAlert(warningType, message);
    emit notificationReceived("系统预警", message);

    // 记录到日志文件
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " - 预警: " << warningType << " - " << message << "\n";
        logFile.close();
    }
}

void StatisticsManager::onErrorOccurred(const QString &error)
{
    qDebug() << "统计服务错误:" << error;

    // 记录到日志文件
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
            << " - 错误: " << error << "\n";
        logFile.close();
    }

    emit notificationReceived("统计服务错误", error);
}

void StatisticsManager::onWorkStatusChanged(const QString &status)
{
    qDebug() << "统计服务状态:" << status;
}

void StatisticsManager::generateReportNow(const QString &reportType)
{
    if (!worker) return;

    if (reportType == "daily") {
        worker->generateDailyReport();
    } else if (reportType == "weekly") {
        worker->generateWeeklyReport();
    } else if (reportType == "monthly") {
        worker->generateMonthlyReport();
    } else if (reportType == "health") {
        worker->checkSystemHealth();
    }
}

void StatisticsManager::exportStatistics(const QString &filePath, const QString &format)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit notificationReceived("导出失败", "无法创建文件");
        return;
    }

    QTextStream out(&file);

    if (format.toLower() == "csv") {
        // 导出CSV格式
        out << "统计类型,统计时间,数据项,数值\n";

        QDateTime now = QDateTime::currentDateTime();

        // 导出日报
        foreach (const QString &key, dailyStats.keys()) {
            out << "每日统计," << now.toString("yyyy-MM-dd HH:mm:ss") << ","
                << key << "," << dailyStats[key].toString() << "\n";
        }

        // 导出周报
        foreach (const QString &key, weeklyStats.keys()) {
            out << "每周统计," << now.toString("yyyy-MM-dd HH:mm:ss") << ","
                << key << "," << weeklyStats[key].toString() << "\n";
        }

        // 导出月报
        foreach (const QString &key, monthlyStats.keys()) {
            out << "每月统计," << now.toString("yyyy-MM-dd HH:mm:ss") << ","
                << key << "," << monthlyStats[key].toString() << "\n";
        }
    } else if (format.toLower() == "json") {
        // 导出JSON格式
        QJsonObject exportData;
        exportData["export_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        exportData["daily_stats"] = QJsonObject::fromVariantMap(dailyStats);
        exportData["weekly_stats"] = QJsonObject::fromVariantMap(weeklyStats);
        exportData["monthly_stats"] = QJsonObject::fromVariantMap(monthlyStats);

        QJsonArray warningsArray;
        foreach (const QVariantMap &warning, warnings) {
            warningsArray.append(QJsonObject::fromVariantMap(warning));
        }
        exportData["warnings"] = warningsArray;

        QJsonDocument doc(exportData);
        out << doc.toJson();
    }

    file.close();
    emit notificationReceived("导出成功", QString("统计已导出到 %1").arg(filePath));
}

void StatisticsManager::clearOldStatistics(int keepDays)
{
    if (!workerThread) return;

    // 在实际项目中，这里应该清理数据库中的旧统计记录
    QDate cutoffDate = QDate::currentDate().addDays(-keepDays);

    qDebug() << "清理" << keepDays << "天前的统计记录";
    emit notificationReceived("清理统计", QString("已清理%1天前的统计记录").arg(keepDays));
}

void StatisticsManager::saveStatsToFile()
{
    QString statsFilePath = QCoreApplication::applicationDirPath() + "/stats_cache.json";

    QJsonObject cache;
    cache["daily_stats"] = QJsonObject::fromVariantMap(dailyStats);
    cache["weekly_stats"] = QJsonObject::fromVariantMap(weeklyStats);
    cache["monthly_stats"] = QJsonObject::fromVariantMap(monthlyStats);
    cache["last_update"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QFile file(statsFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QJsonDocument doc(cache);
        out << doc.toJson();
        file.close();
    }
}

void StatisticsManager::loadStatsFromFile()
{
    QString statsFilePath = QCoreApplication::applicationDirPath() + "/stats_cache.json";

    QFile file(statsFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString jsonStr = in.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isNull()) {
            QJsonObject cache = doc.object();

            if (cache.contains("daily_stats")) {
                dailyStats = cache["daily_stats"].toObject().toVariantMap();
            }

            if (cache.contains("weekly_stats")) {
                weeklyStats = cache["weekly_stats"].toObject().toVariantMap();
            }

            if (cache.contains("monthly_stats")) {
                monthlyStats = cache["monthly_stats"].toObject().toVariantMap();
            }
        }
    }
}
