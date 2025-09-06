#include <QStandardItemModel>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QMessageBox>
#include <QFileDialog>
#include <QFont>
#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include <QIcon>
#include <QRandomGenerator>
#include <QProcess>


#include "../include/services.hpp"
#include "../include/temp_files.hpp"
#include "../include/autostart.hpp"
#include "../include/addautostartdialog.hpp"
#include "../include/mainwindow.h"
#include "ui_mainwindow.h"
#include "../include/logger.hpp"
#include "../include/config_manager.hpp"
#include "../include/start_new_process_dialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , prevCpuStats(Resmon::get_cpu_usage())
    , lastNetworkStats(Resmon::get_internet_usage())

{
    logL("MainWindow: Initializing MainWindow");
    ui->setupUi(this);
    config = std::make_unique<Config>();
    config->load();

    ui->themeComboBox->addItem(tr("Dark Theme"), "dark");
    ui->themeComboBox->addItem(tr("Light Theme"), "light");
    connect(ui->themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);
    ui->themeComboBox->setCurrentIndex(0);

    servicesModel = std::make_unique<QStandardItemModel>(this);
    servicesModel->setColumnCount(3);
    servicesModel->setHorizontalHeaderLabels({tr("Service"), tr("Description"), tr("Status")});

    servicesProxyModel = std::make_unique<QSortFilterProxyModel>(this);
    servicesProxyModel->setSourceModel(servicesModel.get());
    servicesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->servicesTable->setModel(servicesProxyModel.get());
    //ui->servicesTable->setColumnWidth(0, 300);
    //ui->servicesTable->setColumnWidth(1, 338);
    //ui->servicesTable->setColumnWidth(2, 68);

    ui->servicesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->servicesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->servicesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->servicesTable->setColumnWidth(2, 100);
    ui->servicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(ui->searchServicesLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::on_searchServicesTextChanged);

    tempFilesModel = std::make_unique<QStandardItemModel>(this);
    tempFilesModel->setColumnCount(1);
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    ui->tempFilesTable->setModel(tempFilesModel.get());
    ui->tempFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tempFilesTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    autostartModel = std::make_unique<QStandardItemModel>(this);
    autostartModel->setColumnCount(3);
    autostartModel->setHorizontalHeaderLabels({tr("Name"), tr("Executable"), tr("Status"), tr("Comment")});
    ui->autostartTable->setModel(autostartModel.get());
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->autostartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    prevCpuStats = Resmon::get_cpu_usage();

    //taskManagerModel = std::make_unique<QStandardItemModel>(this);
    //taskManagerModel->setColumnCount(5);
    //taskManagerModel->setHorizontalHeaderLabels({"PID", "Name", "CPU Load", "Physical RAM", "Virtual RAM"});
    //taskManagerProxyModel = std::make_unique<QSortFilterProxyModel>(this);
    //taskManagerProxyModel->setSourceModel(taskManagerModel.get());
    //taskManagerProxyModel-> setFilterCaseSensitivity(Qt::CaseInsensitive);

    //taskManagerProxyModel = std::make_unique<QSortFilterProxyModel>(this);
    //taskManagerProxyModel->setSourceModel(taskManagerModel.get());
    //taskManagerProxyModel->setFilterKeyColumn(1);
    //taskManagerProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    //taskManagerProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    resourceTimer = std::make_unique<QTimer>(this);
    connect(resourceTimer.get(), &QTimer::timeout, this, [this]() {
        updateCpuUsage();
        updateSwapUsage();
        updateRamUsage();
        updateDiskUsage();
        updateInternetUsage();
    });
    //не менять сломается инет
    resourceTimer->start(1000);

    createCpuLoadChart();
    loadTheme(QString::fromStdString(config->getTheme()));
    chartUpdateTimer = std::make_unique<QTimer>(this);
    connect(chartUpdateTimer.get(), &QTimer::timeout, this, [this]() {
        Resmon::CPUStats current = Resmon::get_cpu_usage();
        double usage = current.usage_percent(prevCpuStats);
        prevCpuStats = current;
        updateChart(usage);
    });
    chartUpdateTimer->start(700);


    //populateTaskManager();
    populateServicesTable();
    populateAutostartTable();
    ui->cpuGroupBox->setTitle(tr("CPU Usage (%1)").arg(QString::fromStdString(Resmon::get_cpu_name())));
    ui->networkGroupBox->setTitle(tr("Network Usage (%1)").arg(QString::fromStdString(Resmon::get_network_interface())));

    logL("MainWindow: MainWindow initialization completed");
}

