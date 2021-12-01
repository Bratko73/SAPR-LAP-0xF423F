#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
#include <QtXml/QtXml>
#include <QtWidgets>
#include "processor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void draw();

    void slotAlarmTimer();

    void on_CountOfRods_valueChanged(int countOfRods);

    void on_RodsParametersTable_cellChanged(int row, int column);

    void on_LinearLoadTable_cellChanged(int row, int column);

    void on_ConcentratedLoadTable_cellChanged(int row, int column);

    void on_ProcessorButton_clicked();

    void on_termComboBox_currentIndexChanged(int index);

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_MysteryButton_clicked();

    void on_RefreshButton_clicked();

    void on_stepSpinBox_valueChanged(int arg1);

    void on_PointButton_clicked();

private:
    Ui::MainWindow *ui;
    bool ValidateDouble(const QString& str);
    bool ValidateDoubleGZero(const QString& str);
    bool ValidateTables();
    void NxFormulas();
    double NxValue(int rod, double x);
    double SigmaValue(int rod, double x);
    void UxFormulas();
    double UxValue(int rod, double x);
    QDomElement AddRodToXML(QDomDocument& doc,
                        const QString& L,
                        const QString& A,
                        const QString& E,
                        const QString& Sigma,
                        const int Number);
    void saveToFile(const QString& pathToFile);
    void parseXML(QDomNode& node);
    void loadFromFile(QString& pathToFile);
    void LinPoly(QPolygonF &poly, double length, bool isPositive = true);
    void ConPoly(QPolygonF &poly, double length, bool isPositive = true);
    void TermPoly(QPolygonF &poly, double width, bool isLeft = true);
    void PrepareMatrix();
    void MysteryDraw();

    QGraphicsScene* graphicsScene;
    QVector<QPair<double,double>> Nx;
    QVector<QPair<QPair<double,double>,double>> Ux;
    QVector<QVector<double>> matrix;
    QVector<double> results;
    Processor* processor;
    QGraphicsPixmapItem* DeltaXi;
    QTimer* timer;
    bool isCalculated;
};
#endif // MAINWINDOW_H
