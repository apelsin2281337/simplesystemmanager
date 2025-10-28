#include "../include/mainincludes.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , prevCpuStats(Resmon::get_cpu_usage())
    , m_translator(new QTranslator(this))
    , lastNetworkStats(Resmon::get_internet_usage())

{
    logL("MainWindow: Initializing MainWindow");
    ui->setupUi(this);
    cfg = std::make_unique<Config>("config.json");
    cfg->load();

    ui->retranslateUi(this);
    setupComboBoxes();
    setupModels();
    setupTables();
    setupTimers();
    setupControllers();
    //не менять сломается инет
    resourceTimer->start(1000);
    chartUpdateTimer->start(700);

    processUpdateTimer = std::make_unique<QTimer>(this);
    //connect(processUpdateTimer.get(), &QTimer::timeout, this, &MainWindow::populateTaskManager);
    //processUpdateTimer->start(2000);

    ui->cpuGroupBox->setTitle(tr("CPU Usage (%1)").arg(QString::fromStdString(Resmon::get_cpu_name())));
    ui->networkGroupBox->setTitle(tr("Network Usage (%1)").arg(QString::fromStdString(Resmon::get_network_interface())));
    logL("MainWindow: MainWindow initialization completed");
}

MainWindow::~MainWindow() {
    logL("MainWindow: Cleanup started");
    resourceTimer->stop();
    chartUpdateTimer->stop();
    logL("MainWindow: Cleanup completed\n");
}

void MainWindow::setupComboBoxes(){
    ui->languageComboBox->addItem("Русский", "ru_RU");
    ui->languageComboBox->addItem("English", "en_US");
    ui->themeComboBox->addItem(tr("Dark Theme"), "dark");
    ui->themeComboBox->addItem(tr("Light Theme"), "light");
    connect(ui->themeComboBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::onThemeChanged);
    ui->themeComboBox->setCurrentIndex(0);

    connect(ui->languageComboBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::onLanguageChanged);
    ui->languageComboBox->setCurrentIndex(0);
}

