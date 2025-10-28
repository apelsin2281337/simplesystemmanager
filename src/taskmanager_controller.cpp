#include "../include/taskmanager_controller.hpp"
#include "../include/logger.hpp"
#include <thread>
#include "../include/start_new_process_dialog.hpp"
#include <QProcess>

TaskManagerController::TaskManagerController(
    QTableView *table,
    QStandardItemModel *model,
    QSortFilterProxyModel *proxy,
    QWidget *parentWidget,
    QObject *parent)
    : QObject(parent)
    , m_table(table)
    , m_model(model)
    , m_proxy(proxy)
    , m_parentWidget(parentWidget)
{
    if (!table || !model || !proxy) {
        qWarning() << "TaskManagerController: null pointer in constructor";
    }
}

void TaskManagerController::on_searchTaskLineEdit_textChanged(const QString &text)
{
    logL(std::format("TaskManagerController: Filtering tasks with text: {}", text.toStdString()));
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setFilterFixedString(text);
}

void TaskManagerController::populateTaskManager()
{
    logL("TaskManagerController: Populating Task Manager");

    int sortColumn = m_table->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder sortOrder = m_table->horizontalHeader()->sortIndicatorOrder();

    QModelIndexList selectedIndexes = m_table->selectionModel()->selectedRows();
    QSet<QString> selectedPids;
    for (const QModelIndex& index : selectedIndexes) {
        QModelIndex sourceIndex = m_proxy->mapToSource(index);
        if (sourceIndex.isValid()) {
            selectedPids.insert(m_model->item(sourceIndex.row(), 0)->text());
        }
    }

    m_model->removeRows(0, m_model->rowCount());

    auto tasks = TaskManager::getProcessesInfo();

    for (const auto& task : tasks) {
        QList<QStandardItem*> rowItems;
        size_t nproc = std::thread::hardware_concurrency();
        float cpuLoad = task.cpuLoad / nproc;
        QStandardItem *pidItem = new QStandardItem(QString::number(task.pid));
        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(task.command));
        QStandardItem *cpuloadItem = new QStandardItem(QString::number(cpuLoad, 'f', 1));
        QStandardItem *memloadItem = new QStandardItem(QString::number(task.memLoad, 'f', 1));
        QStandardItem *PRAMItem = new QStandardItem(QString::number(task.rss));
        QStandardItem *VRAMItem = new QStandardItem(QString::number(task.vsz));

        pidItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        cpuloadItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        memloadItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        PRAMItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        VRAMItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        for (auto& item : {pidItem, nameItem, cpuloadItem, memloadItem, PRAMItem, VRAMItem}) {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        rowItems << pidItem << nameItem << cpuloadItem << memloadItem << PRAMItem << VRAMItem;
        m_model->appendRow(rowItems);
    }

    if (sortColumn >= 0) {
        m_table->sortByColumn(sortColumn, sortOrder);
    }

    if (!selectedPids.isEmpty()) {
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QString pid = m_model->item(row, 0)->text();
            if (selectedPids.contains(pid)) {
                QModelIndex proxyIndex = m_proxy->mapFromSource(m_model->index(row, 0));
                m_table->selectionModel()->select(proxyIndex,
                                                               QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }

    logL(std::format("TaskManagerController: Updated {} tasks in table", tasks.size()));
}


void TaskManagerController::on_startProcessButton_clicked() {
    StartNewProcess procDialog(m_parentWidget);
    procDialog.exec();
    QString command = procDialog.getCommand();
    bool isRoot = procDialog.isExecuteAsRoot();

    if (command.isEmpty()) {
        return;
    }

    QProcess* process = new QProcess(this);
    QStringList arguments;

    if (isRoot) {
        arguments << "-E" << command;
        process->start("sudo", arguments);
    } else {
        arguments << "-E" << "-u" << "#1000" << "sh" << "-c" << command;
        process->start("sudo", arguments);
        qDebug() << "My QStringList:" << arguments;
    }


    if (process->waitForStarted()) {
        info("Process started successfully!");
    } else {
        error("Error. Failed to start process!");
        process->deleteLater();
    }

}

void TaskManagerController::on_stopProcessButton_clicked() {
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("TaskManagerController: No process was selected to stop");
        error(tr("Please select a process first!"));
        return;
    }
    QModelIndex sourceIndex = m_proxy->mapToSource(selected.first());

    QString pidStr = m_model->item(sourceIndex.row(), 0)->text();
    logF(pidStr.toStdString());
    bool ok;
    int pid = pidStr.toInt(&ok);

    if (!ok || pid <= 0) {
        logE(("TaskManagerController: Invalid PID format: " + pidStr).toStdString());
        error(tr("Invalid process ID"));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        m_parentWidget,
        tr("Confirm Process Termination"),
        tr("Are you sure you want to terminate process %1?").arg(pid),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        return;
    }

    QProcess process;
    QStringList args;
    args << "-15" << QString::number(pid);

    process.start("kill", args);

    if (!process.waitForFinished(2000)) {
        logL("TaskManagerController: Process didn't respond to SIGTERM, trying SIGKILL");

        QProcess killProcess;
        QStringList killArgs;
        killArgs << "-9" << QString::number(pid);
        killProcess.start("kill", killArgs);
        killProcess.waitForFinished(1000);
    }

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        logL(("TaskManagerController: Successfully terminated process: " + pidStr).toStdString());
        info(tr("Process terminated successfully"));
    } else {
        logE(("TaskManagerController: Failed to terminate process: " + pidStr).toStdString());
        error(tr("Failed to terminate process"));
    }

    populateTaskManager();
}

void TaskManagerController::on_updateTasksButton_clicked(){
    populateTaskManager();
    info("Task Manager was updated!");
}