MainWindow::~MainWindow() {
    logL("MainWindow: Cleanup started");
    //resourceTimer->stop();
    //chartUpdateTimer->stop();
    logL("MainWindow: Cleanup completed\n");
}

void MainWindow::populateServicesTable()
{
    logL("MainWindow: Populating services table");
    servicesModel->removeRows(0, servicesModel->rowCount());
    auto services = get_services();
    QFont font;

    for (const auto& service : services) {
        QList<QStandardItem*> rowItems;

        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(service.name));
        QStandardItem *descItem = new QStandardItem(QString::fromStdString(service.description));
        QStandardItem *statusItem = new QStandardItem(QString::fromStdString(service.status));

        if (service.status == "active") {
            statusItem->setForeground(QBrush(Qt::green));
        } else if (service.status == "failed") {
            statusItem->setForeground(QBrush(Qt::red));
        }

        rowItems << nameItem << descItem << statusItem;
        servicesModel->appendRow(rowItems);

        for (auto& item : rowItems){
            item->setFont(font);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }
    logL(std::format("MainWindow: Added {} services to table", services.size()));
}

//void MainWindow::populateTaskManager()
//{
//    logL("MainWindow: Populating Task Manager");
//
//    taskManagerModel->removeRows(0, taskManagerModel->rowCount());
//
//    auto tasks = TaskManager::getAllProcessesInfo();
//
//    //taskManagerModel->setRowCount(tasks.size());
//
//    for (int i = 0; i < tasks.size(); ++i) {
//        const auto& task = tasks[i];
//
//        QStandardItem *pidItem = new QStandardItem(QString::fromStdString(std::to_string(task.pid)));
//        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(task.name));
//        QStandardItem *cpuloadItem = new QStandardItem(QString::fromStdString(std::to_string(task.cpu_usage)));
//        QStandardItem *PRAMItem = new QStandardItem(QString::fromStdString(std::to_string(task.memory_rss_kb)));
//        QStandardItem *VRAMItem = new QStandardItem(QString::fromStdString(std::to_string(task.memory_vsize_kb)));
//
//        QList<QStandardItem*> rowItems = {pidItem, nameItem, cpuloadItem, PRAMItem, VRAMItem};
//        for (auto& item : rowItems) {
//            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
//            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
//        }
//
//        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//
//        taskManagerModel->appendRow(rowItems);
//    }
//
//    logL(std::format("MainWindow: Added {} tasks to table", tasks.size()));
//}

void MainWindow::populateAutostartTable()
{
    logL("MainWindow: Populating autostart table");
    autostartModel->removeRows(0, autostartModel->rowCount());
    std::vector<std::string> entries = AutostartManager::listAutostartEntries();

    for (auto& entry : entries) {
        QList<QStandardItem*> rowItems;
        auto info = AutostartManager::getAutostartEntryInfo(entry);
        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(info["Name"]));
        QStandardItem *execItem = new QStandardItem(QString::fromStdString(info["Exec"]));
        QStandardItem *statusItem = new QStandardItem(QString::fromStdString(info["Status"]));
        QStandardItem *commentItem = new QStandardItem(QString::fromStdString(info["Comment"]));

        rowItems << nameItem << execItem << statusItem << commentItem;
        autostartModel->appendRow(rowItems);
    }
    logL(std::format("MainWindow: Added {} autostart entries to table", entries.size()));
}

void MainWindow::on_scanTempFilesButton_clicked()
{
    logL("MainWindow: Scanning for temporary files");
    tempFilesModel->removeRows(0, tempFilesModel->rowCount());

    try {
        std::filesystem::path home = get_home_directory();
        std::vector<std::filesystem::path> tempfiles_paths = {
            "/tmp",
            "/var/tmp",
            home / ".cache",
            home / ".local/share/Trash"
        };

        for (auto& path : tempfiles_paths) {
            logL(std::format("MainWindow: Scanning directory: {}", path.string()));
            auto folder_contents = get_recursive_folder_content(path);

            if (!folder_contents) {
                logE(folder_contents.error());
                showError(QString::fromStdString(folder_contents.error()));
                continue;
            }

            for (const auto& file : *folder_contents) {
                tempFilesModel->appendRow(new QStandardItem(QString::fromStdString(file.string())));
            }
        }

        showInfo(tr("Found %1 temporary files").arg(tempFilesModel->rowCount()));
        logL(std::format("MainWindow: Found {} temporary files", tempFilesModel->rowCount()));
    }
    catch (const std::exception& e) {
        logE(std::format("MainWindow: Error scanning temp files: {}", e.what()));
        showError(tr("Error: ") + e.what());
    }
}

