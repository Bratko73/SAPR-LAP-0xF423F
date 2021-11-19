#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include "processor.h"

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
    ui->MysteryButton->setStyleSheet("border-image: url(d:/QtProjects/SAPR-LAP-0xF423F/DeltaXi.png); ");
    ui->CountOfRods->setValue(1);
    ui->RodsParametersTable->setRowCount(1);

    graphicsScene = new QGraphicsScene;
    ui->PreprocessorGraphicsWiew->setScene(graphicsScene);

    for (int j = 0; j < ui->RodsParametersTable->columnCount(); j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setText("1");
        item->setTextAlignment(Qt::AlignHCenter);
        ui->RodsParametersTable->setItem(0,j,item);
    }
    ui->LinearLoadTable->setRowCount(1);
    for (int j = 0; j < ui->LinearLoadTable->columnCount(); j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setText("0");
        item->setTextAlignment(Qt::AlignHCenter);
        ui->LinearLoadTable->setItem(0,j,item);
    }
    ui->ConcentratedLoadTable->setRowCount(2);
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < ui->ConcentratedLoadTable->columnCount(); j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setText("0");
            item->setTextAlignment(Qt::AlignHCenter);
            ui->ConcentratedLoadTable->setItem(i,j,item);
        }
}

MainWindow::~MainWindow()
{
    //delete proc;
    delete ui;
}

bool MainWindow::ValidateDouble(const QString &str)
{
    bool res = false;
    if(str == nullptr)
        return res;
    QVariant qvarValue(str);
    qvarValue.toDouble(&res);
    return res;
}

bool MainWindow::ValidateDoubleGZero(const QString &str)
{
    bool res = false;
    if(str == nullptr)
        return res;
    QVariant qvarValue(str);
    double tmp = qvarValue.toDouble(&res);
    if (tmp > 0 && res)
        return res;
    else
        return false;
}

bool MainWindow::ValidateTables()
{
    for (int i = 0; i < ui->RodsParametersTable->rowCount(); i++)
        for (int j = 0; j < ui->RodsParametersTable->columnCount(); j++)
            if (ui->RodsParametersTable->item(i,j)!=nullptr){
                if (!ValidateDoubleGZero(ui->RodsParametersTable->item(i,j)->text()))
                    return false;
            }
            else
                return false;
    for (int i = 0; i < ui->LinearLoadTable->rowCount(); i++)
        for (int j = 0; j < ui->LinearLoadTable->columnCount(); j++)
            if (ui->LinearLoadTable->item(i,j)!=nullptr){
                if (!ValidateDouble(ui->LinearLoadTable->item(i,j)->text()))
                    return false;
            }
            else
                return false;
    for (int i = 0; i < ui->ConcentratedLoadTable->rowCount(); i++)
        for (int j = 0; j < ui->ConcentratedLoadTable->columnCount(); j++)
            if (ui->ConcentratedLoadTable->item(i,j)!=nullptr){
                if (!ValidateDouble(ui->ConcentratedLoadTable->item(i,j)->text()))
                    return false;
            }
            else
                return false;
    return true;
}

QDomElement MainWindow::AddRodToXML(QDomDocument &doc,
                                const QString &L,
                                const QString &A,
                                const QString &E,
                                const QString &Sigma,
                                const int Number)
{
    QDomElement rod = doc.createElement("rod_"+QString().setNum(Number));
    QDomAttr length = doc.createAttribute("L");
    length.setValue(L);
    rod.setAttributeNode(length);
    QDomAttr area = doc.createAttribute("A");
    area.setValue(A);
    rod.setAttributeNode(area);
    QDomAttr elastic = doc.createAttribute("E");
    elastic.setValue(E);
    rod.setAttributeNode(elastic);
    QDomAttr sigmaAttr = doc.createAttribute("Sigma");
    sigmaAttr.setValue(Sigma);
    rod.setAttributeNode(sigmaAttr);
    return rod;
}

