#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <memory>
#include <QTranslator>
#include "resource_monitor.hpp"

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
class AutostartController;
class TaskManagerController;
class ServicesController;
class TempfilesController;
class Config;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_catButton_clicked();
    void onThemeChanged(int index);
    void onLanguageChanged(int index);

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
    std::unique_ptr<QTranslator> translator;
    std::unique_ptr<QTranslator> m_translator;
    std::unique_ptr<ServicesController> SvcCtl;
    std::unique_ptr<TaskManagerController> TmCtl;
    std::unique_ptr<AutostartController> AutostartCtl;
    std::unique_ptr<TempfilesController> TempfilesCtl;
    void setupSlots();
    void showError(const QString &message);
    void showInfo(const QString &message);
    void updateCpuUsage();
    void updateRamUsage();
    void updateDiskUsage();
    void updateSwapUsage();
    void updateSystemInfo();
    void updateInternetUsage();
    void createCpuLoadChart();
    void setupUI();
    void setupComboBoxes();
    void setupModels();
    void setupTables();
    void setupTimers();
    void setupProxyModel(QSortFilterProxyModel *proxy, QStandardItemModel   *source);
    void loadTheme(const QString& themeName);
    void updateChart(double usage);
    void loadLanguage(const QString &localeCode);
    Resmon::CPUStats prevCpuStats;
    Resmon::NetworkStats lastNetworkStats{};
    std::unique_ptr<QTimer> resourceTimer;
    std::unique_ptr<QTimer> chartUpdateTimer;
    std::unique_ptr<QTimer> processUpdateTimer;
    std::unique_ptr<QDialog> newEntryDialog;
    std::unique_ptr<QTimer> networkTimer;
    std::unique_ptr<Config> cfg;
};



#endif // MAINWINDOW_H
