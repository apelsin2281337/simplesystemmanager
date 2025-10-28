#include "../include/services_controller.hpp"
#include "../include/logger.hpp"
#include "../include/services.hpp"

ServicesController::ServicesController(
    QTableView *table,
    QStandardItemModel *model,
    QSortFilterProxyModel *proxy,
    QObject *parent)
    : QObject(parent)
    , m_table(table)
    , m_model(model)
    , m_proxy(proxy)
{
    if (!table || !model || !proxy) {
        qWarning() << "ServicesController: null pointer in constructor";
    }
}

void ServicesController::on_startServiceButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("ServicesController: No service selected for start");
        emit error(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = m_proxy->mapToSource(selected.first());
    QString serviceName = m_model->item(sourceIndex.row(), 0)->text();
    QString serviceStatus = m_model->item(sourceIndex.row(), 2)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("ServicesController: Invalid service selected for start");
        emit error(tr("Invalid service selected"));
        return;
    }

    if (serviceStatus.compare("active", Qt::CaseInsensitive) != 0) {
        logL(std::format("ServicesController: Starting service: {}", serviceName.toStdString()));
        result = start_service(serviceName.toStdString());
    }
    else {
        emit info(tr("ServicesController: Service %1 is already started").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("ServicesController: Successfully started service: {}", serviceName.toStdString()));
        emit info(tr("Service started successfully"));
        populateServicesTable();
    } else {
        logE(std::format("ServicesController: Failed to start service: {}", serviceName.toStdString()));
        emit error(tr("Failed to start service"));
    }
}

void ServicesController::on_stopServiceButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("ServicesController: No service selected for stop");
        emit error(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = m_proxy->mapToSource(selected.first());
    QString serviceName = m_model->item(sourceIndex.row(), 0)->text();
    QString serviceStatus = m_model->item(sourceIndex.row(), 2)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("ServicesController: Invalid service selected for stop");
        emit error(tr("Invalid service selected"));
        return;
    }

    if (serviceStatus.compare("active", Qt::CaseInsensitive) == 0) {
        logL(std::format("ServicesController: Stopping service: {}", serviceName.toStdString()));
        result = stop_service(serviceName.toStdString());
    }
    else {
        emit info(tr("ServicesController: Service %1 is already stopped").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("ServicesController: Successfully stopped service: {}", serviceName.toStdString()));
        emit info(tr("Service stopped successfully"));
        populateServicesTable();
    } else {
        logE(std::format("ServicesController: Failed to stop service: {}", serviceName.toStdString()));
        emit error(tr("Failed to stop service"));
    }
}

void ServicesController::on_enableServiceButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("ServicesController: No service selected for enable");
        emit error(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = m_proxy->mapToSource(selected.first());
    QString serviceName = m_model->item(sourceIndex.row(), 0)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("ServicesController: Invalid service selected for enable");
        emit error(tr("Invalid service selected"));
        return;
    }
    if (!is_service_enabled(serviceName.toStdString())) {
        logL(std::format("ServicesController: Enabling service: {}", serviceName.toStdString()));
        result = enable_service(serviceName.toStdString());
    }
    else {
        emit info(tr("ServicesController: Service %1 is already enabled").arg(serviceName));
        return;
    }

    if (result == 0) {
        logL(std::format("ServicesController: Successfully enabled service: {}", serviceName.toStdString()));
        emit info(tr("Service enabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("ServicesController: Failed to enable service: {}", serviceName.toStdString()));
        emit error(tr("Failed to enable service"));
    }
}

void ServicesController::on_disableServiceButton_clicked()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        logE("ServicesController: No service selected for disable");
        emit error(tr("Please select a service first"));
        return;
    }

    QModelIndex sourceIndex = m_proxy->mapToSource(selected.first());
    QString serviceName = m_model->item(sourceIndex.row(), 0)->text();
    int result = -1;
    if (serviceName.isEmpty()) {
        logE("ServicesController: Invalid service selected for disable");
        emit error(tr("Invalid service selected"));
        return;
    }
    if (is_service_enabled(serviceName.toStdString())) {
        logL(std::format("ServicesController: Disabling service: {}", serviceName.toStdString()));
        result = disable_service(serviceName.toStdString());
    }
    else {
        emit info(tr("ServicesController: Service %1 is already disabled").arg(serviceName));
        return;
    }
    if (result == 0) {
        logL(std::format("ServicesController: Successfully disabled service: {}", serviceName.toStdString()));
        emit info(tr("Service disabled successfully"));
        populateServicesTable();
    } else {
        logE(std::format("ServicesController: Failed to disable service: {}", serviceName.toStdString()));
        emit error(tr("Failed to disable service"));
    }
}

void ServicesController::on_refreshServicesButton_clicked()
{
    logL("ServicesController: Refreshing services table");
    populateServicesTable();
    emit info(tr("Service list refreshed"));
}


void ServicesController::populateServicesTable()
{
    logL("ServicesController: Populating services table");
    QModelIndexList selectedIndexes = m_table->selectionModel()->selectedRows();
    m_model->removeRows(0, m_model->rowCount());
    auto services = get_services();
    QFont font;
    QSet<QString> selectedServices;
    for (const QModelIndex& index : selectedIndexes) {
        QModelIndex sourceIndex = m_proxy->mapToSource(index);
        if (sourceIndex.isValid()) {
            selectedServices.insert(m_model->item(sourceIndex.row(), 0)->text());
        }
    }

    for (const auto& service : services) {
        QList<QStandardItem*> rowItems;

        QStandardItem *nameItem = new QStandardItem(QString::fromStdString(service.name));
        QStandardItem *descItem = new QStandardItem(QString::fromStdString(service.description));
        QStandardItem *statusItem = new QStandardItem(QString::fromStdString(service.status));

        if (service.status == "active"){
            statusItem->setForeground(QBrush(Qt::green));
        } else if (service.status == "failed"){
            statusItem->setForeground(QBrush(Qt::red));
        }

        rowItems << nameItem << descItem << statusItem;
        m_model->appendRow(rowItems);

        for (auto& item : rowItems){
            item->setFont(font);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }

    if (!selectedServices.isEmpty()) {
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QString service = m_model->item(row, 0)->text();
            if (selectedServices.contains(service)) {
                QModelIndex proxyIndex = m_proxy->mapFromSource(m_model->index(row, 0));
                m_table->selectionModel()->select(proxyIndex,
                                                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }
    //logL(std::format("ServicesController: Added {} services to table", services.size()));
}

void ServicesController::on_searchServicesTextChanged(const QString &text)
{
    logL(std::format("ServicesController: Filtering services with text: {}", text.toStdString()));
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setFilterFixedString(text);
}

