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
    isCalculated = false;
    ui->RodsParametersTable->setColumnCount(4);
    ui->RodsParametersTable->setHorizontalHeaderLabels(QStringList()<<"L, м"<<"A, м^2"<<"E, Па"<<"Sigma, Па");
    ui->ConcentratedLoadTable->setColumnCount(1);
    ui->ConcentratedLoadTable->setHorizontalHeaderLabels(QStringList()<<"F, Н");
    ui->LinearLoadTable->setColumnCount(1);
    ui->LinearLoadTable->setHorizontalHeaderLabels(QStringList()<<"q, Н/м");
    ui->CountOfRods->setValue(2);
    on_CountOfRods_valueChanged(2);
    ui->CountOfRods->setMaximum(100);
    ui->RodsParametersTable->setRowCount(1);
    ui->LinearLoadTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->ConcentratedLoadTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tabs->setCurrentIndex(0);
    ui->PreprocessorGraphicsWiew->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->PreprocessorGraphicsWiew->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->stepSpinBox->setValue(10);
    ui->stepSpinBox->setMaximum(1000);
    ui->PointL->setSingleStep(0.00001);
    ui->PointL->setDecimals(5);
    timer = new QTimer();
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(draw()));
    timer->start(50);
    ui->mysteryAction->setIcon(QIcon(":/Pictures/DeltaXi-6.png"));
    ui->mysteryAction->setVisible(false);
    ui->mysteryButton2->setVisible(false);
    ui->mysteryButton3->setVisible(false);
    ui->tabs->setTabVisible(2,false);
    mysteryCounter = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotAlarmTimer()
{

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

void MainWindow::NxFormulas()
{
    Nx.clear();
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        double A = ui->RodsParametersTable->item(i,1)->text().toDouble();
        double E = ui->RodsParametersTable->item(i,2)->text().toDouble();
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        double q = ui->LinearLoadTable->item(i,0)->text().toDouble();
        double res = E*A/L*(results[i+1]-results[i])+q*L/2;
        Nx.push_back(QPair<double,double>(-q,res));
    }
}

double MainWindow::NxValue(int rod, double x)
{
    double result = Nx[rod].first * x + Nx[rod].second;
    return result;
}

double MainWindow::SigmaValue(int rod, double x)
{
    double A = ui->RodsParametersTable->item(rod,1)->text().toDouble();
    double result = NxValue(rod,x)/A;
    return result;
}

void MainWindow::UxFormulas()
{
    Ux.clear();
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        double A = ui->RodsParametersTable->item(i,1)->text().toDouble();
        double E = ui->RodsParametersTable->item(i,2)->text().toDouble();
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        double q = ui->LinearLoadTable->item(i,0)->text().toDouble();
        double a = -q/(2*E*A);
        double b = q*L/(2*E*A)+(results[i+1]-results[i])/L;
        double c = results[i];
        Ux.push_back(QPair<QPair<double,double>,double>(QPair<double,double>(a,b),c));
    }
}

double MainWindow::UxValue(int rod, double x)
{
    double result = Ux[rod].first.first * x*x + Ux[rod].first.second * x + Ux[rod].second;
    return result;
}

