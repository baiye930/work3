#ifndef APPOINTMENTVIEW_H
#define APPOINTMENTVIEW_H

#include <QWidget>

namespace Ui {
class appointmentview;
}

class appointmentview : public QWidget
{
    Q_OBJECT

public:
    explicit appointmentview(QWidget *parent = nullptr);
    ~appointmentview();

private:
    Ui::appointmentview *ui;
};

#endif // APPOINTMENTVIEW_H
