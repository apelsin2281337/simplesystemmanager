#include "../include/tempfiles_controller.hpp"
#include "../include/temp_files.hpp"
#include "../include/logger.hpp"

TempfilesController::TempfilesController(
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

void TempfilesController::on_scanTempFilesButton_clicked()
{
    logL("TempfilesController: Scanning for temporary files");
    m_model->removeRows(0, m_model->rowCount());

    try {
        std::filesystem::path home = get_home_directory();
        std::vector<std::filesystem::path> tempfiles_paths = {
            "/tmp",
            "/var/tmp",
            home / ".local/share/Trash"
        };

        for (auto& path : tempfiles_paths) {
            logL(std::format("TempfilesController: Scanning directory: {}", path.string()));
            auto folder_contents = get_recursive_folder_content(path);

            if (!folder_contents) {
                logE(folder_contents.error());
                emit error(QString::fromStdString(folder_contents.error()));
                continue;
            }

            for (const auto& file : *folder_contents) {
                m_model->appendRow(new QStandardItem(QString::fromStdString(file.string())));
            }
        }

        emit info(tr("Found %1 temporary files").arg(m_model->rowCount()));
        logL(std::format("TempfilesController: Found {} temporary files", m_model->rowCount()));
    }
    catch (const std::exception& e) {
        logE(std::format("TempfilesController: Error scanning temp files: {}", e.what()));
        emit error(tr("Error: ") + e.what());
    }
}



void TempfilesController::on_selectAllFilesButton_clicked(){
    logL("TempfilesController: Selecting all temporary files");
    m_table->selectAll();
}

void TempfilesController::on_clearSelectedFilesButton_clicked(){
    logL("TempfilesController: Clearing temporary files selection");
    m_table->clearSelection();
}

void TempfilesController::on_deleteSelectedFilesButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("TempfilesController: No files selected for deletion");
        emit error(tr("Please select files to delete"));
        return;
    }

    logL(std::format("TempfilesController: Attempting to delete {} files", selected.size()));
    int deletedCount = 0;
    int failedCount = 0;
    QMessageBox::StandardButton reply = QMessageBox::question(
        m_parentWidget,
        tr("Confirm Deletion"),
        tr("Are you sure you want to delete %1 selected file(s)?").arg(selected.size()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        return;
    }

    for (const QModelIndex& index : selected) {
        QString filePath = m_model->item(index.row())->text();
        try {
            if (std::filesystem::remove(filePath.toStdString())) {
                deletedCount++;
                //logL(std::format("TempfilesController: Deleted file: {}", filePath.toStdString()));
            } else {
                failedCount++;
                logE(std::format("TempfilesController: Failed to delete file: {}", filePath.toStdString()));
            }
        } catch (const std::exception& e) {
            failedCount++;
            logE(std::format("TempfilesController: Exception when deleting {}: {}", filePath.toStdString(), e.what()));
        }
    }

    for (int i = m_model->rowCount() - 1; i >= 0; --i) {
        if (m_table->selectionModel()->isRowSelected(i, QModelIndex())) {
            m_model->removeRow(i);
        }
    }

    emit info(tr("Deleted %1 files, failed to delete %2 files").arg(deletedCount).arg(failedCount));
    logL(std::format("TempfilesController: Deletion completed: {} success, {} failures", deletedCount, failedCount));
}
