#pragma once
#include <QObject>
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include "task_manager.hpp"

class TaskManagerController : public QObject
{
    Q_OBJECT

public:
    explicit TaskManagerController(
        QTableView *table,
        QStandardItemModel *model,
        QSortFilterProxyModel *proxy,
        QWidget *parentWidget,
        QObject *parent = nullptr);
    void populateTaskManager();

public slots:
    void on_startProcessButton_clicked();
    void on_stopProcessButton_clicked();
    void on_updateTasksButton_clicked();
    void on_searchTaskLineEdit_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    QTableView *m_table;
    QStandardItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
    QWidget *m_parentWidget;

signals:
    void info(const QString &msg);
    void error(const QString &msg);
};
