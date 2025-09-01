#pragma once
#include <iostream>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "../include/logger.hpp"


class StartNewProcess : public QDialog{
    Q_OBJECT

private:
    QLineEdit* command;
    QCheckBox* executeAsRoot;

public:
    StartNewProcess(QWidget* parent = nullptr);

    QString getCommand() const {
        return command->text();
    }

    bool isExecuteAsRoot() {
        return executeAsRoot->isChecked();
    }

};
