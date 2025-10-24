#include "../include/addautostartdialog.hpp"
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>


#include "../include/logger.hpp"

AddAutostartDialog::AddAutostartDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Add Autostart Entry!"));
    QLabel* filenameLabel = new QLabel(this);
    filenameEdit = new QLineEdit(this);
    nameEdit = new QLineEdit(this);
    execEdit = new QLineEdit(this);
    commentEdit = new QLineEdit(this);

    filenameLabel->setText("Will use a name of an entry as a filename if left empty.");
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(filenameLabel);
    formLayout->addRow("Filename (without .desktop) ", filenameEdit);
    formLayout->addRow(tr("Name: "), nameEdit);
    formLayout->addRow(tr("Executable: "), execEdit);
    formLayout->addRow(tr("Comment: "), commentEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setFixedSize(400, 200);
    logL("Add Dialog Window Invoked!");
}


QString AddAutostartDialog::getFilename() const { return filenameEdit->text(); }
QString AddAutostartDialog::getName() const { return nameEdit->text(); }
QString AddAutostartDialog::getExec() const { return execEdit->text(); }
QString AddAutostartDialog::getComment() const { return commentEdit->text(); }
