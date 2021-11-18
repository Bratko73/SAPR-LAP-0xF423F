#include "processor.h"
#include <QMessageBox>


Processor::Processor()
{
    countOfRods++;
    Length.push_back(1);
    Area.push_back(1);
    Elastic.push_back(1);
    q.push_back(0);
    F.push_back(0);
    F.push_back(0);
    Sigmas.push_back(1);
    isCalculated = false;
}

Processor::~Processor()
{
    delete[] delta;
}

void Processor::OpenFile(QString pathToFile)
{
    Length.clear();
    Area.clear();
    Elastic.clear();
    q.clear();
    F.clear();
    Sigmas.clear();
    QDomDocument doc;
    QFile file(pathToFile);
    if(file.open(QIODevice::ReadOnly)) {
        if(doc.setContent(&file)) {
            //QMessageBox::information(NULL,QObject::tr("А, ой..."),QObject::tr("Файл открылся"));
            QDomElement root = doc.documentElement();
            parseXML(root);
        }
        else{
            QMessageBox::information(NULL,QObject::tr("А, ой..."),QObject::tr("Не получается :("));
        }
            file.close();
    }
}

void Processor::CalculateDelta()
{
    double** matrA;
    double* vecB;

    int countOfUnits = countOfRods+1;
    matrA = new double*[countOfUnits];
    for (int i = 0; i < (countOfUnits); i++)
        matrA[i]=new double[countOfUnits];
    delta = new double[countOfUnits];
    vecB = new double[countOfUnits];
    for(int i = 0; i < countOfUnits; i++){
        delta[i] = 0;
        if (terms.first && i == 0)
            vecB[i] = 0;
        else if (terms.second && i == countOfUnits - 1)
            vecB[i] = 0;
        else if (i == 0){
            vecB[i] = q[i]*Length[i]/2+F[i];
        }
        else if (i == countOfUnits - 1){
            vecB[i] = q[i-1]*Length[i-1]/2+F[i];
        }
        else{
            vecB[i] = q[i-1]*Length[i-1]/2+q[i]*Length[i]/2+F[i];
        }
        for (int j = 0; j < countOfUnits; j++){
            matrA[i][j] = 0;
        }
    }
    for(int i = 0; i < countOfUnits; i++){
        for (int j = 0; j < countOfUnits; j++){
            if(terms.first && i == 0 && j == 0)
                matrA[i][j] = 1;
            else if (terms.second && i == countOfRods - 1 && j == countOfRods - 1) {
                matrA[i][j] += Elastic[i]*Area[i]/Length[i];
                matrA[i+1][j+1] = 1;
            }
            else if (i == j && i < countOfRods-1){
                matrA[i][j] += Elastic[i]*Area[i]/Length[i];
                matrA[i+1][j+1] += Elastic[i]*Area[i]/Length[i];
                matrA[i+1][j] -= Elastic[i]*Area[i]/Length[i];
                matrA[i][j+1] -= Elastic[i]*Area[i]/Length[i];
            }
        }
    }
    Gauss();

    for (int i = 0; i < countOfRods+1; i++)
        delete[] matrA[i];
    delete[] matrA;
    delete[] vecB;
}

void Processor::SetTerms(bool left, bool right)
{
    if(left || right){
        terms.first = left;
        terms.second = right;
    }
    else {
        terms.first = true;
        terms.second = false;
    }
}

void Processor::AddRod()
{
    isCalculated = false;
    countOfRods++;
    Length.push_back(1);
    Area.push_back(1);
    Elastic.push_back(1);
    q.push_back(0);
    F.push_back(0);
    Sigmas.push_back(1);
}

void Processor::ChangeRodParameter(int rodNum, char param, double value)
{
    isCalculated = false;
    if(rodNum < countOfRods){
        if (value > 0){
            switch(param){
            case 'L':
                Length[rodNum] = value;
                break;
            case 'A':
                Area[rodNum] = value;
                break;
            case 'E':
                Elastic[rodNum] = value;
                break;
            case 'S':
                Sigmas[rodNum] = value;
                break;
            }
        }
    }
}

void Processor::ChangeLoad(int Num, double value, bool isLinear)
{
    isCalculated = false;
    if (isLinear){
        if (Num < countOfRods)
        q[Num] = value;
    }
    else if (Num <= countOfRods)
        F[Num] = value;
}

