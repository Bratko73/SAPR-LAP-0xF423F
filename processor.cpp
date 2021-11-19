#include "processor.h"
#include <QMessageBox>


Processor::Processor()
{

}

Processor::~Processor()
{
    delete[] delta;
}

void Processor::Calculate()
{

}

double* Processor::GetDeltas()
{
    return nullptr;
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
