#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <QObject>
#include "statisticsworker.h"
#include <QTimer>
#include <QFile>
#include <QTextStream>

class StatisticsManager : public QObject
{
    Q_OBJECT

public:
    static StatisticsManager& getInstance();

    void startStatisticsService();
    void stopStatisticsService();
    void pauseStatisticsService();
    void resumeStatisticsService();

    void generateReportNow(const QString &reportType);
    void exportStatistics(const QString &filePath, const QString &format = "csv");
    void clearOldStatistics(int keepDays = 30);

    QVariantMap getDailyStats() const { return dailyStats; }
    QVariantMap getWeeklyStats() const { return weeklyStats; }
    QVariantMap getMonthlyStats() const { return monthlyStats; }
    QList<QVariantMap> getWarnings() const { return warnings; }

signals:
    void notificationReceived(const QString &title, const QString &message);
    void statsUpdated();
    void warningAlert(const QString &warningType, const QString &message);

private slots:
    void onDailyStatsReady(const QVariantMap &stats);
    void onWeeklyStatsReady(const QVariantMap &stats);
    void onMonthlyStatsReady(const QVariantMap &stats);
    void onWarningGenerated(const QString &warningType, const QString &message);
    void onErrorOccurred(const QString &error);
    void onWorkStatusChanged(const QString &status);

private:
    explicit StatisticsManager(QObject *parent = nullptr);
    ~StatisticsManager();
    StatisticsManager(const StatisticsManager&) = delete;
    StatisticsManager& operator=(const StatisticsManager&) = delete;

    void saveStatsToFile();
    void loadStatsFromFile();
    void setupConnections();

    StatisticsWorker *worker;
    QThread *workerThread;

    QVariantMap dailyStats;
    QVariantMap weeklyStats;
    QVariantMap monthlyStats;
    QList<QVariantMap> warnings;

    bool isServiceRunning;
    QString logFilePath;
};

#endif // STATISTICSMANAGER_H
