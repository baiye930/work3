#ifndef STATISTICSWORKER_H
#define STATISTICSWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

class StatisticsWorker : public QObject
{
    Q_OBJECT

public:
    explicit StatisticsWorker(QObject *parent = nullptr);
    ~StatisticsWorker();

public slots:
    void startWork();
    void stopWork();
    void pauseWork();
    void resumeWork();
    void generateDailyReport();
    void generateWeeklyReport();
    void generateMonthlyReport();
    void checkSystemHealth();

signals:
    void statisticsGenerated(const QString &type, const QString &result);
    void warningGenerated(const QString &warningType, const QString &message);
    void errorOccurred(const QString &error);
    void workStatusChanged(const QString &status);
    void dailyStatsReady(const QVariantMap &stats);
    void weeklyStatsReady(const QVariantMap &stats);
    void monthlyStatsReady(const QVariantMap &stats);

private:
    void initDatabase();
    void cleanupDatabase();

    // 统计方法
    QVariantMap generateDailyStatistics();
    QVariantMap generateWeeklyStatistics();
    QVariantMap generateMonthlyStatistics();

    // 预警检查方法
    void checkLowStock();
    void checkNearExpiry();
    void checkPendingAppointments();
    void checkOverduePrescriptions();

    // 系统健康检查
    void performSystemHealthCheck();

    // 私有成员
    QThread *workerThread;
    QSqlDatabase db;
    QMutex mutex;
    QWaitCondition waitCondition;
    bool isRunning;
    bool isPaused;
    QString dbPath;

    // 配置参数
    int checkInterval; // 检查间隔（分钟）
    bool enableAutoReports;
    QTime dailyReportTime;
};

#endif // STATISTICSWORKER_H