void MainWindow::saveToFile(const QString& pathToFile)
{
    int row = ui->RodsParametersTable->rowCount();
    QDomDocument doc;
    QDomElement root = doc.createElement("Sapr-Lap-0xF423F");
    QDomElement countOfRods = doc.createElement("CountOfRods");
    QDomAttr countOfRodsData = doc.createAttribute("Count");
    countOfRodsData.setValue(ui->CountOfRods->text());
    QDomElement rodsParameters = doc.createElement("RodsParameters");
    QDomElement Loads = doc.createElement("Loads");
    QDomElement Linear = doc.createElement("Linear");
    QDomElement Concentrated = doc.createElement("Concentrated");
    QDomElement term = doc.createElement("terms");
    QDomAttr termData = doc.createAttribute("t");
    termData.setValue(QString().setNum(ui->termComboBox->currentIndex()));

    doc.appendChild(root);
    root.appendChild(countOfRods);
    root.appendChild(term);
    term.setAttributeNode(termData);
    countOfRods.setAttributeNode(countOfRodsData);
    root.appendChild(rodsParameters);
    for (int i = 0; i < row; i++){
        QString L = ui->RodsParametersTable->item(i,0)->text();
        QString A = ui->RodsParametersTable->item(i,1)->text();
        QString E = ui->RodsParametersTable->item(i,2)->text();
        QString Sigma = ui->RodsParametersTable->item(i,3)->text();
        rodsParameters.appendChild(AddRodToXML(doc,L,A,E,Sigma,i+1));
    }
    root.appendChild(Loads);
    Loads.appendChild(Linear);
    for(int i = 0; i < row; i++){
        QDomElement q = doc.createElement("q_"+QString().setNum(i+1));
        QDomAttr qAttr = doc.createAttribute("q");
        qAttr.setValue(ui->LinearLoadTable->item(i,0)->text());
        Linear.appendChild(q);
        q.setAttributeNode(qAttr);
    }
    Loads.appendChild(Concentrated);
    for(int i = 0; i < row + 1; i++){
        QDomElement F = doc.createElement("F_"+QString().setNum(i+1));
        QDomAttr FAttr = doc.createAttribute("F");
        FAttr.setValue(ui->ConcentratedLoadTable->item(i,0)->text());
        Concentrated.appendChild(F);
        F.setAttributeNode(FAttr);
    }


    QFile file(pathToFile);
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream(&file) << doc.toString();
        file.close();
    }
    else
        QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("Мы не смогли сохранить файл :("));
}

void MainWindow::parseXML(QDomNode &node)
{

    QDomNode nodeToParse = node.firstChild();
    while (!nodeToParse.isNull()){
        if (nodeToParse.isElement()){
            QDomElement elem = nodeToParse.toElement();
            if (!elem.isNull()){
              if (elem.tagName()=="CountOfRods"){
                  QVariant qvarValue(elem.attribute("Count",""));
                  ui->CountOfRods->setValue(qvarValue.toInt());
                  on_CountOfRods_valueChanged(qvarValue.toInt());

              }
              if (elem.tagName()=="terms"){
                  QVariant qvarValue(elem.attribute("t",""));
                  ui->termComboBox->setCurrentIndex(qvarValue.toInt());
              }
              for(int i = 0; i < ui->CountOfRods->value(); i++){
                  QString tmp = "rod_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      ui->RodsParametersTable->item(i,0)->setText(elem.attribute("L",""));
                      on_RodsParametersTable_cellChanged(i,0);
                      ui->RodsParametersTable->item(i,1)->setText(elem.attribute("A",""));
                      on_RodsParametersTable_cellChanged(i,1);
                      ui->RodsParametersTable->item(i,2)->setText(elem.attribute("E",""));
                      on_RodsParametersTable_cellChanged(i,2);
                      ui->RodsParametersTable->item(i,3)->setText(elem.attribute("Sigma",""));
                      on_RodsParametersTable_cellChanged(i,3);
                  }
                  tmp = "q_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      ui->LinearLoadTable->item(i,0)->setText(elem.attribute("q",""));
                      on_LinearLoadTable_cellChanged(i,0);
                  }
                  tmp = "F_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      ui->ConcentratedLoadTable->item(i,0)->setText(elem.attribute("F",""));
                      on_ConcentratedLoadTable_cellChanged(i,0);
                  }
              }
              QString tmp = "F_"+QString().setNum(ui->CountOfRods->value()+1);
              if(elem.tagName()==tmp){
                  ui->ConcentratedLoadTable->item(ui->CountOfRods->value(),0)->setText(elem.attribute("F",""));
              }
            }
        }
        parseXML(nodeToParse);
        nodeToParse = nodeToParse.nextSibling();
    }

}

void MainWindow::loadFromFile(QString& pathToFile)
{
    ui->CountOfRods->blockSignals(true);
    ui->RodsParametersTable->blockSignals(true);
    ui->LinearLoadTable->blockSignals(true);
    ui->ConcentratedLoadTable->blockSignals(true);

    QDomDocument doc;
    QFile file(pathToFile);
    if(file.open(QIODevice::ReadOnly)) {
        if(doc.setContent(&file)) {
            QDomElement root = doc.documentElement();
            parseXML(root);
            doc.clear();
        }
        else{
            QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("Не получается :("));

        }
            file.close();
    }

    ui->CountOfRods->blockSignals(false);
    ui->RodsParametersTable->blockSignals(false);
    ui->LinearLoadTable->blockSignals(false);
    ui->ConcentratedLoadTable->blockSignals(false);
}
// Рисование
void MainWindow::draw()
{

}


