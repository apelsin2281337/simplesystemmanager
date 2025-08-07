#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QFont>
#include <QDialog>
#include "resource_monitor.hpp"
#include <QTimer>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , prevCpuStats(Resmon::get_cpu_usage())
    , m_translator(new QTranslator(this))
{

    ui->setupUi(this);
    ui->languageComboBox->addItem("English", "en");
    ui->languageComboBox->addItem("Русский", "ru");
    connect(
        ui->languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,&MainWindow::onLanguageChanged
    );

    ui->languageComboBox->setCurrentIndex(0);


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



    tempFilesModel = new QStandardItemModel(this);
    tempFilesModel->setColumnCount(1);
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    ui->tempFilesTable->setModel(tempFilesModel);
    ui->tempFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tempFilesTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    autostartModel = new QStandardItemModel(this);
    autostartModel->setColumnCount(3);
    autostartModel->setHorizontalHeaderLabels({tr("Name"), tr("Executable"), tr("Comment")});

    ui->autostartTable->setModel(autostartModel);
    ui->autostartTable->setColumnWidth(0, 256);
    ui->autostartTable->setColumnWidth(1, 255);
    ui->autostartTable->setColumnWidth(2, 255);
    ui->autostartTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    prevCpuStats = Resmon::get_cpu_usage();

    resourceTimer = new QTimer(this);
    connect(resourceTimer, &QTimer::timeout, this, [this]() {
        updateCpuUsage();
        updateSwapUsage();
        updateRamUsage();
        updateDiskUsage();
    });
    //частота обновления в миллисекундах это вот эта циферка
    resourceTimer->start(1000);
    createCpuLoadChart();
    chartUpdateTimer = new QTimer(this);
    connect(chartUpdateTimer, &QTimer::timeout, this, [this]() {
        Resmon::CPUStats current = Resmon::get_cpu_usage();
        double usage = current.usage_percent(prevCpuStats);
        prevCpuStats = current;
        updateChart(usage);
    });
    chartUpdateTimer->start(150);

    populateServicesTable();
    populateAutostartTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateServicesTable()
{
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
}

void MainWindow::on_scanTempFilesButton_clicked()
{
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
            auto folder_contents = get_recursive_folder_content(path);

            if (!folder_contents) {
                showError(QString::fromStdString(folder_contents.error()));
                continue;
            }

            for (const auto& file : *folder_contents) {
                tempFilesModel->appendRow(new QStandardItem(QString::fromStdString(file.string())));
            }
        }

        showInfo(tr("Found %1 temporary files").arg(tempFilesModel->rowCount()));
    }
    catch (const std::exception& e) {
        showError(tr("Error: ") + e.what());
    }
}



void MainWindow::on_startServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("Please select a service first"));
        return;
    }

    QString serviceName = servicesModel->item(selected.first().row(), 0)->text();
    if (serviceName.isEmpty()) {
        showError(tr("Invalid service selected"));
        return;
    }

    int result = start_service(serviceName.toStdString());
    if (result == 0) {
        showInfo(tr("Service started successfully"));
        populateServicesTable();
    } else {
        showError(tr("Failed to start service"));
    }
}

void MainWindow::on_stopServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("Please select a service first"));
        return;
    }

    QString serviceName = servicesModel->item(selected.first().row(), 0)->text();
    if (serviceName.isEmpty()) {
        showError(tr("Invalid service selected"));
        return;
    }

    int result = stop_service(serviceName.toStdString());
    if (result == 0) {
        showInfo(tr("Service stopped successfully"));
        populateServicesTable();
    } else {
        showError(tr("Failed to stop service"));
    }
}

void MainWindow::on_enableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("Please select a service first"));
        return;
    }

    QString serviceName = servicesModel->item(selected.first().row(), 0)->text();
    if (serviceName.isEmpty()) {
        showError(tr("Invalid service selected"));
        return;
    }

    int result = enable_service(serviceName.toStdString());
    if (result == 0) {
        showInfo(tr("Service enabled successfully"));
        populateServicesTable();
    } else {
        showError(tr("Failed to enable service"));
    }
}

