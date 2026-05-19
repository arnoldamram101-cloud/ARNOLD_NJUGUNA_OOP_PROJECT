#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QTextBrowser> // Added for the rich telemetry panel
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "circuit.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void on_changeValueBtn_clicked();

private slots:
    void on_addBatteryBtn_clicked();
    void on_addResistorBtn_clicked();
    void on_addLedBtn_clicked();
    void on_addGroundBtn_clicked();
    void on_deleteBtn_clicked();
    void on_simulationToggle_clicked();

private:
    QGraphicsScene *scene;
    QGraphicsView *graphicsView;
    QStatusBar *customStatusBar;

    // New UI Element pointers
    QTextBrowser *telemetrySidebar;
    QPushButton *simulationToggleBtn;

    Circuit coreEngine;
    bool isSimulatingActive = false;

    void updateCalculationLoop();
    void syncTelemetrySidebar(QString statusLiteral);
};

#endif // MAINWINDOW_H