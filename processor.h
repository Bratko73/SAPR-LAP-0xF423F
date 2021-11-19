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
    void Calculate();
    double* GetDeltas();
private:

    void Gauss();
    bool isCalculated;
    int countOfRods;
    double* delta;
};

#endif // PROCESSOR_H
