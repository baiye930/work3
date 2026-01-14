#ifndef MEDICINEVIEW_H
#define MEDICINEVIEW_H

#include <QWidget>

namespace Ui {
class medicineview;
}

class medicineview : public QWidget
{
    Q_OBJECT

public:
    explicit medicineview(QWidget *parent = nullptr);
    ~medicineview();

private:
    Ui::medicineview *ui;
};

#endif // MEDICINEVIEW_H
