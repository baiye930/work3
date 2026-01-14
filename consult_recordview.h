#ifndef CONSULT_RECORDVIEW_H
#define CONSULT_RECORDVIEW_H

#include <QWidget>

namespace Ui {
class consult_recordview;
}

class consult_recordview : public QWidget
{
    Q_OBJECT

public:
    explicit consult_recordview(QWidget *parent = nullptr);
    ~consult_recordview();

private:
    Ui::consult_recordview *ui;
};

#endif // CONSULT_RECORDVIEW_H