void MainWindow::UpdatePostProcTable()
{
    if (isCalculated){
        while(ui->postTableWidget->rowCount()>0)
            ui->postTableWidget->removeRow(0);
        for (int i = 0; i < ui->CountOfRods->value(); i++){
            double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
            double step = L/ui->stepSpinBox->value();
            double currL = 0;
            while(currL <= L + step/2){
                ui->postTableWidget->insertRow(ui->postTableWidget->rowCount());
                for (int j = 0; j < ui->postTableWidget->columnCount(); j++){
                    QTableWidgetItem* item = new QTableWidgetItem();
                    item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
                    item->setTextAlignment(Qt::AlignHCenter);
                    switch (j) {
                    case 0:
                        if(i%2==0)
                            item->setBackground(Qt::yellow);
                        item->setText(tr("%1").arg(1e-5*std::round(1e5*currL)));
                        break;
                    case 1:
                        item->setText(tr("%1").arg(1e-5 * std::round(1e5 * UxValue(i,currL))));
                        break;
                    case 2:
                        item->setText(tr("%1").arg(1e-5 * std::round(1e5 * NxValue(i,currL))));
                        break;
                    case 3:
                        double sigma = std::fabs(1e-5 * std::round(1e5 * SigmaValue(i,currL)));
                        if(ui->RodsParametersTable->item(i,3)->text().toDouble()< sigma)
                            item->setBackground(Qt::red);
                        item->setText(tr("%1").arg(sigma));
                        break;
                    }
                    ui->postTableWidget->setItem(ui->postTableWidget->rowCount()-1,j,item);
                }
                currL+=step;
            }
        }
    }
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
    countOfRodsData.setValue(QString().setNum(ui->CountOfRods->value()));
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

void MainWindow::savePostProcToFile(const QString &pathToFile)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Sapr-Lap-0xF423F_PostProcessor");
    doc.appendChild(root);
    QDomElement countOfRods = doc.createElement("CountOfRods");
    root.appendChild(countOfRods);
    QDomAttr countOfRodsData = doc.createAttribute("Count");
    countOfRodsData.setValue(QString().setNum(ui->CountOfRods->value()));
    countOfRods.setAttributeNode(countOfRodsData);
    QDomElement step = doc.createElement("Step");
    root.appendChild(step);
    QDomAttr stepData = doc.createAttribute("Step");
    stepData.setValue(QString().setNum(ui->stepSpinBox->value()));
    step.setAttributeNode(stepData);
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QDomElement rod = doc.createElement("rod_"+QString().setNum(i+1));
        for(int j = 0; j < ui->stepSpinBox->value()+1; j++){
            QDomElement L = doc.createElement("L_"+QString().setNum(j+1));
            QDomAttr length = doc.createAttribute("L");
            length.setValue(ui->postTableWidget->item(i*(ui->stepSpinBox->value()+1)+j,0)->text());
            L.setAttributeNode(length);
            QDomElement Values = doc.createElement("Values");
            QDomAttr Ux = doc.createAttribute("Ux");
            Ux.setValue(ui->postTableWidget->item(i*(ui->stepSpinBox->value()+1)+j,1)->text());
            Values.setAttributeNode(Ux);
            QDomAttr Nx = doc.createAttribute("Nx");
            Nx.setValue(ui->postTableWidget->item(i*(ui->stepSpinBox->value()+1)+j,2)->text());
            Values.setAttributeNode(Nx);
            QDomAttr sigmaAttr = doc.createAttribute("σx");
            sigmaAttr.setValue(ui->postTableWidget->item(i*(ui->stepSpinBox->value()+1)+j,3)->text());
            Values.setAttributeNode(sigmaAttr);
            L.appendChild(Values);
            rod.appendChild(L);
        }
        root.appendChild(rod);
    }

    QFile file(pathToFile);
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream(&file) << doc.toString();
        file.close();
    }
    else
        QMessageBox::information(NULL,QObject::tr("А, ой..."),tr("Мы не смогли сохранить файл :("));
}

