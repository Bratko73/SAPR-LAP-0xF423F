#include "processor.h"
#include <QMessageBox>


Processor::Processor(int countOfRods)
    :countOfNodes(countOfRods+1){
    /*delta = new double[countOfNodes];
    for(int i = 0; i < countOfNodes; i++)
        delta[i]=0;*/
}

Processor::~Processor()
{
    delete[] delta;
}

void Processor::Calculate(const QVector<QVector<double>>& Matrix)
{
    double** matr= new double*[countOfNodes];
    for (int i = 0; i < countOfNodes; i++)
        matr[i] = new double[countOfNodes+1];

    for(int i = 0; i < countOfNodes; i++)
        for(int j = 0; j < countOfNodes+1; j++)
            matr[i][j] = Matrix[i][j];

    double tmp;
    delta = new double[countOfNodes];
    for (int i = 0; i< countOfNodes; i++)
    {
        tmp = matr[i][i];
        for (int j = countOfNodes; j >= i; j--)
            matr[i][j] /= tmp;
        for (int j = i + 1; j < countOfNodes; j++)
        {
            tmp = matr[j][i];
            for (int k = countOfNodes; k >= i; k--)
                matr[j][k] -= tmp*matr[i][k];
        }
    }
    //обратный ход
    delta[countOfNodes - 1] = matr[countOfNodes - 1][countOfNodes];
    for (int i = countOfNodes - 2; i >= 0; i--)
    {
        delta[i] = matr[i][countOfNodes];
        for (int j = i + 1; j< countOfNodes; j++)
            delta[i] -= matr[i][j] * delta[j];
    }

}

double* Processor::GetDeltas()
{
    return delta;
}



void Processor::Gauss()
{
   /* //Метод Гаусса
        //Прямой ход, приведение к верхнетреугольному виду
        float  tmp;
        int k;
        float *xx = new float[m];

        for (i = 0; i<n; i++)
        {
            tmp = matrix[i][i];
            for (j = n; j >= i; j--)
                matrix[i][j] /= tmp;
            for (j = i + 1; j<n; j++)
            {
                tmp = matrix[j][i];
                for (k = n; k >= i; k--)
                    matrix[j][k] -= tmp*matrix[i][k];
            }
        }
        обратный ход
        xx[n - 1] = matrix[n - 1][n];
        for (i = n - 2; i >= 0; i--)
        {
            xx[i] = matrix[i][n];
            for (j = i + 1; j<n; j++) xx[i] -= matrix[i][j] * xx[j];
        }

        //Выводим решения
        for (i = 0; i<n; i++)
            cout << xx[i] << " ";
        cout << endl;*/
}
