#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <memory>
#include "../include/resource_monitor.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStandardItemModel;
class QMainWindow;
class QSortFilterProxyModel;
class QChart;
class QSplineSeries;
class QValueAxis;
class QChartView;

class Config;

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
    void on_searchTaskLineEdit_textChanged(const QString &text);
    void on_addEntryButton_clicked();
    void on_enableEntryButton_clicked();
    void on_removeEntryButton_clicked();
    void on_updateEntriesButton_clicked();
    void on_selectAllFilesButton_clicked();
    void on_clearSelectedFilesButton_clicked();
    void on_startProcessButton_clicked();
    void on_stopProcessButton_clicked();
    void on_catButton_clicked();
    void onThemeChanged(int index);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QStandardItemModel> servicesModel;
    std::unique_ptr<QSortFilterProxyModel> servicesProxyModel;
    std::unique_ptr<QStandardItemModel> tempFilesModel;
    std::unique_ptr<QStandardItemModel> autostartModel;
    std::unique_ptr<QStandardItemModel> taskManagerModel;
    std::unique_ptr<QSortFilterProxyModel> taskManagerProxyModel;
    QChart* chart = nullptr;
    QSplineSeries* series = nullptr;
    QValueAxis* axisX = nullptr;
    QValueAxis* axisY = nullptr;
    QChartView* chartView = nullptr;

    void populateServicesTable();
    void populateTaskManager();
    void populateAutostartTable();
    void showError(const QString &message);
    void showInfo(const QString &message);
    void updateCpuUsage();
    void updateRamUsage();
    void updateDiskUsage();
    void updateSwapUsage();
    void updateSystemInfo();
    void updateInternetUsage();
    void createCpuLoadChart();
    void loadLanguage(const QString &localeCode);
    void loadTheme(const QString& themeName);
    void updateChart(double usage);
    Resmon::CPUStats prevCpuStats;
    Resmon::NetworkStats lastNetworkStats{};
    std::unique_ptr<QTimer> resourceTimer;
    std::unique_ptr<QTimer> chartUpdateTimer;
    std::unique_ptr<QDialog> newEntryDialog;
    std::unique_ptr<QTimer> networkTimer;
    std::unique_ptr<Config> config;
};



#endif // MAINWINDOW_H