void MainWindow::savePostProcImage(const QString &pathToFile)
{
    QImage image(scene->width(), scene->height(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    scene->render(&painter);
    image.save(pathToFile);
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
    draw();
    ui->CountOfRods->blockSignals(false);
    ui->RodsParametersTable->blockSignals(false);
    ui->LinearLoadTable->blockSignals(false);
    ui->ConcentratedLoadTable->blockSignals(false);
}

void MainWindow::LinPoly(QPolygonF &poly, double length, bool isPositive)
{
    poly.clear();
    int k = 1;
    if (!isPositive)
        k = -1;
    poly << QPointF(0,0)
         << QPointF(k*length,0)
         << QPointF(k*length/2,-k*length/8)
         << QPointF(k*length,0)
         << QPointF(k*length/2,k*length/8)
         << QPointF(k*length,0);
}

void MainWindow::ConPoly(QPolygonF &poly, double length, bool isPositive)
{
    poly.clear();
    int k = 1;
    if (!isPositive)
        k = -1;
    poly << QPointF(0,-k*length/8)
         << QPointF(k*length/2,-k*length/8)
         << QPointF(k*length/2,-3*k*length/8)
         << QPointF(k*length,0)
         << QPointF(k*length/2,3*k*length/8)
         << QPointF(k*length/2,k*length/8)
         << QPointF(0,k*length/8)
         << QPointF(0,-k*length/8);
}

void MainWindow::TermPoly(QPolygonF &poly, double width, bool isLeft)
{
    poly.clear();
    int k = 1;
    if (!isLeft)
        k = -1;
    double step=sqrt(width);
    for(double tmp = width/2; tmp > -width/2; tmp-=step){
        poly << QPointF(0,-tmp/2)
             << QPointF(-k*step,-tmp/2+k*step)
             << QPointF(0,-tmp/2);
    }
}

double MainWindow::MaxNx()
{
    double result = fabs(NxValue(0,0));
    for (int i = 0; i < ui->CountOfRods->value(); i++){
        double currL=0;
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        while(currL< L+L/200){
            if(result < fabs(NxValue(i,currL)))
                result = fabs(NxValue(i,currL));
            currL += L/100;
        }
    }
    return result;
}

double MainWindow::MaxUx()
{
    double result = fabs(UxValue(0,0));
    for (int i = 0; i < ui->CountOfRods->value(); i++){
        double currL=0;
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        while(currL< L+L/2000){
            if(result < fabs(UxValue(i,currL)))
                result = fabs(UxValue(i,currL));
            currL += L/1000;
        }
    }
    return result;
}

double MainWindow::MaxSigma()
{
    double result = fabs(SigmaValue(0,0));
    for (int i = 0; i < ui->CountOfRods->value(); i++){
        double currL=0;
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        while(currL< L+L/200){
            if(result < fabs(SigmaValue(i,currL)))
                result = fabs(SigmaValue(i,currL));
            currL += L/100;
        }
    }
    return result;
}

double MainWindow::MeanA()
{
    double result = 0;
    for (int i = 0; i < ui->CountOfRods->value(); i++){
        double A = ui->RodsParametersTable->item(i,1)->text().toDouble();
        result += A;
    }
    return result/ui->CountOfRods->value();
}

void MainWindow::NxPoly(QPolygonF &poly, double length, int rod)
{
    double maxNx = MaxNx();
    double L = ui->RodsParametersTable->item(rod,0)->text().toDouble();
    poly.clear();
    poly << QPointF(0,0)
         << QPointF(length,0)
         << QPointF(length,-100*NxValue(rod,L)/maxNx)
         << QPointF(0,-100*NxValue(rod,0)/maxNx)
         << QPointF(0,0);
}

void MainWindow::UxPoly(QPolygonF &poly, double length, int rod)
{
    double maxUx = MaxUx();
    double L = ui->RodsParametersTable->item(rod,0)->text().toDouble();
    poly.clear();
    double currL = 0;
    double currPixL = 0;
    poly.append(QPoint(0,0));
    while(currL<L+L/2000){
        poly.append(QPoint(currPixL,-150*UxValue(rod,currL)/maxUx));
        currL+=L/1000;
        currPixL+=length/1000;
    }
    poly.append(QPoint(length,0));
    poly.append(QPoint(0,0));
}

void MainWindow::SigmaPoly(QPolygonF &poly, double length, int rod)
{
    NxPoly(poly,length,rod);
    double A = ui->RodsParametersTable->item(rod,1)->text().toDouble();
        poly[2].ry()*= A/MeanA();
        poly[3].ry()*= A/MeanA();
}
// Рисование
void MainWindow::draw()
{
    graphicsScene = new QGraphicsScene;
    graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->PreprocessorGraphicsWiew->setScene(graphicsScene);
    if (!ValidateTables()){
        graphicsScene->clear();
        DeltaXi = graphicsScene->addPixmap(QPixmap(":/resourses/cat.jpg"));
        QRectF r = graphicsScene->sceneRect();
        ui->PreprocessorGraphicsWiew->fitInView(r,Qt::KeepAspectRatio);
        return;
    }
    ui->PreprocessorGraphicsWiew->clearMask();
    graphicsScene->clear();
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    pen.setBrush(Qt::black);
    double sizeOfConArrows=50;
    double sizeOfLinArrows=30;
    double offsetX = 0;
    double widthCoeff = 0;
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,1)->text();
        widthCoeff += tmp.toDouble();
    }
    widthCoeff = ui->PreprocessorGraphicsWiew->rect().height()/2 * ui->CountOfRods->value()/ widthCoeff;
    double lengthCoeff = ui->RodsParametersTable->item(0,0)->text().toDouble();
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,0)->text();
        lengthCoeff += tmp.toDouble();
    }
    lengthCoeff = 200 * ui->CountOfRods->value() / lengthCoeff;


    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,1)->text();
        double h = tmp.toDouble() * widthCoeff;
        tmp = ui->RodsParametersTable->item(i,0)->text();
        double w = tmp.toDouble() * lengthCoeff;
        QRectF rect (0,-h/2,w,h);
        rect.translate(offsetX,0);


        if ((ui->termComboBox->currentIndex() == 0 || ui->termComboBox->currentIndex() == 2)&& i == 0){
            QPolygonF poly;
            TermPoly(poly,h+150);
            graphicsScene->addPolygon(poly)->setPen(pen);
        }
        else if(ui->ConcentratedLoadTable->item(i,0)->text().toDouble() > 0){
            QPolygonF poly;
            ConPoly(poly,sizeOfConArrows);
            poly.translate(offsetX,0);
            graphicsScene->addPolygon(poly)->setBrush(Qt::blue);
        }
        else if(ui->ConcentratedLoadTable->item(i,0)->text().toDouble() < 0){
            QPolygonF poly;
            ConPoly(poly,sizeOfConArrows,0);
            poly.translate(offsetX,0);
            graphicsScene->addPolygon(poly)->setBrush(Qt::blue);
        }

        if(ui->LinearLoadTable->item(i,0)->text().toDouble() > 0){
            QPolygonF poly;

            double miniOffSet = w;
            int j = 1;
            while (miniOffSet > sizeOfLinArrows){
                miniOffSet = w/j;
                j++;
            }
            LinPoly(poly,miniOffSet);
            poly.translate(offsetX,0);
            for (int k = 0; k < j-1; k++){
                graphicsScene->addPolygon(poly.translated(k*miniOffSet,0))->setPen(pen);
            }
        }
        else if(ui->LinearLoadTable->item(i,0)->text().toDouble() < 0){
            QPolygonF poly;

            double miniOffSet = w;
            int j = 1;
            while (miniOffSet > sizeOfLinArrows){
                miniOffSet = w/j;
                j++;
            }
            LinPoly(poly,miniOffSet,0);
            poly.translate(offsetX,0);
            for (int k = 1; k < j; k++){
                graphicsScene->addPolygon(poly.translated(k*miniOffSet,0))->setPen(pen);
            }
        }

        graphicsScene->addRect(rect)->setPen(pen);
        offsetX += rect.width();
        if(i == ui->CountOfRods->value()-1){
            if (ui->termComboBox->currentIndex() == 0 || ui->termComboBox->currentIndex() == 1){
                QPolygonF poly;
                TermPoly(poly,h+150,0);
                poly.translate(offsetX,0);
                graphicsScene->addPolygon(poly)->setPen(pen);
            }
            else if(ui->ConcentratedLoadTable->item(ui->CountOfRods->value(),0)->text().toDouble() > 0){
                QPolygonF poly;
                ConPoly(poly,sizeOfConArrows);
                poly.translate(offsetX,0);
                graphicsScene->addPolygon(poly)->setBrush(Qt::blue);
            }
            else if(ui->ConcentratedLoadTable->item(ui->CountOfRods->value(),0)->text().toDouble() < 0){
                QPolygonF poly;
                ConPoly(poly,sizeOfConArrows,0);
                poly.translate(offsetX,0);
                graphicsScene->addPolygon(poly)->setBrush(Qt::blue);
            }
        }
    }
}