void MainWindow::on_startServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: No service selected for start");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    QString serviceStatus = servicesModel->item(sourceIndex.row(), 2)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("MainWindow: Invalid service selected for start");
        showError(tr("Invalid service selected"));
        return;
    }

    if (serviceStatus.compare("active", Qt::CaseInsensitive) != 0) {
        logL(std::format("MainWindow: Starting service: {}", serviceName.toStdString()));
        result = start_service(serviceName.toStdString());
    }
    else {
        showInfo(tr("MainWindow: Service %1 is already started").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("MainWindow: Successfully started service: {}", serviceName.toStdString()));
        showInfo(tr("Service started successfully"));
        populateServicesTable();
    } else {
        logE(std::format("MainWindow: Failed to start service: {}", serviceName.toStdString()));
        showError(tr("Failed to start service"));
    }
}

void MainWindow::on_stopServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: No service selected for stop");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    QString serviceStatus = servicesModel->item(sourceIndex.row(), 2)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("MainWindow: Invalid service selected for stop");
        showError(tr("Invalid service selected"));
        return;
    }

    if (serviceStatus.compare("active", Qt::CaseInsensitive) == 0) {
        logL(std::format("MainWindow: Stopping service: {}", serviceName.toStdString()));
        result = stop_service(serviceName.toStdString());
    }
    else {
        showInfo(tr("MainWindow: Service %1 is already stopped").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("MainWindow: Successfully stopped service: {}", serviceName.toStdString()));
        showInfo(tr("Service stopped successfully"));
        populateServicesTable();
    } else {
        logE(std::format("MainWindow: Failed to stop service: {}", serviceName.toStdString()));
        showError(tr("Failed to stop service"));
    }
}

void MainWindow::on_enableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: No service selected for enable");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("MainWindow: Invalid service selected for enable");
        showError(tr("Invalid service selected"));
        return;
    }
    if (!is_service_enabled(serviceName.toStdString())) {
        logL(std::format("MainWindow: Enabling service: {}", serviceName.toStdString()));
        result = enable_service(serviceName.toStdString());
    }
    else {
        showInfo(tr("MainWindow: Service %1 is already enabled").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("MainWindow: Successfully enabled service: {}", serviceName.toStdString()));
        showInfo(tr("Service enabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("MainWindow: Failed to enable service: {}", serviceName.toStdString()));
        showError(tr("Failed to enable service"));
    }
}

void MainWindow::on_disableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: No service selected for disable");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("MainWindow: Invalid service selected for disable");
        showError(tr("Invalid service selected"));
        return;
    }
    if (is_service_enabled(serviceName.toStdString())) {
        logL(std::format("MainWindow: Disabling service: {}", serviceName.toStdString()));
        result = disable_service(serviceName.toStdString());
    }
    else {
        showInfo(tr("MainWindow: Service %1 is already disabled").arg(serviceName));
        return;
    }
    if (result == 0) {
        logL(std::format("MainWindow: Successfully disabled service: {}", serviceName.toStdString()));
        showInfo(tr("Service disabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("MainWindow: Failed to disable service: {}", serviceName.toStdString()));
        showError(tr("Failed to disable service"));
    }
}

void MainWindow::on_refreshServicesButton_clicked()
{
    logL("MainWindow: Refreshing services table");
    populateServicesTable();
    showInfo(tr("Service list refreshed"));
}

void MainWindow::on_selectAllFilesButton_clicked()
{
    logL("MainWindow: Selecting all temporary files");
    ui->tempFilesTable->selectAll();
}

void MainWindow::on_clearSelectedFilesButton_clicked()
{
    logL("MainWindow: Clearing temporary files selection");
    ui->tempFilesTable->clearSelection();
}

void MainWindow::on_catButton_clicked(){
    ui->catButton->setMaximumWidth(64);
    ui->catButton->setText("");

    const int maxCats = 34;
    int randomIndex = QRandomGenerator::global()->bounded(maxCats);

    QString imagePath = QString(":cats/cat%1.jpg").arg(randomIndex);
    logL(imagePath.toStdString());
    ui->catButton->setIcon(QIcon(imagePath));
}


void MainWindow::on_deleteSelectedFilesButton_clicked()
{
    QModelIndexList selected = ui->tempFilesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: No files selected for deletion");
        showError(tr("Please select files to delete"));
        return;
    }

    logL(std::format("MainWindow: Attempting to delete {} files", selected.size()));
    int deletedCount = 0;
    int failedCount = 0;

    for (const QModelIndex &index : selected) {
        QString filePath = tempFilesModel->item(index.row())->text();
        try {
            if (std::filesystem::remove(filePath.toStdString())) {
                deletedCount++;
                //logL(std::format("MainWindow: Deleted file: {}", filePath.toStdString()));
            } else {
                failedCount++;
                logE(std::format("MainWindow: Failed to delete file: {}", filePath.toStdString()));
            }
        } catch (const std::exception& e) {
            failedCount++;
            logE(std::format("MainWindow: Exception when deleting {}: {}", filePath.toStdString(), e.what()));
        }
    }

    for (int i = tempFilesModel->rowCount() - 1; i >= 0; --i) {
        if (ui->tempFilesTable->selectionModel()->isRowSelected(i, QModelIndex())) {
            tempFilesModel->removeRow(i);
        }
    }

    showInfo(tr("Deleted %1 files, failed to delete %2 files").arg(deletedCount).arg(failedCount));
    logL(std::format("MainWindow: Deletion completed: {} success, {} failures", deletedCount, failedCount));
}

