#include "include/mainwindow.h"
#include "include/logger.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , prevCpuStats(Resmon::get_cpu_usage())
    , m_translator(new QTranslator(this))
{
    logL("Initializing MainWindow");
    ui->setupUi(this);

    // Language setup
    ui->languageComboBox->addItem("English", "en");
    ui->languageComboBox->addItem("Русский", "ru");
    ui->languageComboBox->addItem("Polski (wip)", "pl");
    connect(ui->languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);
    ui->languageComboBox->setCurrentIndex(0);

    // Services table setup
    servicesModel = new QStandardItemModel(this);
    servicesModel->setColumnCount(3);
    servicesModel->setHorizontalHeaderLabels({tr("Service"), tr("Description"), tr("Status")});

    servicesProxyModel = new QSortFilterProxyModel(this);
    servicesProxyModel->setSourceModel(servicesModel);
    servicesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->servicesTable->setModel(servicesProxyModel);
    ui->servicesTable->setColumnWidth(0, 300);
    ui->servicesTable->setColumnWidth(1, 338);
    ui->servicesTable->setColumnWidth(2, 68);
    ui->servicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(ui->searchServicesLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::on_searchServicesTextChanged);

    // Temp files table setup
    tempFilesModel = new QStandardItemModel(this);
    tempFilesModel->setColumnCount(1);
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    ui->tempFilesTable->setModel(tempFilesModel);
    ui->tempFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tempFilesTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Autostart table setup
    autostartModel = new QStandardItemModel(this);
    autostartModel->setColumnCount(3);
    autostartModel->setHorizontalHeaderLabels({tr("Name"), tr("Executable"), tr("Comment")});
    ui->autostartTable->setModel(autostartModel);
    ui->autostartTable->setColumnWidth(0, 256);
    ui->autostartTable->setColumnWidth(1, 255);
    ui->autostartTable->setColumnWidth(2, 255);
    ui->autostartTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Resource monitoring setup
    prevCpuStats = Resmon::get_cpu_usage();
    resourceTimer = new QTimer(this);
    connect(resourceTimer, &QTimer::timeout, this, [this]() {
        updateCpuUsage();
        updateSwapUsage();
        updateRamUsage();
        updateDiskUsage();
    });
    resourceTimer->start(1000);

    // CPU chart setup
    createCpuLoadChart();
    chartUpdateTimer = new QTimer(this);
    connect(chartUpdateTimer, &QTimer::timeout, this, [this]() {
        Resmon::CPUStats current = Resmon::get_cpu_usage();
        double usage = current.usage_percent(prevCpuStats);
        prevCpuStats = current;
        updateChart(usage);
    });
    chartUpdateTimer->start(150);

    // Populate tables
    populateServicesTable();
    populateAutostartTable();
    logL("MainWindow initialization completed");
}

MainWindow::~MainWindow()
{
    logL("MainWindow destruction started");
    delete ui;
    logL("MainWindow destroyed");
}

void MainWindow::populateServicesTable()
{
    logL("Populating services table");
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
    logL(std::format("Added {} services to table", services.size()));
}

void MainWindow::populateAutostartTable()
{
    logL("Populating autostart table");
    autostartModel->removeRows(0, autostartModel->rowCount());
    std::vector<std::string> entries = AutostartManager::listAutostartEntries();

    for (auto& entry : entries) {
        QList<QStandardItem*> rowItems;
        auto info = AutostartManager::getAutostartEntryInfo(entry);
        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(info["Name"]));
        QStandardItem *execItem = new QStandardItem(QString::fromStdString(info["Exec"]));
        QStandardItem *commentItem = new QStandardItem(QString::fromStdString(info["Comment"]));

        rowItems << nameItem << execItem << commentItem;
        autostartModel->appendRow(rowItems);
    }
    logL(std::format("Added {} autostart entries to table", entries.size()));
}

void MainWindow::on_scanTempFilesButton_clicked()
{
    logL("Scanning for temporary files");
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
            logL(std::format("Scanning directory: {}", path.string()));
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
        logL(std::format("Found {} temporary files", tempFilesModel->rowCount()));
    }
    catch (const std::exception& e) {
        logE(std::format("Error scanning temp files: {}", e.what()));
        showError(tr("Error: ") + e.what());
    }
}

void MainWindow::on_startServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No service selected for start");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();

    if (serviceName.isEmpty()) {
        logE("Invalid service selected for start");
        showError(tr("Invalid service selected"));
        return;
    }

    logL(std::format("Starting service: {}", serviceName.toStdString()));
    int result = start_service(serviceName.toStdString());
    if (result == 0) {
        logL(std::format("Successfully started service: {}", serviceName.toStdString()));
        showInfo(tr("Service started successfully"));
        populateServicesTable();
    } else {
        logE(std::format("Failed to start service: {}", serviceName.toStdString()));
        showError(tr("Failed to start service"));
    }
}