void MainWindow::postProcDraw()
{
    scene = new QGraphicsScene;
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    ui->postGraphicsView->setScene(scene);
    if (!ValidateTables()){
        scene->clear();
        DeltaXi = scene->addPixmap(QPixmap(":/resourses/cat.jpg"));
        QRectF r = scene->sceneRect();
        ui->PreprocessorGraphicsWiew->fitInView(r,Qt::KeepAspectRatio);
        return;
    }
    ui->PreprocessorGraphicsWiew->clearMask();
    scene->clear();
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    pen.setBrush(Qt::black);


    double sizeOfConArrows=50;
    double sizeOfLinArrows=30;
    double offsetX = 0;
    double widthCoeff = 0;
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,1)->text();
        widthCoeff += tmp.toDouble();
    }
    widthCoeff = ui->PreprocessorGraphicsWiew->rect().height()/2 * ui->CountOfRods->value()/ widthCoeff;
    double lengthCoeff = ui->RodsParametersTable->item(0,0)->text().toDouble();
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,0)->text();
        lengthCoeff += tmp.toDouble();
    }
    lengthCoeff = 200 * ui->CountOfRods->value() / lengthCoeff;

    double offsetY = 400;
    for(int i = 0; i < ui->CountOfRods->value(); i++){
        QVariant tmp = ui->RodsParametersTable->item(i,1)->text();
        double h = tmp.toDouble() * widthCoeff;
        tmp = ui->RodsParametersTable->item(i,0)->text();
        double w = tmp.toDouble() * lengthCoeff;

        QRectF rect (0,-h/2,w,h);
        rect.translate(offsetX,0);
        QLine lineV(0,0,0,offsetY);
        lineV.translate(offsetX,0);
        QLine lineH(0,offsetY-100,w,offsetY-100);
        lineH.translate(offsetX,0);

        if(ui->checkNx->isChecked()){
            QPolygonF poly;
            NxPoly(poly,w,i);
            poly.translate(offsetX,offsetY-100);
            QPen Nxpen;
            Nxpen.setWidth(2);
            Nxpen.setColor(Qt::blue);
            Nxpen.setBrush(Qt::blue);
            scene->addPolygon(poly)->setPen(Nxpen);
        }
        if(ui->checkUx->isChecked()){
            QPolygonF poly;
            UxPoly(poly,w,i);
            poly.translate(offsetX,offsetY-100);
            QPen Uxpen;
            Uxpen.setWidth(2);
            Uxpen.setColor(Qt::red);
            Uxpen.setBrush(Qt::red);
            scene->addPolygon(poly)->setPen(Uxpen);
        }
        if(ui->checkSigma->isChecked()){
            QPolygonF poly;
            SigmaPoly(poly,w,i);
            poly.translate(offsetX,offsetY-100);
            QPen Sigmapen;
            Sigmapen.setWidth(2);
            Sigmapen.setColor(Qt::green);
            Sigmapen.setBrush(Qt::green);
            scene->addPolygon(poly)->setPen(Sigmapen);
        }
        if ((ui->termComboBox->currentIndex() == 0 || ui->termComboBox->currentIndex() == 2)&& i == 0){
            QPolygonF poly;
            TermPoly(poly,h+150);
            scene->addPolygon(poly)->setPen(pen);
        }
        else if(ui->ConcentratedLoadTable->item(i,0)->text().toDouble() > 0){
            QPolygonF poly;
            ConPoly(poly,sizeOfConArrows);
            poly.translate(offsetX,0);
            scene->addPolygon(poly)->setBrush(Qt::blue);
        }
        else if(ui->ConcentratedLoadTable->item(i,0)->text().toDouble() < 0){
            QPolygonF poly;
            ConPoly(poly,sizeOfConArrows,0);
            poly.translate(offsetX,0);
            scene->addPolygon(poly)->setBrush(Qt::blue);
        }
        if(ui->LinearLoadTable->item(i,0)->text().toDouble() > 0){
            QPolygonF poly;

            double miniOffSet = w;
            int j = 1;
            while (miniOffSet > sizeOfLinArrows){
                miniOffSet = w/j;
                j++;
            }
            LinPoly(poly,miniOffSet);
            poly.translate(offsetX,0);
            for (int k = 0; k < j-1; k++){
                scene->addPolygon(poly.translated(k*miniOffSet,0))->setPen(pen);
            }
        }
        else if(ui->LinearLoadTable->item(i,0)->text().toDouble() < 0){
            QPolygonF poly;

            double miniOffSet = w;
            int j = 1;
            while (miniOffSet > sizeOfLinArrows){
                miniOffSet = w/j;
                j++;
            }
            LinPoly(poly,miniOffSet,0);
            poly.translate(offsetX,0);
            for (int k = 1; k < j; k++){
                scene->addPolygon(poly.translated(k*miniOffSet,0))->setPen(pen);
            }
        }

        scene->addRect(rect)->setPen(pen);
        scene->addLine(lineV)->setPen(pen);
        scene->addLine(lineH)->setPen(pen);
        offsetX += rect.width();
        if(i == ui->CountOfRods->value()-1){
            QLine line(0,0,0,offsetY);
            line.translate(offsetX,0);
            scene->addLine(line)->setPen(pen);
            if (ui->termComboBox->currentIndex() == 0 || ui->termComboBox->currentIndex() == 1){
                QPolygonF poly;
                TermPoly(poly,h+150,0);
                poly.translate(offsetX,0);
                scene->addPolygon(poly)->setPen(pen);
            }
            else if(ui->ConcentratedLoadTable->item(ui->CountOfRods->value(),0)->text().toDouble() > 0){
                QPolygonF poly;
                ConPoly(poly,sizeOfConArrows);
                poly.translate(offsetX,0);
                scene->addPolygon(poly)->setBrush(Qt::blue);
            }
            else if(ui->ConcentratedLoadTable->item(ui->CountOfRods->value(),0)->text().toDouble() < 0){
                QPolygonF poly;
                ConPoly(poly,sizeOfConArrows,0);
                poly.translate(offsetX,0);
                scene->addPolygon(poly)->setBrush(Qt::blue);
            }
        }
    }

}

