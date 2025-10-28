#pragma once
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QMessageBox>
#include <QFileDialog>
#include <QFont>
#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include <QIcon>
#include <QRandomGenerator>
#include <QProcess>
#include <thread>
#include <QDebug>


#include "../include/services.hpp"
#include "../include/temp_files.hpp"
#include "../include/autostart.hpp"
#include "../include/addautostartdialog.hpp"
#include "../include/mainwindow.h"
#include "ui_mainwindow.h"
#include "../include/logger.hpp"
#include "../include/config_manager.hpp"
#include "../include/start_new_process_dialog.hpp"
#include "../include/task_manager.hpp"


#include "services_controller.hpp"
#include "taskmanager_controller.hpp"
#include "autostart_controller.hpp"
#include "tempfiles_controller.hpp"
