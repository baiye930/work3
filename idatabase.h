#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QDataWidgetMapper>

class IDatabase : public QObject
{
    Q_OBJECT
public:

    static IDatabase &getInstance(){
        static IDatabase instance;
        return instance;
    }

    QString userLogin(QString userName,QString password);



private:
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(IDatabase const &)       =delete;
    void operator=(IDatabase const &)  =delete;

    QSqlDatabase database;

    void ininDatabase();


signals:

public:
    bool initPatientModel();
    int addNewPatient();
    bool searchPatient(QString filter);
    bool deleteCurrentPatient();
    bool submitPatientEdit();
    void revertPatientEdit();


    // 医生管理方法（新增）
    bool initDoctorModel();
    int addNewDoctor();
    bool searchDoctor(const QString &filter);
    bool deleteCurrentDoctor();
    bool submitDoctorEdit();
    void revertDoctorEdit();
    QList<QString> getDepartmentsForCombo();  // 获取科室列表供下拉框使用
    QStringList getDoctorTitles();           // 获取职称列表
    QStringList getDoctorStatuses();         // 获取状态列表

    QSqlTableModel *patientTabModel;
    QItemSelectionModel *thePatientSelection;

    QSqlTableModel *doctorTabModel;          // 新增医生模型
    QItemSelectionModel *theDoctorSelection; // 新增医生选择模型


};

#endif
