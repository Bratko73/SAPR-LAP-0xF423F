#ifndef POSTPROC_H
#define POSTPROC_H

#include <QWidget>

namespace Ui {
class PostProc;
}

class PostProc : public QWidget
{
    Q_OBJECT

public:
    explicit PostProc(QWidget *parent = nullptr);
    ~PostProc();

private:
    Ui::PostProc *ui;
};

#endif // POSTPROC_H