void MainWindow::on_disableServiceButton_clicked()
{
    QModelIndexList selected = ui->servicesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("Please select a service first"));
        return;
    }

    QString serviceName = servicesModel->item(selected.first().row(), 0)->text();
    if (serviceName.isEmpty()) {
        showError(tr("Invalid service selected"));
        return;
    }

    int result = disable_service(serviceName.toStdString());
    if (result == 0) {
        showInfo(("Service disabled successfully"));
        populateServicesTable();
    } else {
        showError(tr("Failed to disable service"));
    }
}

void MainWindow::on_refreshServicesButton_clicked()
{
    populateServicesTable();
    showInfo(tr("Service list refreshed"));
}

void MainWindow::on_selectAllFilesButton_clicked(){
    ui->tempFilesTable->selectAll();
    QModelIndexList selectedIndexes = ui->tempFilesTable->selectionModel()->selectedIndexes();
}

void MainWindow::on_clearSelectedFilesButton_clicked(){
    ui->tempFilesTable->clearSelection();
    QModelIndexList selectedIndexes = ui->tempFilesTable->selectionModel()->selectedIndexes();
}

void MainWindow::on_deleteSelectedFilesButton_clicked()
{
    QModelIndexList selected = ui->tempFilesTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("Please select files to delete"));
        return;
    }

    int deletedCount = 0;
    int failedCount = 0;

    for (const QModelIndex &index : selected) {
        QString filePath = tempFilesModel->item(index.row())->text();
        try {
            if (std::filesystem::remove(filePath.toStdString())) {
                deletedCount++;
            } else {
                failedCount++;
            }
        } catch (const std::exception&) {
            failedCount++;
        }
    }


    for (int i = tempFilesModel->rowCount() - 1; i >= 0; --i) {
        if (ui->tempFilesTable->selectionModel()->isRowSelected(i, QModelIndex())) {
            tempFilesModel->removeRow(i);
        }
    }

    showInfo(tr("Deleted %1 files, failed to delete %2 files").arg(deletedCount).arg(failedCount));
}

void MainWindow::on_searchServicesTextChanged(const QString &text)
{

    servicesProxyModel->setFilterKeyColumn(-1);
    servicesProxyModel->setFilterFixedString(text);
}

void MainWindow::createCpuLoadChart() {
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

void MainWindow::updateChart(double usage) {
    static int x = 0;
    const int xlim = 500;
    series->append(x++, usage);

    if (series->count() > xlim) {
        series->remove(0);
    }

    axisX->setRange(x-xlim, x);
}

void MainWindow::updateCpuUsage() {
    Resmon::CPUStats current = Resmon::get_cpu_usage();
    double usage = current.usage_percent(prevCpuStats);
    prevCpuStats = current;

    ui->cpuUsageBar->setValue(static_cast<int>(usage));
    ui->cpuUsageLabel->setText(tr("%1%").arg(usage, 0, 'f', 1));
}


void MainWindow::updateRamUsage() {
    Resmon::MemStats mem = Resmon::get_mem_usage();
    double usage = mem.usage_percent();

    ui->ramUsageBar->setValue(static_cast<int>(usage));
    ui->ramUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
        .arg(usage, 0, 'f', 1)
        .arg((mem.total - mem.available) / (1024 * 1024))
        .arg(mem.total / (1024 * 1024)));
}

void MainWindow::updateSwapUsage() {
    Resmon::MemStats swap = Resmon::get_mem_usage();
    double usage = swap.swap_usage_percent();

    ui->swapUsageBar->setValue(static_cast<int>(usage));
    ui->swapUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                    .arg(usage, 0, 'f', 1)
                                    .arg((swap.swaptotal - swap.swapfree) / (1024 * 1024))
                                    .arg(swap.swaptotal / (1024 * 1024)));
}

void MainWindow::updateDiskUsage() {
    Resmon::DiskStats disk = Resmon::get_disk_usage();
    double usage = disk.usage_percent();

    ui->diskUsageBar->setValue(static_cast<int>(usage));
    ui->diskUsageLabel->setText(tr("%1% (%2 MB / %3 MB)")
                                   .arg(usage, 0, 'f', 1)
                                   .arg((disk.used) / (1024 * 1024))
                                   .arg(disk.total / (1024 * 1024)));
}

