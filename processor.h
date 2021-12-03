#ifndef PROCESSOR_H
#define PROCESSOR_H
#include<QString>
#include <QtXml/QtXml>
#include <QVector>

class Processor
{
public:
    Processor(int countOfRods);
    ~Processor();
    void Calculate(const QVector<QVector<double>>& Matrix);
    double* GetDeltas();
private:

    void Gauss();
    //bool isCalculated;
    int countOfNodes;
    double* delta;
};

#endif // PROCESSOR_H