void MainWindow::on_stopServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No service selected for stop");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    if (serviceName.isEmpty()) {
        logE("Invalid service selected for stop");
        showError(tr("Invalid service selected"));
        return;
    }

    logL(std::format("Stopping service: {}", serviceName.toStdString()));
    int result = stop_service(serviceName.toStdString());
    if (result == 0) {
        logL(std::format("Successfully stopped service: {}", serviceName.toStdString()));
        showInfo(tr("Service stopped successfully"));
        populateServicesTable();
    } else {
        logE(std::format("Failed to stop service: {}", serviceName.toStdString()));
        showError(tr("Failed to stop service"));
    }
}

void MainWindow::on_enableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No service selected for enable");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    if (serviceName.isEmpty()) {
        logE("Invalid service selected for enable");
        showError(tr("Invalid service selected"));
        return;
    }

    logL(std::format("Enabling service: {}", serviceName.toStdString()));
    int result = enable_service(serviceName.toStdString());
    if (result == 0) {
        logL(std::format("Successfully enabled service: {}", serviceName.toStdString()));
        showInfo(tr("Service enabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("Failed to enable service: {}", serviceName.toStdString()));
        showError(tr("Failed to enable service"));
    }
}

void MainWindow::on_disableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No service selected for disable");
        showError(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = servicesProxyModel->mapToSource(selected.first());
    QString serviceName = servicesModel->item(sourceIndex.row(), 0)->text();
    if (serviceName.isEmpty()) {
        logE("Invalid service selected for disable");
        showError(tr("Invalid service selected"));
        return;
    }

    logL(std::format("Disabling service: {}", serviceName.toStdString()));
    int result = disable_service(serviceName.toStdString());
    if (result == 0) {
        logL(std::format("Successfully disabled service: {}", serviceName.toStdString()));
        showInfo(tr("Service disabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("Failed to disable service: {}", serviceName.toStdString()));
        showError(tr("Failed to disable service"));
    }
}

void MainWindow::on_refreshServicesButton_clicked()
{
    logL("Refreshing services table");
    populateServicesTable();
    showInfo(tr("Service list refreshed"));
}

void MainWindow::on_selectAllFilesButton_clicked()
{
    logL("Selecting all temporary files");
    ui->tempFilesTable->selectAll();
}

void MainWindow::on_clearSelectedFilesButton_clicked()
{
    logL("Clearing temporary files selection");
    ui->tempFilesTable->clearSelection();
}

void MainWindow::on_deleteSelectedFilesButton_clicked()
{
    QModelIndexList selected = ui->tempFilesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No files selected for deletion");
        showError(tr("Please select files to delete"));
        return;
    }

    logL(std::format("Attempting to delete {} files", selected.size()));
    int deletedCount = 0;
    int failedCount = 0;

    for (const QModelIndex &index : selected) {
        QString filePath = tempFilesModel->item(index.row())->text();
        try {
            if (std::filesystem::remove(filePath.toStdString())) {
                deletedCount++;
                logL(std::format("Deleted file: {}", filePath.toStdString()));
            } else {
                failedCount++;
                logE(std::format("Failed to delete file: {}", filePath.toStdString()));
            }
        } catch (const std::exception& e) {
            failedCount++;
            logE(std::format("Exception when deleting {}: {}", filePath.toStdString(), e.what()));
        }
    }

    for (int i = tempFilesModel->rowCount() - 1; i >= 0; --i) {
        if (ui->tempFilesTable->selectionModel()->isRowSelected(i, QModelIndex())) {
            tempFilesModel->removeRow(i);
        }
    }

    showInfo(tr("Deleted %1 files, failed to delete %2 files").arg(deletedCount).arg(failedCount));
    logL(std::format("Deletion completed: {} success, {} failures", deletedCount, failedCount));
}

void MainWindow::on_searchServicesTextChanged(const QString &text)
{
    logL(std::format("Filtering services with text: {}", text.toStdString()));
    servicesProxyModel->setFilterKeyColumn(-1);
    servicesProxyModel->setFilterFixedString(text);
}

void MainWindow::on_addEntryButton_clicked()
{
    logL("Showing add autostart entry dialog");
    AddAutostartDialog dialog(this);
    dialog.exec();
    QString name = dialog.getName();
    QString exec = dialog.getExec();
    QString comment = dialog.getComment();

    if (name.isEmpty() || exec.isEmpty()) {
        logE("Empty name or executable when adding autostart entry");
        showError(tr("Name and Executable fields are required"));
        return;
    }

    logL(std::format("Adding autostart entry: {}", name.toStdString()));
    bool success = AutostartManager::addAutostartEntry(
        name.toStdString(),
        exec.toStdString(),
        comment.toStdString());

    if (success) {
        logL(std::format("Successfully added autostart entry: {}", name.toStdString()));
        showInfo(tr("Entry %1 has been added successfully").arg(name));
        populateAutostartTable();
    } else {
        logE(std::format("Failed to add autostart entry: {}", name.toStdString()));
        showError(tr("Entry %1 has not been added").arg(name));
    }
}

void MainWindow::on_removeEntryButton_clicked()
{
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No autostart entry selected for removal");
        showError(tr("Please select an entry first!"));
        return;
    }

    QString name = autostartModel->item(selected.first().row(), 0)->text();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm Removal"),
                                  tr("Are you sure you want to remove '%1'?").arg(name),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        logL(std::format("Removing autostart entry: {}", name.toStdString()));
        bool success = AutostartManager::removeAutostartEntry(name.toStdString());
        if (success) {
            logL(std::format("Successfully removed autostart entry: {}", name.toStdString()));
            showInfo(tr("Entry %1 removed successfully").arg(name));
            populateAutostartTable();
        } else {
            logE(std::format("Failed to remove autostart entry: {}", name.toStdString()));
            showError(tr("Failed to remove autostart entry %1").arg(name));
        }
    } else {
        logL("Autostart entry removal cancelled by user");
    }
}