void MainWindow::on_searchServicesTextChanged(const QString &text)
{
    logL(std::format("MainWindow: Filtering services with text: {}", text.toStdString()));
    servicesProxyModel->setFilterKeyColumn(-1);
    servicesProxyModel->setFilterFixedString(text);
}

void MainWindow::on_searchTaskLineEdit_textChanged(const QString &text)
{
    logL(std::format("MainWindow: Filtering tasks with text: {}", text.toStdString()));
    taskManagerProxyModel->setFilterKeyColumn(-1);
    taskManagerProxyModel->setFilterFixedString(text);
}

void MainWindow::on_addEntryButton_clicked()
{
    logL("MainWindow: Showing add autostart entry dialog");
    AddAutostartDialog dialog(this);
    dialog.exec();
    QString name = dialog.getName();
    QString exec = dialog.getExec();
    QString comment = dialog.getComment();

    if (name.isEmpty() || exec.isEmpty()) {
        logE("MainWindow: MainWindow: Empty name or executable when adding autostart entry");
        showError(tr("Name and Executable fields are required"));
        return;
    }

    logL(std::format("MainWindow: Adding autostart entry: {}", name.toStdString()));
    bool success = AutostartManager::addAutostartEntry(
        name.toStdString(),
        exec.toStdString(),
        comment.toStdString());

    if (success) {
        logL(std::format("MainWindow: Successfully added autostart entry: {}", name.toStdString()));
        showInfo(tr("Entry %1 has been added successfully").arg(name));
        populateAutostartTable();
    } else {
        logE(std::format("MainWindow: Failed to add autostart entry: {}", name.toStdString()));
        showError(tr("Entry %1 has not been added").arg(name));
    }
}

void MainWindow::on_removeEntryButton_clicked()
{
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: MainWindow: No autostart entry selected for removal");
        showError(tr("Please select an entry first!"));
        return;
    }

    QString name = autostartModel->item(selected.first().row(), 0)->text();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm Removal"),
                                  tr("Are you sure you want to remove '%1'?").arg(name),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        logL(std::format("MainWindow: Removing autostart entry: {}", name.toStdString()));
        bool success = AutostartManager::removeAutostartEntry(name.toStdString());
        if (success) {
            logL(std::format("MainWindow: Successfully removed autostart entry: {}", name.toStdString()));
            showInfo(tr("Entry %1 removed successfully").arg(name));
            populateAutostartTable();
        } else {
            logE(std::format("MainWindow: Failed to remove autostart entry: {}", name.toStdString()));
            showError(tr("Failed to remove autostart entry %1").arg(name));
        }
    } else {
        logL("MainWindow: MainWindow: Autostart entry removal cancelled by user");
    }
}

