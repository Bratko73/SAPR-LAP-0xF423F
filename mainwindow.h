#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
#include <QtXml/QtXml>
#include <QTableWidgetItem>
#include "processor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool ValidateDouble(const QString& str);
    bool ValidateDoubleGZero(const QString& str);
    bool ValidateTables();
    QDomElement AddRodToXML(QDomDocument& doc,
                        const QString& L,
                        const QString& A,
                        const QString& E,
                        const QString& Sigma,
                        const int Number);
    void saveToFile(const QString& pathToFile);
    void parseXML(QDomNode& node);
    void loadFromFile(QString& pathToFile);
private slots:
    void on_CountOfRods_valueChanged(int countOfRods);

    void on_RodsParametersTable_cellChanged(int row, int column);

    void on_LinearLoadTable_cellChanged(int row, int column);

    void on_ConcentratedLoadTable_cellChanged(int row, int column);

    void on_ProcessorButton_clicked();

    void on_termComboBox_currentIndexChanged(int index);

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

private:
    Ui::MainWindow *ui;
    Processor* proc;
};
#endif // MAINWINDOW_H
