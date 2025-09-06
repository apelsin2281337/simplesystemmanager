#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;

class StartNewProcess : public QDialog{
    Q_OBJECT

private:
    QLineEdit* command;
    QCheckBox* executeAsRoot;

public:
    StartNewProcess(QWidget* parent = nullptr);

    QString getCommand() const;
    bool isExecuteAsRoot();
};
