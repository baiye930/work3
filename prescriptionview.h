#ifndef PRESCRIPTIONVIEW_H
#define PRESCRIPTIONVIEW_H

#include <QWidget>

namespace Ui {
class Prescriptionview;
}

class Prescriptionview : public QWidget
{
    Q_OBJECT

public:
    explicit Prescriptionview(QWidget *parent = nullptr);
    ~Prescriptionview();

private:
    Ui::Prescriptionview *ui;
};

#endif // PRESCRIPTIONVIEW_H