void MainWindow::MysteryRestore()
{
    mysteryCounter = 0;
    ui->MysteryButton->setVisible(true);
    ui->mysteryAction->setVisible(false);
    ui->mysteryButton2->setVisible(false);
    ui->mysteryButton3->setVisible(false);
    ui->EasterEgg->setVisible(false);
    ui->tabs->setTabVisible(0,true);
    ui->tabs->setTabVisible(1,true);
    ui->tabs->setTabVisible(2,false);
}

void MainWindow::on_CountOfRods_valueChanged(int countOfRods)
{
    isCalculated = false;
    if(ui->PointRod->count()<countOfRods) {
        for(int i = ui->PointRod->count(); i < countOfRods; i++)
            ui->PointRod->addItem(QString(tr("%1").arg(i+1)));
    }
    else if(ui->PointRod->count()>countOfRods && countOfRods >= 1){
        for(int i = ui->PointRod->count(); i > countOfRods; i--)
            ui->PointRod->removeItem(i-1);
    }

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
               //item->setTextAlignment(Qt::AlignHCenter);
               item->setBackground(Qt::green);
               ui->LinearLoadTable->setItem(i,j,item);
           }
       }
       for (int i = ui->ConcentratedLoadTable->rowCount(); i < countOfRods+1; i++){
           ui->ConcentratedLoadTable->insertRow(i);
           for (int j = 0; j < ui->ConcentratedLoadTable->columnCount(); j++) {
               QTableWidgetItem* item = new QTableWidgetItem(tr("%1").arg(0));
               //item->setTextAlignment(Qt::AlignHCenter);
               item->setBackground(Qt::green);
               ui->ConcentratedLoadTable->setItem(i,j,item);
           }
       }
       draw();
    }
    else if(ui->RodsParametersTable->rowCount()>countOfRods && countOfRods >= 1){

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
        draw();
    }
    else
        ui->CountOfRods->setValue(1);

}

void MainWindow::on_RodsParametersTable_cellChanged(int row, int column)
{
    isCalculated = false;
    if(ValidateDoubleGZero(ui->RodsParametersTable->item(row,column)->text())){
        ui->RodsParametersTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->RodsParametersTable->item(row,column)->setBackground(Qt::red);
    }
}

void MainWindow::on_LinearLoadTable_cellChanged(int row, int column)
{
    isCalculated = false;
    if(ValidateDouble(ui->LinearLoadTable->item(row,column)->text())){
        ui->LinearLoadTable->item(row,column)->setBackground(Qt::green);    
    }
    else {
        ui->LinearLoadTable->item(row,column)->setBackground(Qt::red);
    }
}

void MainWindow::on_ConcentratedLoadTable_cellChanged(int row, int column)
{
    isCalculated = false;
    if(ValidateDouble(ui->ConcentratedLoadTable->item(row,column)->text())){
        ui->ConcentratedLoadTable->item(row,column)->setBackground(Qt::green);
    }
    else {
        ui->ConcentratedLoadTable->item(row,column)->setBackground(Qt::red);
    }
}

