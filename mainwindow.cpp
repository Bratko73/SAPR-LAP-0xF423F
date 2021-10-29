#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->RodsParametersTable->setColumnCount(4);
    ui->RodsParametersTable->setHorizontalHeaderLabels(QStringList()<<"L"<<"A"<<"E"<<"Sigma");
    ui->ConcentratedLoadTable->setColumnCount(1);
    ui->ConcentratedLoadTable->setHorizontalHeaderLabels(QStringList()<<"F");
    ui->LinearLoadTable->setColumnCount(1);
    ui->LinearLoadTable->setHorizontalHeaderLabels(QStringList()<<"q");
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::ValidateDouble(const QString &str)
{
    bool res = false;
    QVariant qvarValue(str);
    qvarValue.toDouble(&res);
    return res;
}

bool MainWindow::ValidateDoubleGZero(const QString &str)
{
    bool res = false;
    QVariant qvarValue(str);
    double tmp = qvarValue.toDouble(&res);
    if (tmp > 0 && res)
        return res;
    else
        return false;
}


void MainWindow::on_CountOfRods_valueChanged(int countOfRods)
{

    if(countOfRods > 0 && ui->RodsParametersTable->rowCount()<countOfRods) {
        ui->RodsParametersTable->setRowCount(countOfRods);
       for (int i = ui->RodsParametersTable->rowCount(); i < countOfRods; i++){
           for (int j = 0; j < ui->RodsParametersTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem();
               ui->RodsParametersTable->setItem(i,j,item);
           }
       }
       ui->LinearLoadTable->setRowCount(countOfRods);
       for (int i = ui->LinearLoadTable->rowCount(); i < countOfRods; i++){
           for (int j = 0; j < ui->LinearLoadTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem();
               ui->LinearLoadTable->setItem(i,j,item);
           }
       }
       ui->ConcentratedLoadTable->setRowCount(countOfRods+1);
       for (int i = ui->ConcentratedLoadTable->rowCount()+1; i < countOfRods+1; i++){
           for (int j = 0; j < ui->ConcentratedLoadTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem();
               ui->ConcentratedLoadTable->setItem(i,j,item);
           }
       }

    }
    else if(ui->RodsParametersTable->rowCount()>countOfRods){
        int tmp =ui->RodsParametersTable->rowCount();

        while (ui->RodsParametersTable->rowCount()>tmp){
            ui->RodsParametersTable->removeRow(tmp-1);
            ui->LinearLoadTable->removeRow(tmp-1);
            ui->ConcentratedLoadTable->removeRow(tmp);
            tmp--;
        }
        ui->RodsParametersTable->setRowCount(countOfRods);
        ui->LinearLoadTable->setRowCount(countOfRods);
        ui->ConcentratedLoadTable->setRowCount(countOfRods+1);
    }
}



void MainWindow::on_RodsParametersTable_cellChanged(int row, int column)
{

    if(ValidateDoubleGZero(ui->RodsParametersTable->item(row,column)->text())){
        ui->RodsParametersTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->RodsParametersTable->item(row,column)->setText(" ");
    }
}

