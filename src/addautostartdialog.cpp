#include "../include/addautostartdialog.hpp"

AddAutostartDialog::AddAutostartDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Add Autostart Entry!"));

    // Create widgets directly (Qt will manage memory)
    nameEdit = new QLineEdit(this);
    execEdit = new QLineEdit(this);
    commentEdit = new QLineEdit(this);

    // Create layouts directly
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Name: "), nameEdit);
    formLayout->addRow(tr("Executable:"), execEdit);
    formLayout->addRow(tr("Comment:"), commentEdit);

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
