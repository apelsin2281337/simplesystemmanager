#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QTranslator>
#include <QMessageBox>
#include <QFileDialog>
#include <QFont>
#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>

#include "include/services.hpp"
#include "include/temp_files.hpp"
#include "include/autostart.hpp"
#include "include/addautostartdialog.hpp"
#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/logger.hpp"
#include "include/resource_monitor.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_scanTempFilesButton_clicked();
    void on_startServiceButton_clicked();
    void on_stopServiceButton_clicked();
    void on_enableServiceButton_clicked();
    void on_disableServiceButton_clicked();
    void on_refreshServicesButton_clicked();
    void on_deleteSelectedFilesButton_clicked();
    void on_searchServicesTextChanged(const QString &text);
    void on_addEntryButton_clicked();
    void on_enableEntryButton_clicked();
    void on_removeEntryButton_clicked();
    void on_updateEntriesButton_clicked();
    void on_selectAllFilesButton_clicked();
    void on_clearSelectedFilesButton_clicked();
    void onLanguageChanged(int index);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *servicesModel;
    QSortFilterProxyModel *servicesProxyModel;
    QStandardItemModel *tempFilesModel;
    QStandardItemModel *autostartModel;
    QChart *chart = nullptr;
    QSplineSeries *series = nullptr;
    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    QChartView *chartView = nullptr;
    void populateServicesTable();
    void populateAutostartTable();
    void showError(const QString &message);
    void showInfo(const QString &message);
    void updateCpuUsage();
    void updateRamUsage();
    void updateDiskUsage();
    void updateSwapUsage();
    void updateSystemInfo();
    void createCpuLoadChart();
    void updateChart(double usage);
    Resmon::CPUStats prevCpuStats;
    QTimer *resourceTimer;
    QTimer* chartUpdateTimer;
    QDialog *newEntryDialog;
    QTranslator *translator;
    QTranslator *m_translator;
};



#endif // MAINWINDOW_H
