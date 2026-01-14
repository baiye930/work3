#ifndef DOCTOREDITVIEW_H
#define DOCTOREDITVIEW_H

#include <QWidget>

namespace Ui {
class DoctoreditView;
}

class DoctoreditView : public QWidget
{
    Q_OBJECT

public:
    explicit DoctoreditView(QWidget *parent = nullptr);
    ~DoctoreditView();

private:
    Ui::DoctoreditView *ui;
};

#endif // DOCTOREDITVIEW_H
