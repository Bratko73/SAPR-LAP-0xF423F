#ifndef PROCESSOR_H
#define PROCESSOR_H
#include<QString>
#include <QtXml/QtXml>
#include <QVector>

class Processor
{
public:
    Processor();
    ~Processor();
    void OpenFile(QString pathToFile);
    void CalculateDelta();
    void SetTerms(bool left =true, bool right = false);
    void AddRod();
    void ChangeRodParameter(int rodNum, char param, double value);
    void ChangeLoad(int Num,double value, bool isLinear = true);
    double GetRodParameter(int rodNum, char param);
    double GetLoad(int Num, bool isLinear = true);
    void DeleteRod();
    void Save(QString pathToFile);
private:
    void parseXML(QDomNode& node);
    QDomElement AddRodToXML(QDomDocument &doc,
                            const QString &L,
                            const QString &A,
                            const QString &E,
                            const QString &Sigma,
                            const int Number);
    void Gauss();
    bool isCalculated;
    int countOfRods;
    double* delta;
    QPair<bool,bool> terms;
    QVector<double> Length;
    QVector<double> Area;
    QVector<double> Elastic;
    QVector<double> q;
    QVector<double> F;
    QVector<double> Sigmas;
};

#endif // PROCESSOR_H