void MainWindow::setupControllers(){
    SvcCtl = std::make_unique<ServicesController>(ui->servicesTable, servicesModel.get(), servicesProxyModel.get());
    connect(ui->startServiceButton,  &QPushButton::clicked, SvcCtl.get(),      &ServicesController::on_startServiceButton_clicked);\
    connect(ui->stopServiceButton,   &QPushButton::clicked, SvcCtl.get(),      &ServicesController::on_stopServiceButton_clicked);
    connect(ui->enableServiceButton, &QPushButton::clicked, SvcCtl.get(),      &ServicesController::on_enableServiceButton_clicked);
    connect(ui->disableServiceButton,&QPushButton::clicked, SvcCtl.get(),      &ServicesController::on_disableServiceButton_clicked);
    connect(ui->refreshServicesButton,&QPushButton::clicked, SvcCtl.get(),     &ServicesController::on_refreshServicesButton_clicked);
    connect(ui->searchServicesLineEdit, &QLineEdit::textChanged, SvcCtl.get(), &ServicesController::on_searchServicesTextChanged);
    connect(SvcCtl.get(), &ServicesController::info,  this, &MainWindow::showInfo);
    connect(SvcCtl.get(), &ServicesController::error, this, &MainWindow::showError);
    SvcCtl->populateServicesTable();

    TmCtl = std::make_unique<TaskManagerController>(ui->taskManagerTable, taskManagerModel.get(), taskManagerProxyModel.get(), this, this);
    connect(ui->startProcessButton, &QPushButton::clicked, TmCtl.get(), &TaskManagerController::on_startProcessButton_clicked);
    connect(ui->stopProcessButton, &QPushButton::clicked, TmCtl.get(), &TaskManagerController::on_stopProcessButton_clicked);
    connect(ui->updateTasksButton, &QPushButton::clicked, TmCtl.get(), &TaskManagerController::on_updateTasksButton_clicked);
    connect(ui->searchTaskLineEdit, &QLineEdit::textChanged, TmCtl.get(), &TaskManagerController::on_searchTaskLineEdit_textChanged);
    TmCtl->populateTaskManager();
    connect(TmCtl.get(), &TaskManagerController::info,  this, &MainWindow::showInfo);
    connect(TmCtl.get(), &TaskManagerController::error, this, &MainWindow::showError);

    AutostartCtl = std::make_unique<AutostartController>(ui->autostartTable, autostartModel.get(), this, this);
    connect(ui->addEntryButton, &QPushButton::clicked, AutostartCtl.get(), &AutostartController::on_addEntryButton_clicked);
    connect(ui->removeEntryButton, &QPushButton::clicked, AutostartCtl.get(), &AutostartController::on_removeEntryButton_clicked);
    connect(ui->enableEntryButton, &QPushButton::clicked, AutostartCtl.get(), &AutostartController::on_enableEntryButton_clicked);
    connect(ui->updateEntriesButton, &QPushButton::clicked, AutostartCtl.get(), &AutostartController::on_updateEntriesButton_clicked);
    AutostartCtl->populateAutostartTable();
    connect(AutostartCtl.get(), &AutostartController::info,  this, &MainWindow::showInfo);
    connect(AutostartCtl.get(), &AutostartController::error, this, &MainWindow::showError);

    TempfilesCtl = std::make_unique<TempfilesController>(ui->tempFilesTable, tempFilesModel.get(), this, this);
    connect(ui->scanTempFilesButton, &QPushButton::clicked, TempfilesCtl.get(), &TempfilesController::on_scanTempFilesButton_clicked);
    connect(ui->clearSelectedFilesButton, &QPushButton::clicked, TempfilesCtl.get(), &TempfilesController::on_clearSelectedFilesButton_clicked);
    connect(ui->selectAllFilesButton, &QPushButton::clicked, TempfilesCtl.get(), &TempfilesController::on_selectAllFilesButton_clicked);
    connect(ui->deleteSelectedFilesButton, &QPushButton::clicked, TempfilesCtl.get(), &TempfilesController::on_deleteSelectedFilesButton_clicked);
    connect(TempfilesCtl.get(), &TempfilesController::info,  this, &MainWindow::showInfo);
    connect(TempfilesCtl.get(), &TempfilesController::error, this, &MainWindow::showError);
}


void MainWindow::setupModels()
{
    servicesModel = std::make_unique<QStandardItemModel>(this);
    servicesModel->setColumnCount(3);
    servicesModel->setHorizontalHeaderLabels({
        tr("Service"), tr("Description"), tr("Status")
    });

    servicesProxyModel = std::make_unique<QSortFilterProxyModel>(this);
    setupProxyModel(servicesProxyModel.get(), servicesModel.get());
    ui->servicesTable->setModel(servicesProxyModel.get());

    tempFilesModel = std::make_unique<QStandardItemModel>(this);
    tempFilesModel->setColumnCount(1);
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    ui->tempFilesTable->setModel(tempFilesModel.get());

    autostartModel = std::make_unique<QStandardItemModel>(this);
    autostartModel->setColumnCount(5);
    autostartModel->setHorizontalHeaderLabels({
        tr("Filename"), tr("Name"), tr("Executable"),
        tr("Status"),   tr("Comment")
    });
    ui->autostartTable->setModel(autostartModel.get());

    taskManagerModel = std::make_unique<QStandardItemModel>(0, 6, this);
    taskManagerModel->setHorizontalHeaderLabels({
        tr("PID"), tr("Name"), tr("CPU Load"),
        tr("Memory Load"), tr("Physical RAM"), tr("Virtual RAM")
    });

    taskManagerProxyModel = std::make_unique<QSortFilterProxyModel>(this);
    setupProxyModel(taskManagerProxyModel.get(), taskManagerModel.get());
    ui->taskManagerTable->setModel(taskManagerProxyModel.get());
    ui->taskManagerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}