void MainWindow::on_ProcessorButton_clicked()
{
    if (!ValidateTables()){
        return;
    }
    for(int i = 0; i < matrix.size(); i++)
        matrix[i].clear();
    matrix.clear();
    results.clear();
    int countOfNodes = ui->CountOfRods->value()+1;
    for (int i = 0; i < countOfNodes; i++){
        QVector<double> tmp;
        for (int j = 0; j < countOfNodes+1; j++){
            tmp.push_back(0);
        }
        matrix.push_back(tmp);
    }
    for(int i = 0; i < countOfNodes-1; i++){
        double L = ui->RodsParametersTable->item(i,0)->text().toDouble();
        double A = ui->RodsParametersTable->item(i,1)->text().toDouble();
        double E = ui->RodsParametersTable->item(i,2)->text().toDouble();
        double K = E*A/L;
        if(i < countOfNodes-2){
            matrix[i][i] += K;
            matrix[i+1][i] += -K;
            matrix[i][i+1] += -K;
            matrix[i+1][i+1] += K;
        }
        else if(i == countOfNodes-2){
                matrix[i][i] += K;
                matrix[i+1][i] += -K;
                matrix[i][i+1] += -K;
                matrix[i+1][i+1] += K;
        }
    }
    for (int i = 0; i < countOfNodes-1;i++){
        if (i == 0){
            double b = ui->LinearLoadTable->item(i,0)->text().toDouble();
            b *= ui->RodsParametersTable->item(i,0)->text().toDouble()/2;
            b += ui->ConcentratedLoadTable->item(i,0)->text().toDouble();
            matrix[i][countOfNodes] = b;
            if (countOfNodes-1 == 1){
                double b = ui->LinearLoadTable->item(i,0)->text().toDouble();
                b *= ui->RodsParametersTable->item(i,0)->text().toDouble()/2;
                b += ui->ConcentratedLoadTable->item(i+1,0)->text().toDouble();
                matrix[i+1][countOfNodes] = b;
            }
        }
        else if(i == countOfNodes-2){
            double q1 = ui->LinearLoadTable->item(i,0)->text().toDouble();
            q1 *= ui->RodsParametersTable->item(i,0)->text().toDouble()/2;
            double q2 = ui->LinearLoadTable->item(i-1,0)->text().toDouble();
            q2 *= ui->RodsParametersTable->item(i-1,0)->text().toDouble()/2;
            q2 += ui->ConcentratedLoadTable->item(i,0)->text().toDouble() + q1;
            matrix[i][countOfNodes] = q2;
            double b = ui->LinearLoadTable->item(i,0)->text().toDouble();
            b *= ui->RodsParametersTable->item(i,0)->text().toDouble()/2;
            b += ui->ConcentratedLoadTable->item(i+1,0)->text().toDouble();
            matrix[i+1][countOfNodes] = b;
        }
        else{
            double q1 = ui->LinearLoadTable->item(i,0)->text().toDouble();
            q1 *= ui->RodsParametersTable->item(i,0)->text().toDouble()/2;
            double q2 = ui->LinearLoadTable->item(i-1,0)->text().toDouble();
            q2 *= ui->RodsParametersTable->item(i-1,0)->text().toDouble()/2;
            q2 += ui->ConcentratedLoadTable->item(i,0)->text().toDouble() + q1;
            matrix[i][countOfNodes] = q2;
        }

    }
    switch (ui->termComboBox->currentIndex()) {
    case 0:
        matrix[0][0] = 1;
        matrix[0][1] = 0;
        matrix[1][0] = 0;
        matrix[0][countOfNodes] = 0;
        matrix[countOfNodes-1][countOfNodes-1] = 1;
        matrix[countOfNodes-2][countOfNodes-1] = 0;
        matrix[countOfNodes-1][countOfNodes-2] = 0;
        matrix[countOfNodes-1][countOfNodes] = 0;
        break;
    case 1:
        matrix[countOfNodes-1][countOfNodes-1] = 1;
        matrix[countOfNodes-2][countOfNodes-1] = 0;
        matrix[countOfNodes-1][countOfNodes-2] = 0;
        matrix[countOfNodes-1][countOfNodes] = 0;
        break;
    case 2:
        matrix[0][0] = 1;
        matrix[0][1] = 0;
        matrix[1][0] = 0;
        matrix[0][countOfNodes] = 0;
        break;

    }
    QString tmp = "";
    for (int i = 0; i < countOfNodes;i++){
        tmp += tr("A_%1 |").arg(i);
        for(int j = 0; j < countOfNodes+1; j++){
            QVariant var = matrix[i][j];
            tmp+=var.toString()+"|";
        }
        tmp += "\n";
    }
    //QMessageBox::information(NULL,QObject::tr("Я сделяль.."),tmp);
    processor = new Processor(ui->CountOfRods->value());
    processor->Calculate(matrix);
    double* result = processor->GetDeltas();
    for(int i = 0; i < countOfNodes; i++){
        results.push_back(result[i]);
    }
    tmp = "";
    for (int i=0;i<countOfNodes; i++){
        QVariant var = results[i];
        tmp += var.toString()+"| ";
    }
    QMessageBox::information(NULL,QObject::tr("Процессор сделяль.."),tmp);
    isCalculated = true;
    NxFormulas();
    UxFormulas();
    UpdatePostProcTable();
    postProcDraw();
}