void MainWindow::on_enableEntryButton_clicked()
{
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("MainWindow: MainWindow: No autostart entry selected for enable/disable");
        showError(tr("Please select an entry first!"));
        return;
    }

    QString name = autostartModel->item(selected.first().row(), 0)->text();
    auto info = AutostartManager::getAutostartEntryInfo(name.toStdString());
    bool statusBool = (QString::fromStdString(info["Status"]).toLower() == "true");

    logL(std::format("MainWindow: Setting autostart entry {} to {}", name.toStdString(), !statusBool));
    bool success = AutostartManager::setAutostartEntryEnabledStatus(name.toStdString(), !statusBool);
    if (success) {
        logL(std::format("MainWindow: Successfully changed status for autostart entry: {}", name.toStdString()));
        showInfo(tr("Entry %1 has been %2!").arg(name).arg(statusBool ? tr("disabled") : tr("enabled")));
        populateAutostartTable();
    } else {
        logE(std::format("MainWindow: Failed to change status for autostart entry: {}", name.toStdString()));
        showError(tr("Error occurred"));
    }
}

void MainWindow::on_updateEntriesButton_clicked()
{
    logL("MainWindow: Refreshing autostart entries");
    populateAutostartTable();
    showInfo(tr("Autostart Entries has been reloaded!"));
}


void MainWindow::on_startProcessButton_clicked() {
    StartNewProcess procDialog(this);
    procDialog.exec();
    QString command = procDialog.getCommand();
    bool isRoot = procDialog.isExecuteAsRoot();

    if (command.isEmpty()) {
        return;
    }

    QProcess process;
    QStringList arguments;

    if (isRoot) {
        arguments << "-E" << command;
        process.start("sudo", arguments);
    } else {
        arguments << "-u" << "#1000" << "sh" << "-c" << command;
        process.start("sudo", arguments);
    }

    if (!process.waitForStarted()) {
        showError("Error. Failed to start process!");
        return;
    }

    //process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        showInfo("Process executed successfully!");
    } else {
        QMessageBox::critical(this, "Error",
                              QString("Process failed with exit code: %1\nError: %2")
                                  .arg(process.exitCode())
                                  .arg(QString(process.readAllStandardError())));
    }
}

void MainWindow::on_stopProcessButton_clicked(){

}
//void MainWindow::on_stopProcessButton_clicked() {
//    QModelIndexList selected = ui->taskManagerTable->selectionModel()->selectedRows();
//    if (selected.isEmpty()) {
//        logE("MainWindow: No process was selected to stop");
//        showError(tr("Please select a process first!"));
//        return;
//    }
//    QModelIndex sourceIndex = taskManagerProxyModel->mapToSource(selected.first());
//
//    QString pidStr = taskManagerModel->item(sourceIndex.row(), 0)->text();
//    logF(pidStr.toStdString());
//    bool ok;
//    int pid = pidStr.toInt(&ok);
//
//    if (!ok || pid <= 0) {
//        logE(("MainWindow: Invalid PID format: " + pidStr).toStdString());
//        showError(tr("Invalid process ID"));
//        return;
//    }
//
//    QMessageBox::StandardButton reply = QMessageBox::question(
//        this,
//        tr("Confirm Process Termination"),
//        tr("Are you sure you want to terminate process %1?").arg(pid),
//        QMessageBox::Yes | QMessageBox::No
//        );
//
//    if (reply != QMessageBox::Yes) {
//        return;
//    }
//
//    QProcess process;
//    QStringList args;
//    args << "-15" << QString::number(pid);
//
//    process.start("kill", args);
//
//    if (!process.waitForFinished(2000)) {
//        logL("MainWindow: Process didn't respond to SIGTERM, trying SIGKILL");
//
//        QProcess killProcess;
//        QStringList killArgs;
//        killArgs << "-9" << QString::number(pid);
//        killProcess.start("kill", killArgs);
//        killProcess.waitForFinished(1000);
//    }
//
//    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
//        logL(("MainWindow: Successfully terminated process: " + pidStr).toStdString());
//        showInfo(tr("Process terminated successfully"));
//    } else {
//        logE(("MainWindow: Failed to terminate process: " + pidStr).toStdString());
//        showError(tr("Failed to terminate process"));
//    }
//
//    populateTaskManager();
//}

void MainWindow::createCpuLoadChart()
{
    logL("MainWindow: Creating CPU load chart");
    chart = new QChart();
    series = new QSplineSeries();
    chart->addSeries(series);
    chart->setTitle(tr("CPU Load Graph"));
    chart->setMinimumHeight(200);
    chart->setMaximumHeight(300);
    //chart->setTitleBrush();
    axisX = new QValueAxis();
    axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisX->setGridLineVisible(false);

    axisY->setLabelFormat(QString("%d%%"));
    axisX->setLabelsVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignRight);

    series->attachAxis(axisX);
    series->attachAxis(axisY);
    series->setColor(QColor(76, 175, 80));
    chart->setBackgroundBrush(Qt::NoBrush);
    chart->setBackgroundVisible(false);
    chart->legend()->hide();

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout_10->addWidget(chartView);
}

