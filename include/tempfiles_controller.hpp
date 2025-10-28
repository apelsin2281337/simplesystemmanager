#pragma once
#include <QObject>
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

class TempfilesController : public QObject
{
    Q_OBJECT

public:
    explicit TempfilesController(
        QTableView *table,
        QStandardItemModel *model,
        QWidget *parentWidget,
        QObject *parent = nullptr);
    void populateServicesTable();

public slots:
    void on_scanTempFilesButton_clicked();
    void on_deleteSelectedFilesButton_clicked();
    void on_selectAllFilesButton_clicked();
    void on_clearSelectedFilesButton_clicked();


private:
    Ui::MainWindow *ui;
    QTableView *m_table;
    QStandardItemModel *m_model;
    QWidget *m_parentWidget;
    QObject *m_parent;

signals:
    void info(const QString &msg);
    void error(const QString &msg);
};