void MainWindow::on_termComboBox_currentIndexChanged(int index)
{
    isCalculated = false;
    draw();
}

void MainWindow::on_actionOpen_triggered()
{
    isCalculated = false;
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
    if(ui->tabs->currentIndex() == 0){
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
    else if (isCalculated){
        QString fileName = QFileDialog::getSaveFileName(this,
                                    QString::fromUtf8("Сохранить файл"),
                                    QDir::currentPath(),
                                "XML (*.xml);;All files (*.*)");
        savePostProcToFile(fileName);

        fileName = QFileDialog::getSaveFileName(this,
                                    QString::fromUtf8("Сохранить файл"),
                                    QDir::currentPath(),
                                "PNG (*.png);;All files (*.*)");
        savePostProcImage(fileName);
    }

}

void MainWindow::on_MysteryButton_clicked()
{
    mysteryCounter++;
    QMessageBox msg= QMessageBox();
    if(mysteryCounter < 14)
        msg.setIconPixmap(QPixmap(":/Pictures/DuckPic.png").scaled(200,200));
    else
        msg.setIconPixmap(QPixmap(":/resourses/RealDuck.png"));
    msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
    msg.setTextFormat(Qt::TextFormat::MarkdownText);
    msg.addButton("Окей",QMessageBox::DestructiveRole);
    switch (mysteryCounter) {
    case 1:
        msg.setWindowTitle("Приветствую");
        msg.setText("# Сжатие уточки");
        msg.setInformativeText("Не стоит трогать эту кнопку, некоторых вещей лучше не знать...");
        msg.exec();
        break;
    case 5:
        msg.setWindowTitle("Интересный факт");
        msg.setText("# Опора уточки");
        msg.setInformativeText("Интересное слово 'Безумие', а вы знаете что это такое?");
        msg.exec();
        break;
    case 7:
        msg.setWindowTitle("А вы упорный");
        msg.setText("# Поперечное сечение уточки");
        msg.setInformativeText("Безумие — это точное повторение одного и того же действия раз за разом в надежде на изменение.");
        msg.exec();
        break;
    case 8:
        msg.setWindowTitle("А вы всё упорнее");
        msg.setText("# Растяжение уточки");
        msg.setInformativeText("Это.");
        msg.exec();
        break;
    case 9:
        msg.setWindowTitle("УПОРСТВО!");
        msg.setText("# Оболочка уточки");
        msg.setInformativeText("Есть.");
        msg.exec();
        break;
    case 10:
        msg.setWindowTitle("Решимость");
        msg.setText("# Метод утиных сил");
        msg.setInformativeText("Безумие.");
        msg.exec();
        break;
    case 11:
        msg.setIconPixmap(QPixmap(":/Pictures/DuckWithKnife.png").scaled(200,200));
        msg.setWindowTitle("...");
        msg.setText("# нАпряжение уточки");
        msg.setInformativeText("Ну как так можно? Остановитесь!");
        msg.exec();
        ui->MysteryButton->setStyleSheet("image: url(:/Pictures/DeltaXi-1.png);");
        break;
    case 12:
        msg.setWindowTitle("...");
        msg.setText("# Текучесть уточки");
        msg.setInformativeText("Простых слов вы не понимаете... ЛАДНО, я уберу эту кнопку! Лучше идите считайте стержни!.");
        msg.exec();
        ui->MysteryButton->setVisible(false);
        ui->mysteryButton2->setVisible(true);
        break;
    case 20:
        msg.setIconPixmap(QPixmap(":/resourses/RealDuck.png"));
        msg.setWindowIcon(QIcon(":/resourses/DeltaXi"));
        msg.setWindowTitle("Что будем делать?");
        msg.setText("# Аркадий");
        msg.setInformativeText("Уважаемый пользователь, раз вам не нужно считать стержни, давайте их уберем :)");
        msg.exec();
        ui->tabs->setTabVisible(0,false);
        ui->tabs->setTabVisible(1,false);
        ui->tabs->setTabVisible(2,true);
        break;
    default: break;
    }
}

void MainWindow::on_RefreshButton_clicked()
{
    if(mysteryCounter == 13){
        QMessageBox msg = QMessageBox();
        msg.setIconPixmap(QPixmap(":/resourses/RealDuck1111.png").scaled(200,200));
        msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
        msg.setTextFormat(Qt::TextFormat::MarkdownText);
        msg.addButton("Окей",QMessageBox::DestructiveRole);
        msg.setWindowTitle("Упс...");
        msg.setText("# Аркаша");
        msg.setInformativeText("Это какой-то программный сбой. Теперь точно удалено!");
        msg.exec();
        ui->RefreshButton->setStyleSheet("image: url(:/resourses/restart.png);");
        ui->mysteryAction->setVisible(true);
        return;
    }
    draw();
}

void MainWindow::on_stepSpinBox_valueChanged(int arg1)
{
    if(arg1<10)
        ui->stepSpinBox->setValue(10);
    UpdatePostProcTable();
}

void MainWindow::on_PointButton_clicked()
{
    if(isCalculated){
        double currRod = ui->PointRod->currentIndex();
        double currL = ui->RodsParametersTable->item(currRod,0)->text().toDouble();
        if(ui->PointL->value() <= currL){
            double res = NxValue(currRod, ui->PointL->value());
            ui->NxPoint_label->setText(QString(tr("Nx = %1").arg(res)));
            res = UxValue(currRod,ui->PointL->value());
            ui->UxPoint_label->setText(QString(tr("Ux = %1").arg(res)));
            res = SigmaValue(currRod,ui->PointL->value());
            ui->SigmaPoint_label->setText(QString(tr("σx = %1").arg(res)));
        }
        else {
            ui->NxPoint_label->setText(QString("Не лги мне!"));
            ui->UxPoint_label->setText(QString("Мне не лги!"));
            ui->SigmaPoint_label->setText(QString("Лги не мне!"));
        }
    }
    else{
        ui->NxPoint_label->setText(QString("Данные не пересчитаны!"));
        ui->UxPoint_label->setText(QString("Не пересчитаны данные!"));
        ui->SigmaPoint_label->setText(QString("Пересчитаны не данные!"));
    }
}

void MainWindow::on_checkNx_stateChanged(int arg1)
{
    if(isCalculated)
        postProcDraw();
}

void MainWindow::on_checkUx_stateChanged(int arg1)
{
    if(isCalculated)
        postProcDraw();
}

void MainWindow::on_checkSigma_stateChanged(int arg1)
{
    if(isCalculated)
        postProcDraw();
}

void MainWindow::on_mysteryAction_triggered()
{
    QMessageBox msg = QMessageBox();
    msg.setIconPixmap(QPixmap(":/resourses/RealDuck.png"));
    msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
    msg.setTextFormat(Qt::TextFormat::MarkdownText);
    msg.addButton("Окей",QMessageBox::DestructiveRole);
    msg.setWindowTitle("...");
    msg.setText("# Аркадий");
    msg.setInformativeText("МУ-ХА-ХА... Продолжай...");
    ui->MysteryButton->setStyleSheet("image: url(:/Pictures/DeltaXi.png);");
    msg.exec();
    ui->mysteryAction->setVisible(false);
    ui->MysteryButton->setVisible(true);
}

void MainWindow::on_mysteryButton2_clicked()
{
    QMessageBox msg = QMessageBox();
    msg.setIconPixmap(QPixmap(":/Pictures/DuckPic.png").scaled(200,200));
    msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
    msg.setTextFormat(Qt::TextFormat::MarkdownText);
    msg.addButton("Окей",QMessageBox::DestructiveRole);
    msg.setWindowTitle("Упс...");
    msg.setText("# Аркаша");
    msg.setInformativeText("Вот это да, вы смогли найти её! Хорошо, спрячу её получше.");
    msg.exec();
    ui->mysteryButton2->setVisible(false);
    ui->mysteryButton3->setVisible(true);
}

void MainWindow::on_mysteryButton3_clicked()
{
    QMessageBox msg = QMessageBox();
    msg.setIconPixmap(QPixmap(":/resourses/RealDuck1111.png").scaled(200,200));
    msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
    msg.setTextFormat(Qt::TextFormat::MarkdownText);
    msg.addButton("Окей",QMessageBox::DestructiveRole);
    msg.setWindowTitle("Упс...");
    msg.setText("# Аркаша");
    msg.setInformativeText("Ну нет, это ни в какие ворота не лезет!!! Всё, удаляю кнопку.");
    msg.exec();
    mysteryCounter++;
    ui->mysteryButton3->setVisible(false);
    ui->RefreshButton->setStyleSheet("image: url(:/Pictures/DeltaXi-5.png);");
}

void MainWindow::on_endThis_clicked()
{
    QMessageBox msg = QMessageBox();
    msg.setIconPixmap(QPixmap(":/Pictures/fox.jpg").scaled(500,600));
    msg.setWindowIcon(QIcon(":/Pictures/cat.gif"));
    msg.setTextFormat(Qt::TextFormat::MarkdownText);
    msg.addButton("До новых встреч!",QMessageBox::DestructiveRole);
    msg.setWindowTitle("Пара слов на последок");
    msg.setText("# Вы прошли Компютерную механику!\n Курс был очень веселым, и интересным, спасибо!");
    msg.exec();
    MysteryRestore();
}

void MainWindow::on_pushButton_4_clicked()
{
    mysteryCounter--;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

void MainWindow::on_pushButton_2_clicked()
{
    mysteryCounter--;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

void MainWindow::on_pushButton_clicked()
{
    mysteryCounter++;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

void MainWindow::on_pushButton_3_clicked()
{
    mysteryCounter++;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

void MainWindow::on_pushButton_5_clicked()
{
    mysteryCounter++;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

void MainWindow::on_pushButton_6_clicked()
{
    mysteryCounter++;
    ui->label->setText(tr("%1").arg(mysteryCounter));
}