double Processor::GetRodParameter(int rodNum, char param)
{
    double result = 0;
    if(rodNum < countOfRods)
        switch(param){
        case 'L':
            result = Length[rodNum];
            break;
        case 'A':
            result = Area[rodNum];
            break;
        case 'E':
            result = Elastic[rodNum];
            break;
        case 'S':
            result = Sigmas[rodNum];
            break;
        }
    return result;
}

double Processor::GetLoad(int Num, bool isLinear)
{
    double result = 0;
    if (isLinear){
        if (Num < countOfRods)
        result = q[Num];
    }
    else if (Num <= countOfRods) {
        result = F[Num];
    }
    return result;
}

void Processor::DeleteRod()
{
    if(countOfRods > 1){
        isCalculated = false;
        countOfRods--;
        Length.pop_back();
        Area.pop_back();
        Elastic.pop_back();
        q.pop_back();
        F.pop_back();
        Sigmas.pop_back();
    }
}

QDomElement Processor::AddRodToXML(QDomDocument &doc,
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

void Processor::Save(QString pathToFile)
{
    QString termIndex;
    if (terms.first && terms.second)
        termIndex = "0";
    else if (!terms.first && terms.second)
        termIndex = "1";
    else if (terms.first && !terms.second)
        termIndex = "2";
    QDomDocument doc;
    QDomElement root = doc.createElement("Sapr-Lap-0xF423F");
    QDomElement CountOfRods = doc.createElement("CountOfRods");
    QDomAttr countOfRodsData = doc.createAttribute("Count");
    countOfRodsData.setValue(QString().setNum(countOfRods));
    QDomElement rodsParameters = doc.createElement("RodsParameters");
    QDomElement Loads = doc.createElement("Loads");
    QDomElement Linear = doc.createElement("Linear");
    QDomElement Concentrated = doc.createElement("Concentrated");
    QDomElement term = doc.createElement("terms");
    QDomAttr termData = doc.createAttribute("t");
    termData.setValue(termIndex);


    doc.appendChild(root);
    root.appendChild(CountOfRods);
    root.appendChild(term);
    term.setAttributeNode(termData);
    CountOfRods.setAttributeNode(countOfRodsData);
    root.appendChild(rodsParameters);
    for (int i = 0; i < countOfRods; i++){
        QString L = QVariant(Length[i]).toString();
        QString A = QVariant(Area[i]).toString();
        QString E = QVariant(Elastic[i]).toString();
        QString Sigma = QVariant(Sigmas[i]).toString();
        rodsParameters.appendChild(AddRodToXML(doc,L,A,E,Sigma,i+1));
    }
    root.appendChild(Loads);
    Loads.appendChild(Linear);
    for(int i = 0; i < countOfRods; i++){
        QDomElement LinLoad = doc.createElement("q_"+QString().setNum(i+1));
        QDomAttr qAttr = doc.createAttribute("q");
        qAttr.setValue(QVariant(q[i]).toString());
        Linear.appendChild(LinLoad);
        LinLoad.setAttributeNode(qAttr);
    }
    Loads.appendChild(Concentrated);
    for(int i = 0; i < countOfRods + 1; i++){
        QDomElement ConLoads = doc.createElement("F_"+QString().setNum(i+1));
        QDomAttr FAttr = doc.createAttribute("F");
        FAttr.setValue(QVariant(F[i]).toString());
        Concentrated.appendChild(ConLoads);
        ConLoads.setAttributeNode(FAttr);
    }
    if (isCalculated){
        QDomElement process = doc.createElement("Processor");
        root.appendChild(process);
        for(int i = 0; i < countOfRods + 1; i++){
            QDomElement deltas = doc.createElement("Delta_"+QString().setNum(i+1));
            QDomAttr deltaAttr = doc.createAttribute("delta");
            deltaAttr.setValue(QVariant(delta[i]).toString());
            process.appendChild(deltas);
            deltas.setAttributeNode(deltaAttr);
        }
    }

    QFile file(pathToFile);
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream(&file) << doc.toString();
        file.close();
    }
    else
        QMessageBox::information(NULL,QObject::tr("А, ой..."),QObject::tr("Процессор не смог сохранить файл :("));

}