void MainWindow::setupProxyModel(QSortFilterProxyModel *proxy,
                                 QStandardItemModel   *source)
{
    proxy->setSourceModel(source);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->setDynamicSortFilter(true);
}

void MainWindow::setupTables(){


    ui->servicesTable->setModel(servicesProxyModel.get());
    //ui->servicesTable->setColumnWidth(0, 300);
    //ui->servicesTable->setColumnWidth(1, 338);
    //ui->servicesTable->setColumnWidth(2, 68);

    ui->servicesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->servicesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->servicesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->servicesTable->setColumnWidth(2, 100);
    ui->servicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);


    ui->tempFilesTable->setModel(tempFilesModel.get());
    ui->tempFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tempFilesTable->setSelectionMode(QAbstractItemView::ExtendedSelection);


    ui->autostartTable->setModel(autostartModel.get());
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->autostartTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->autostartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    prevCpuStats = Resmon::get_cpu_usage();

    ui->taskManagerTable->setModel(taskManagerProxyModel.get());
}

void MainWindow::setupTimers(){
    resourceTimer = std::make_unique<QTimer>(this);
    connect(resourceTimer.get(), &QTimer::timeout, this, [this]() {
        updateCpuUsage();
        updateSwapUsage();
        updateRamUsage();
        updateDiskUsage();
        updateInternetUsage();
        TmCtl->populateTaskManager();
        SvcCtl->populateServicesTable();
    });

    createCpuLoadChart();
    loadTheme(QString::fromStdString(cfg->getTheme()));
    loadLanguage(QString::fromStdString(cfg->getLanguage()));
    chartUpdateTimer = std::make_unique<QTimer>(this);
    connect(chartUpdateTimer.get(), &QTimer::timeout, this, [this]() {
        Resmon::CPUStats current = Resmon::get_cpu_usage();
        double usage = current.usage_percent(prevCpuStats);
        prevCpuStats = current;
        updateChart(usage);
    });
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
        cfg->setTheme(themeName.toStdString());
        cfg->save();
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

void MainWindow::loadLanguage(const QString &localeCode)
{
    int index = ui->languageComboBox->findData(localeCode);
    if (index != -1) {
        const bool signalsBlocked = ui->languageComboBox->blockSignals(true);
        ui->languageComboBox->setCurrentIndex(index);
        ui->languageComboBox->blockSignals(signalsBlocked);
    } else {
        ui->languageComboBox->setCurrentIndex(0);
    }

    logL(std::format("MainWindow: Changing language to: {}", localeCode.toStdString()));

    qApp->removeTranslator(m_translator.get());
    m_translator = std::make_unique<QTranslator>(this);

    QString translationFile = QString(":translations/qtguiinterface_%1.qm").arg(localeCode);
    logL(std::format("MainWindow: Loading translation file: {}", translationFile.toStdString()));

    if (m_translator->load(translationFile)) {
        qApp->installTranslator(m_translator.get());
        logL("MainWindow: Translation loaded successfully");
    } else {
        logE("MainWindow: Failed to load translation");
    }

    ui->retranslateUi(this);

    servicesModel->setHorizontalHeaderLabels({tr("Service"), tr("Description"), tr("Status")});
    autostartModel->setHorizontalHeaderLabels({tr("Filename"), tr("Name"), tr("Executable"), tr("Status"), tr("Comment")});
    tempFilesModel->setHorizontalHeaderLabels({tr("File Path")});
    taskManagerModel->setHorizontalHeaderLabels({tr("PID"), tr("Name"), tr("CPU Load"), tr("Memory Load"), tr("Physical RAM"), tr("Virtual RAM")});


    chart->setTitle(tr("CPU Load Graph"));
}

void MainWindow::onLanguageChanged(int index)
{
    QString localeCode = ui->languageComboBox->itemData(index).toString();
    loadLanguage(localeCode);

    cfg->setLanguage(localeCode.toStdString());
    qDebug() << localeCode;
    cfg->save();
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
