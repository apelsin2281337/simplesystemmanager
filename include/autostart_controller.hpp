#pragma once
#include <QObject>
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include "task_manager.hpp"

class AutostartController : public QObject
{
    Q_OBJECT

public:
    explicit AutostartController(
        QTableView *table,
        QStandardItemModel *model,
        QWidget *parentWidget,
        QObject *parent = nullptr);
    void populateAutostartTable();

public slots:
    void on_addEntryButton_clicked();
    void on_enableEntryButton_clicked();
    void on_removeEntryButton_clicked();
    void on_updateEntriesButton_clicked();

private:
    Ui::MainWindow *ui;
    QTableView *m_table;
    QStandardItemModel *m_model;
    QWidget *m_parentWidget;

signals:
    void info(const QString &msg);
    void error(const QString &msg);
};