void MainWindow::populateAutostartTable(){

    autostartModel->removeRows(0, autostartModel->rowCount());
    std::vector<std::string> entries;
    entries = AutostartManager::listAutostartEntries();
    for (auto& entry : entries){
        QList<QStandardItem*> rowItems;
        auto info = AutostartManager::getAutostartEntryInfo(entry);
        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(info["Name"]));
        QStandardItem *execItem = new QStandardItem(QString::fromStdString(info["Exec"]));
        QStandardItem *commentItem = new QStandardItem(QString::fromStdString(info["Comment"]));

        rowItems << nameItem << execItem << commentItem;
        autostartModel->appendRow(rowItems);
    }
}

void MainWindow::on_addEntryButton_clicked(){
    AddAutostartDialog dialog(this);
    dialog.exec();
    QString name = dialog.getName();
    QString exec = dialog.getExec();
    QString comment = dialog.getComment();

    if (name.isEmpty() || exec.isEmpty()) {
        showError(tr("Name and Executable fields are required"));
        return;
    }
    bool success = AutostartManager::addAutostartEntry(
        name.toStdString(),
        exec.toStdString(),
        comment.toStdString());

    if (success){
        showInfo(tr("Entry %1 has been added successfully").arg(name));
        populateAutostartTable();
    }
    else{
        showError(tr("Entry %1 has not been added").arg(name));
    }
}

void MainWindow::on_removeEntryButton_clicked(){
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()){
        showError(tr("Please select an entry first!"));
        return;
    }
    QString name = autostartModel->item(selected.first().row(), 0)->text();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm Removal"),
                                  tr("Are you sure you want to remove '%1'?").arg(name),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes){
        bool success = AutostartManager::removeAutostartEntry(name.toStdString());
        if (success) {
            showInfo(tr("Entry %1 removed successfully").arg(name));
            populateAutostartTable();
        } else {
            showError(tr("Failed to remove autostart entry %1").arg(name));
        }
    }

}

void MainWindow::on_enableEntryButton_clicked(){
    QModelIndexList selected = ui->autostartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()){
        showError(tr("Please select an entry first!"));
        return;
    }
    QString name = autostartModel->item(selected.first().row(), 0)->text();
    auto info = AutostartManager::getAutostartEntryInfo(name.toStdString());
    QString status = QString::fromStdString(info["Status"]);
    bool statusBool = (QString::fromStdString(info["Status"]).toLower() == "true");
    if (statusBool){
        bool success = AutostartManager::setAutostartEntryEnabledStatus(name.toStdString(), false);
        if (success){
            showInfo(tr("Entry %1 has been disabled!").arg(name));
        }
        else{
            showError(tr("Error Occured"));
        }
    }
    else{
        bool success = AutostartManager::setAutostartEntryEnabledStatus(name.toStdString(), true);
        if(success){
            showInfo(tr("Entry %1 has been enabled!").arg(name));
        }
        else{
            showError(tr("Error Occured"));
        }
    }
}

void MainWindow::on_updateEntriesButton_clicked(){
    populateAutostartTable();
    showInfo(tr("Autostart Entries has been reloaded!"));
}

void MainWindow::onLanguageChanged(int index)
{
    QString languageCode = ui->languageComboBox->itemData(index).toString();


    QString localeCode = (languageCode == "en") ? "en_US" : "ru_RU";

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = new QTranslator(this);

    QString translationFile = QString(":translations/qtguiinterface_%1.qm").arg(localeCode);
    std::cerr << "Loading translation file: " << translationFile.toStdString() << std::endl;

    if (m_translator->load(translationFile)) {
        qApp->installTranslator(m_translator);
        std::cerr << "Successfully loaded translation for:" << localeCode.toStdString()<< std::endl;
    } else {
        std::cerr << "Failed to load translation for:" << localeCode.toStdString()<< std::endl;
    }

    ui->retranslateUi(this);


    servicesModel->setHorizontalHeaderLabels({tr("Service"), tr("Description"), tr("Status")});
    autostartModel->setHorizontalHeaderLabels({tr("Name"), tr("Executable"), tr("Comment")});
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    chart->setTitle(tr("CPU Load Graph"));
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::showInfo(const QString &message)
{
    QMessageBox::information(this, tr("Information"), message);
}
