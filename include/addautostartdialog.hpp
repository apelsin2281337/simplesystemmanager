#pragma once
#include <QDialog>

class QDialog;
class QLineEdit;
class QString;

class AddAutostartDialog : public QDialog{
    Q_OBJECT
private:
    QLineEdit* nameEdit;
    QLineEdit* execEdit;
    QLineEdit* commentEdit;
public:
    AddAutostartDialog(QWidget *parent = nullptr);

    QString getName() const;
    QString getExec() const;
    QString getComment() const;
};