void MainWindow::on_CountOfRods_valueChanged(int countOfRods)
{

    if(countOfRods > 1 && ui->RodsParametersTable->rowCount()<countOfRods) {
       for (int i = ui->RodsParametersTable->rowCount(); i < countOfRods; i++){
           ui->RodsParametersTable->insertRow(i);
           for (int j = 0; j < ui->RodsParametersTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem(tr("%1").arg(1));
               item->setTextAlignment(Qt::AlignHCenter);
               item->setBackground(Qt::green);
               ui->RodsParametersTable->setItem(i,j,item);
           }
       }
       for (int i = ui->LinearLoadTable->rowCount(); i < countOfRods; i++){
           ui->LinearLoadTable->insertRow(i);
           for (int j = 0; j < ui->LinearLoadTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem(tr("%1").arg(0));
               item->setTextAlignment(Qt::AlignHCenter);
               item->setBackground(Qt::green);
               ui->LinearLoadTable->setItem(i,j,item);
           }
       }
       for (int i = ui->ConcentratedLoadTable->rowCount(); i < countOfRods+1; i++){
           ui->ConcentratedLoadTable->insertRow(i);
           for (int j = 0; j < ui->ConcentratedLoadTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem(tr("%1").arg(0));
               item->setTextAlignment(Qt::AlignHCenter);
               item->setBackground(Qt::green);
               ui->ConcentratedLoadTable->setItem(i,j,item);
           }
       }
    }
    else if(ui->RodsParametersTable->rowCount()>countOfRods){

        while (ui->RodsParametersTable->rowCount()>countOfRods){
            int tmp =ui->RodsParametersTable->rowCount();
            ui->RodsParametersTable->removeRow(tmp-1);
            ui->LinearLoadTable->removeRow(tmp-1);
            ui->ConcentratedLoadTable->removeRow(tmp);
            tmp--;
        }
        ui->RodsParametersTable->setRowCount(countOfRods);
        ui->LinearLoadTable->setRowCount(countOfRods);
        if(countOfRods == 0)
           ui->ConcentratedLoadTable->setRowCount(countOfRods);
        else
           ui->ConcentratedLoadTable->setRowCount(countOfRods+1);
    }
}



void MainWindow::on_RodsParametersTable_cellChanged(int row, int column)
{

    if(ValidateDoubleGZero(ui->RodsParametersTable->item(row,column)->text())){
        ui->RodsParametersTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->RodsParametersTable->item(row,column)->setBackground(Qt::red);
    }
}


void MainWindow::on_LinearLoadTable_cellChanged(int row, int column)
{
    ui->LinearLoadTable->item(row,column)->setTextAlignment(Qt::AlignHCenter);
    if(ValidateDouble(ui->LinearLoadTable->item(row,column)->text())){
        ui->LinearLoadTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->LinearLoadTable->item(row,column)->setBackground(Qt::red);
    }
}


void MainWindow::on_ConcentratedLoadTable_cellChanged(int row, int column)
{
    ui->ConcentratedLoadTable->item(row,column)->setTextAlignment(Qt::AlignHCenter);
    if(ValidateDouble(ui->ConcentratedLoadTable->item(row,column)->text())){
        ui->ConcentratedLoadTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->ConcentratedLoadTable->item(row,column)->setBackground(Qt::red);
    }
}


void MainWindow::on_ProcessorButton_clicked()
{
    /*if(ValidateTables()){
        QString fileName = QFileDialog::getSaveFileName(this,
                                    QString::fromUtf8("Сохранить файл"),
                                    QDir::currentPath(),
                                    "XML (*.xml);;All files (*.*)");
        saveToFile(fileName);
    }
    else{
        QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("Давай по новой, данные не валидны :("));
    }*/
}


void MainWindow::on_termComboBox_currentIndexChanged(int index)
{

}


void MainWindow::on_actionOpen_triggered()
{
        QString fileName = QFileDialog::getOpenFileName(this,
                                    QString::fromUtf8("Открыть файл"),
                                    QDir::currentPath(),
                                    "XML (*.xml);;All files (*.*)");
        if (fileName!=nullptr){
            loadFromFile(fileName);
        }
        else{
            QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("И файл... испарился?"));
            return;
        }
}


void MainWindow::on_actionSave_triggered()
{
    if(ValidateTables()){
        QString fileName = QFileDialog::getSaveFileName(this,
                                    QString::fromUtf8("Сохранить файл"),
                                    QDir::currentPath(),
                                    "XML (*.xml);;All files (*.*)");
        saveToFile(fileName);
    }
    else{
        QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("А жареных гвоздей не хочешь?"));
    }
}