void Processor::parseXML(QDomNode &node)
{
    QDomNode nodeToParse = node.firstChild();
    while (!nodeToParse.isNull()){
        if (nodeToParse.isElement()){
            QDomElement elem = nodeToParse.toElement();
            if (!elem.isNull()){
              if (elem.tagName()=="CountOfRods"){
                  QVariant qvarValue(elem.attribute("Count",""));
                  countOfRods=qvarValue.toInt();
              }
              if (elem.tagName()=="terms"){
                  QVariant qvarValue(elem.attribute("t",""));
                  if(qvarValue.toInt()==0)
                      terms=QPair<bool,bool> (true,true);
                  else if (qvarValue.toInt()==1)
                      terms=QPair<bool,bool> (false,true);
                  else if (qvarValue.toInt()==2)
                      terms=QPair<bool,bool> (true,false);
              }
              for(int i = 0; i < countOfRods; i++){
                  QString tmp = "rod_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      Length.push_back(QVariant(elem.attribute("L","")).toDouble());
                      Area.push_back(QVariant(elem.attribute("A","")).toDouble());
                      Elastic.push_back(QVariant(elem.attribute("E","")).toDouble());
                      Sigmas.push_back(QVariant(elem.attribute("Sigma","")).toDouble());
                  }
                  tmp = "q_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      q.push_back(QVariant(elem.attribute("q","")).toDouble());
                  }
                  tmp = "F_"+QString().setNum(i+1);
                  if(elem.tagName()==tmp){
                      F.push_back(QVariant(elem.attribute("F","")).toDouble());
                  }
              }
              QString tmp = "F_"+QString().setNum(countOfRods+1);
              if(elem.tagName()==tmp){
                  F.push_back(QVariant(elem.attribute("F","")).toDouble());
              }
            }
        }
        parseXML(nodeToParse);
        nodeToParse = nodeToParse.nextSibling();
    }

}

void Processor::Gauss()
{
/*
    int countOfUnits = countOfRods+1;
    //1 матрица
    double** matrA_copy_1 = new double*[countOfUnits];
    for(int i = 0; i < countOfUnits + 1; i++)
        matrA_copy_1[i] = new double [countOfUnits];
    for(int i = 0; i < countOfUnits; i++)
        for(int j = 0; j < countOfUnits+1; j++)
            if (j == countOfUnits)
                matrA_copy_1[i][j] = vecB[i];
            else
                matrA_copy_1[i][j] = matrA[i][j];

    double** matrA_copy_2 = new double*[countOfUnits];
    for(int i = 0; i < countOfUnits + 1; i++)
        matrA_copy_2[i] = new double [countOfUnits];
    for(int i = 0; i < countOfUnits; i++)
        for(int j = 0; j < countOfUnits+1; j++)
                matrA_copy_2[i][j] = matrA_copy_1[i][j];

    for(int k = 0; k < countOfUnits; k++)
    {
        for(int i = 0; i < countOfUnits; i++)
            matrA_copy_1[k][i] /= matrA_copy_2[k][k];
        for(int i = k+1; i < countOfUnits; i++){
            double K = matrA_copy_1[i][k] / matrA_copy_1[k][k];
            for (int j = 0; j < countOfUnits + 1; j++)
                matrA_copy_1[i][j] = matrA_copy_1[i][j] - matrA_copy_1[k][j] * K;
        }
        for(int i = 0; i < countOfUnits; i++){
            for (int j = 0; j < countOfUnits+1; j++)
                matrA_copy_2[i][j] = matrA_copy_1[i][j];
        }
    }

    for (int k = countOfUnits-1; k > -1; k--)
    {
        for(int i = countOfUnits; i > -1; i--)
            matrA_copy_1[k][i] = matrA_copy_1[k][i]/ matrA_copy_2[k][k];
        for(int i = countOfUnits-1; i > -1; i--){
            double K = matrA_copy_1[i][k] / matrA_copy_1[k][k];
            for (int j = countOfUnits; j > -1; j--)
                matrA_copy_1[i][j] = matrA_copy_1[i][j] - matrA_copy_1[k][j]*K;
        }
    }

    for (int i = 0; i < countOfUnits; i++)
        delta[i] = matrA_copy_1[i][countOfUnits];*/
}
