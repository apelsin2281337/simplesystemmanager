#include "../include/autostart_controller.hpp"
#include "../include/autostart.hpp"
#include "../include/addautostartdialog.hpp"

AutostartController::AutostartController(
    QTableView *table,
    QStandardItemModel *model,
    QWidget *parentWidget,
    QObject *parent)
    : QObject(parent)
    , m_table(table)
    , m_model(model)
    , m_parentWidget(parentWidget)
{
    if (!table || !model) {
        qWarning() << "TaskManagerController: null pointer in constructor";
    }
}

void AutostartController::populateAutostartTable()
{
    logL("AutostartController: Populating autostart table");
    m_model->removeRows(0, m_model->rowCount());
    std::vector<std::string> entries = AutostartManager::listAutostartEntries();

    for (auto& entry : entries) {
        QList<QStandardItem*> rowItems;
        auto info = AutostartManager::getAutostartEntryInfo(entry);
        QStandardItem *filenameItem = new QStandardItem(QString::fromStdString(info["Filename"]));
        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(info["Name"]));
        QStandardItem *execItem = new QStandardItem(QString::fromStdString(info["Exec"]));
        QStandardItem *statusItem = new QStandardItem(QString::fromStdString(info["Status"]));
        QStandardItem *commentItem = new QStandardItem(QString::fromStdString(info["Comment"]));

        rowItems << filenameItem << nameItem << execItem << statusItem << commentItem;
        m_model->appendRow(rowItems);
    }
    logL(std::format("AutostartController: Added {} autostart entries to table", entries.size()));
}


void AutostartController::on_addEntryButton_clicked()
{
    logL("AutostartController: Showing add autostart entry dialog");
    AddAutostartDialog dialog(m_parentWidget);
    dialog.exec();
    QString filename = dialog.getFilename();
    QString name = dialog.getName();
    QString exec = dialog.getExec();
    QString comment = dialog.getComment();
    if (filename.isEmpty()){
        filename = name;
    }

    if (name.isEmpty() || exec.isEmpty()) {
        logE("AutostartController: Empty name or executable when adding autostart entry");
        emit error(tr("Name and Executable fields are required"));
        return;
    }

    logL(std::format("AutostartController: Adding autostart entry: {}", name.toStdString()));
    bool success = AutostartManager::addAutostartEntry(
        filename.toStdString(),
        name.toStdString(),
        exec.toStdString(),
        comment.toStdString());

    if (success) {
        logL(std::format("AutostartController: Successfully added autostart entry: {}", name.toStdString()));
        emit info(tr("Entry %1 has been added successfully").arg(name));
        populateAutostartTable();
    } else {
        logE(std::format("AutostartController: Failed to add autostart entry: {}", name.toStdString()));
        emit error(tr("Entry %1 has not been added").arg(name));
    }
}

void AutostartController::on_removeEntryButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("AutostartController: No autostart entry selected for removal");
        emit error(tr("Please select an entry first!"));
        return;
    }

    QString filename = m_model->item(selected.first().row(), 0)->text();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(m_parentWidget, tr("Confirm Removal"),
                                  tr("Are you sure you want to remove '%1'?").arg(filename),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        logL(std::format("AutostartController: Removing autostart entry: {}", filename.toStdString()));
        bool success = AutostartManager::removeAutostartEntry(filename.toStdString());
        if (success) {
            logL(std::format("AutostartController: Successfully removed autostart entry: {}", filename.toStdString()));
            emit info(tr("Entry %1 removed successfully").arg(filename));
        } else {
            logE(std::format("AutostartController: Failed to remove autostart entry: {}", filename.toStdString()));
            emit error(tr("Failed to remove autostart entry %1").arg(filename));
        }
    } else {
        logL("AutostartController:Autostart entry removal cancelled by user");
    }
}

void AutostartController::on_enableEntryButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("AutostartController: No autostart entry selected for enable/disable");
        emit error(tr("Please select an entry first!"));
        return;
    }

    QString name = m_model->item(selected.first().row(), 0)->text();
    auto entryInfo = AutostartManager::getAutostartEntryInfo(name.toStdString());
    bool statusBool = (QString::fromStdString(entryInfo["Status"]).toLower() == "true");

    logL(std::format("AutostartController: Setting autostart entry {} to {}", name.toStdString(), !statusBool));
    bool success = AutostartManager::setAutostartEntryEnabledStatus(name.toStdString(), !statusBool);
    if (success) {
        logL(std::format("AutostartController: Successfully changed status for autostart entry: {}", name.toStdString()));
        emit info(tr("Entry %1 has been %2!").arg(name).arg(statusBool ? tr("disabled") : tr("enabled")));
        populateAutostartTable();
    } else {
        logE(std::format("AutostartController: Failed to change status for autostart entry: {}", name.toStdString()));
        emit error(tr("Error occurred"));
    }
}

void AutostartController::on_updateEntriesButton_clicked()
{
    logL("AutostartController: Refreshing autostart entries");
    populateAutostartTable();
    emit info(tr("Autostart Entries has been reloaded!"));
}