void MainWindow::updateChart(double usage)
{
    static int x = 0;
    const int xlim = 50;
    series->append(x++, usage);

    if (series->count() > xlim) {
        series->remove(0);
    }

    axisX->setRange(x-xlim, x);
}

void MainWindow::updateCpuUsage()
{
    Resmon::CPUStats current = Resmon::get_cpu_usage();
    double usage = current.usage_percent(prevCpuStats);
    prevCpuStats = current;

    //logL(std::format("MainWindow: CPU usage updated: {:.1f}%", usage));
    ui->cpuUsageBar->setValue(static_cast<int>(usage));
    ui->cpuUsageLabel->setText(tr("%1%").arg(usage, 0, 'f', 1));
}

void MainWindow::updateRamUsage()
{
    Resmon::MemStats mem = Resmon::get_mem_usage();
    double usage = mem.usage_percent();

    //logL(std::format("MainWindow: RAM usage updated: {:.1f}%", usage));
    ui->ramUsageBar->setValue(static_cast<int>(usage));
    ui->ramUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                   .arg(usage, 0, 'f', 1)
                                   .arg((mem.total - mem.available) / (1024 * 1024))
                                   .arg(mem.total / (1024 * 1024)));
}

void MainWindow::updateSwapUsage()
{
    Resmon::MemStats swap = Resmon::get_mem_usage();
    double usage = swap.swap_usage_percent();

    //logL(std::format("MainWindow: Swap usage updated: {:.1f}%", usage));
    ui->swapUsageBar->setValue(static_cast<int>(usage));
    ui->swapUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                    .arg(usage, 0, 'f', 1)
                                    .arg((swap.swaptotal - swap.swapfree) / (1024 * 1024))
                                    .arg(swap.swaptotal / (1024 * 1024)));
}

void MainWindow::updateDiskUsage()
{
    Resmon::DiskStats disk = Resmon::get_disk_usage();
    double usage = disk.usage_percent();

    //logL(std::format("MainWindow: Disk usage updated: {:.1f}%", usage));
    ui->diskUsageBar->setValue(static_cast<int>(usage));
    ui->diskUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                    .arg(usage, 0, 'f', 1)
                                    .arg((disk.used) / (1024 * 1024))
                                    .arg(disk.total / (1024 * 1024)));
}



void MainWindow::updateInternetUsage(){
    Resmon::NetworkStats network = Resmon::get_internet_usage();
    const QStringList units = {"KB/s", "MB/s", "GB/s"};
    auto formatSpeed = [&](double speed){
        int unitIndex = 0;
        while (speed >= 1024.0 && unitIndex < units.size() - 1) {
            speed /= 1024.0;
            unitIndex++;
        }

        return QString("%1 %2").arg(QString::number(speed, 'f', 2)).arg(units[unitIndex]);
    };
    ui->downloadLabel->setText(tr("Download: %1").arg(formatSpeed(network.rx_speed)));
    ui->uploadLabel->setText(tr("Upload: %1").arg(formatSpeed(network.tx_speed)));
}

void MainWindow::loadTheme(const QString& themeName) {
    QString path = QString(":/%1.qss").arg(themeName);
    QFile styleFile(path);
    int index = ui->themeComboBox->findData(themeName);
    if (index != -1) {
        ui->themeComboBox->setCurrentIndex(index);
    } else {
        ui->themeComboBox->setCurrentIndex(0);
    }

    if (styleFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = QString(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
        config->setTheme(themeName.toStdString());
        config->save();
        if (chart) {
            QColor textColor = (themeName == "dark") ? Qt::white : Qt::black;
            chart->setTitleBrush(QBrush(textColor));
            axisY->setLabelsColor(textColor);
        }
    } else {
        logE("Failed to load theme: " + path.toStdString());
    }
}

void MainWindow::onThemeChanged(int index){
    QString themeName = ui->themeComboBox->itemData(index).toString();
    logF(themeName.toStdString());
    loadTheme(themeName);
}

void MainWindow::showError(const QString &message)
{
    logE(message.toStdString());
    QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::showInfo(const QString &message)
{
    logL(message.toStdString());
    QMessageBox::information(this, tr("Information"), message);
}