void MainWindow::on_enableEntryButton_clicked()
{
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("No autostart entry selected for enable/disable");
        showError(tr("Please select an entry first!"));
        return;
    }

    QString name = autostartModel->item(selected.first().row(), 0)->text();
    auto info = AutostartManager::getAutostartEntryInfo(name.toStdString());
    bool statusBool = (QString::fromStdString(info["Status"]).toLower() == "true");

    logL(std::format("Setting autostart entry {} to {}", name.toStdString(), !statusBool));
    bool success = AutostartManager::setAutostartEntryEnabledStatus(name.toStdString(), !statusBool);
    if (success) {
        logL(std::format("Successfully changed status for autostart entry: {}", name.toStdString()));
        showInfo(tr("Entry %1 has been %2!").arg(name).arg(statusBool ? "disabled" : "enabled"));
        populateAutostartTable();
    } else {
        logE(std::format("Failed to change status for autostart entry: {}", name.toStdString()));
        showError(tr("Error occurred"));
    }
}

void MainWindow::on_updateEntriesButton_clicked()
{
    logL("Refreshing autostart entries");
    populateAutostartTable();
    showInfo(tr("Autostart Entries has been reloaded!"));
}

void MainWindow::createCpuLoadChart()
{
    logL("Creating CPU load chart");
    chart = new QChart();
    series = new QSplineSeries();
    chart->addSeries(series);
    chart->setTitle(tr("CPU Load Graph"));
    chart->setTitleFont(QFont("Ubuntu", 11));
    chart->setTitleBrush(QBrush(QColor("white")));
    axisX = new QValueAxis();
    axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisX->setGridLineVisible(false);
    axisY->setGridLineColor(QColor("#3d3d3d"));
    axisX->setLabelsBrush(QBrush(QColor("white")));
    axisY->setLabelsBrush(QBrush(QColor("white")));
    axisY->setLabelFormat(QString("%d%%"));
    axisX->setLabelsVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignRight);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->setBackgroundBrush(Qt::NoBrush);
    chart->setBackgroundVisible(false);
    chart->legend()->hide();

    chartView = new QChartView(chart);
    ui->verticalLayout_10->addWidget(chartView);
}

void MainWindow::updateChart(double usage)
{
    static int x = 0;
    const int xlim = 500;
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

    //logL(std::format("CPU usage updated: {:.1f}%", usage));
    ui->cpuUsageBar->setValue(static_cast<int>(usage));
    ui->cpuUsageLabel->setText(tr("%1%").arg(usage, 0, 'f', 1));
}

void MainWindow::updateRamUsage()
{
    Resmon::MemStats mem = Resmon::get_mem_usage();
    double usage = mem.usage_percent();

    //logL(std::format("RAM usage updated: {:.1f}%", usage));
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

    //logL(std::format("Swap usage updated: {:.1f}%", usage));
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

    //logL(std::format("Disk usage updated: {:.1f}%", usage));
    ui->diskUsageBar->setValue(static_cast<int>(usage));
    ui->diskUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                    .arg(usage, 0, 'f', 1)
                                    .arg((disk.used) / (1024 * 1024))
                                    .arg(disk.total / (1024 * 1024)));
}

void MainWindow::onLanguageChanged(int index)
{
    QString languageCode = ui->languageComboBox->itemData(index).toString();
    QString localeCode;


    if (languageCode == "en") {
        localeCode = "en_US";
    } else if (languageCode == "ru") {
        localeCode = "ru_RU";
    } else if (languageCode == "pl") {
        localeCode = "pl_PL";
    } else {
        localeCode = "en_US";
    }

    logL(std::format("Changing language to: {}", localeCode.toStdString()));

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = new QTranslator(this);

    QString translationFile = QString(":translations/qtguiinterface_%1.qm").arg(localeCode);
    logL(std::format("Loading translation file: {}", translationFile.toStdString()));

    if (m_translator->load(translationFile)) {
        qApp->installTranslator(m_translator);
        logL("Translation loaded successfully");
    } else {
        logE("Failed to load translation");
    }

    ui->retranslateUi(this);

    servicesModel->setHorizontalHeaderLabels({tr("Service"), tr("Description"), tr("Status")});
    autostartModel->setHorizontalHeaderLabels({tr("Name"), tr("Executable"), tr("Comment")});
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    chart->setTitle(tr("CPU Load Graph"));
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
