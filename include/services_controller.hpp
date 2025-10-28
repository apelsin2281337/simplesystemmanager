#pragma once
#include <QObject>
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

class ServicesController : public QObject
{
    Q_OBJECT

public:
    explicit ServicesController(
        QTableView *table,
        QStandardItemModel *model,
        QSortFilterProxyModel *proxy,
        QObject *parent = nullptr);
    void populateServicesTable();

public slots:
    void on_startServiceButton_clicked();
    void on_stopServiceButton_clicked();
    void on_enableServiceButton_clicked();
    void on_disableServiceButton_clicked();
    void on_refreshServicesButton_clicked();
    void on_searchServicesTextChanged(const QString &text);


private:
    Ui::MainWindow *ui;
    QTableView *m_table;
    QStandardItemModel *m_model;
    QSortFilterProxyModel *m_proxy;

signals:
    void info(const QString &msg);
    void error(const QString &msg);
};
