#ifndef DOCTOREDITVIEW_H
#define DOCTOREDITVIEW_H

#include <QWidget>
#include <QDataWidgetMapper>
#include <QRegularExpression>

namespace Ui {
class DoctoreditView;
}

class DoctoreditView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctoreditView(QWidget *parent = nullptr, int index = 0);
    ~DoctoreditView();

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();

signals:
    void goPreviousView();

private:
    void initComboBoxes();
    bool validateInput();

    Ui::DoctoreditView *ui;
    QDataWidgetMapper *dataMapper;
};

#endif // DOCTOREDITVIEW_H
