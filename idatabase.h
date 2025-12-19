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


    QSqlTableModel *patientTabModel;
    QItemSelectionModel *thePatientSelection;



};

#endif
