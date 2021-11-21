#include "postproc.h"
#include "ui_postproc.h"

PostProc::PostProc(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PostProc)
{
    ui->setupUi(this);
}

PostProc::~PostProc()
{
    delete ui;
}
