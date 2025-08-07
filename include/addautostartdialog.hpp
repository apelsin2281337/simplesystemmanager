#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>

class AddAutostartDialog : public QDialog{
    Q_OBJECT
private:
    QLineEdit *nameEdit;
    QLineEdit *execEdit;
    QLineEdit *commentEdit;
public:
    AddAutostartDialog(QWidget *parent = nullptr);

    QString getName() const { return nameEdit->text(); }
    QString getExec() const { return execEdit->text(); }
    QString getComment() const { return commentEdit->text(); }
};
