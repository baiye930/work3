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


    // 科室管理方法（新增）
    bool initDepartmentModel();
    int addNewDepartment();
    bool searchDepartment(const QString &filter);
    bool deleteCurrentDepartment();
    bool submitDepartmentEdit();
    void revertDepartmentEdit();

    // 获取科室状态列表
    QStringList getDepartmentStatuses();

    // 获取模型指针
    QSqlTableModel *departmentTabModel;
    QItemSelectionModel *theDepartmentSelection;

    // 药品管理方法（新增）
    bool initMedicineModel();
    int addNewMedicine();
    bool searchMedicine(const QString &filter);
    bool deleteCurrentMedicine();
    bool submitMedicineEdit();
    void revertMedicineEdit();

    // 库存管理方法
    bool stockIn(const QString &medicineId, int quantity, const QString &batchNumber = "",
                 const QDate &expiryDate = QDate());
    bool stockOut(const QString &medicineId, int quantity, const QString &referenceId = "");
    bool adjustStock(const QString &medicineId, int newQuantity);

    // 预警方法
    QList<QString> getLowStockMedicines(int threshold = 10);
    QList<QString> getNearExpiryMedicines(int days = 30);
    double getTotalInventoryValue();

    // 获取药品分类列表
    QStringList getMedicineCategories();
    QStringList getDosageForms();

    // 获取模型指针
    QSqlTableModel *medicineTabModel;
    QItemSelectionModel *theMedicineSelection;


    // 处方管理方法（新增）
    bool initPrescriptionModel();
    int addNewPrescription();
    bool searchPrescription(const QString &filter);
    bool deleteCurrentPrescription();
    bool submitPrescriptionEdit();
    void revertPrescriptionEdit();

    // 处方详情方法
    bool initPrescriptionDetailModel(const QString &prescriptionId);
    bool addPrescriptionDetail(const QString &prescriptionId, const QString &medicineId,
                               int quantity, const QString &dosage);
    bool deletePrescriptionDetail(const QString &detailId);

    // 处方状态操作
    bool dispensePrescription(const QString &prescriptionId);
    bool processPayment(const QString &prescriptionId, const QString &paymentMethod);
    bool auditPrescription(const QString &prescriptionId);

    // 统计方法
    QMap<QString, QVariant> getTodayPrescriptionStats();
    QMap<QString, QVariant> getMonthPrescriptionStats();
    int getPendingDispenseCount();
    int getUnpaidCount();

    // 获取医生列表
    QList<QString> getDoctorsForCombo();

    // 获取模型指针
    QSqlTableModel *prescriptionTabModel;
    QItemSelectionModel *thePrescriptionSelection;
    QSqlTableModel *prescriptionDetailTabModel;
    QItemSelectionModel *thePrescriptionDetailSelection;

    QSqlTableModel *patientTabModel;
    QItemSelectionModel *thePatientSelection;

    QSqlTableModel *doctorTabModel;          // 新增医生模型
    QItemSelectionModel *theDoctorSelection; // 新增医生选择模型

    // 预约管理方法（新增）
    bool initAppointmentModel();
    int addNewAppointment();
    bool searchAppointment(const QString &filter);
    bool deleteCurrentAppointment();
    bool submitAppointmentEdit();
    void revertAppointmentEdit();

    // 获取预约状态列表
    QStringList getAppointmentStatuses();

    // 获取预约模型指针
    QSqlTableModel *appointmentTabModel;
    QItemSelectionModel *theAppointmentSelection;

    // 统计方法
    QMap<QString, QVariant> getTodayAppointmentStats();
    QMap<QString, QVariant> getTomorrowAppointmentStats();
    QMap<QString, QVariant> getAppointmentSummaryStats();

    // 状态操作
    bool updateAppointmentStatus(const QString &appointmentId, const QString &newStatus);

    // 获取医生和患者列表
    QList<QString> getPatientsForCombo();
    QList<QString> getDoctorsForAppointmentCombo();

    // 预约时间冲突检查
    bool checkTimeConflict(const QString &doctorId, const QDateTime &appointmentTime,
                           const QString &excludeAppointmentId = "");

};

#endif
