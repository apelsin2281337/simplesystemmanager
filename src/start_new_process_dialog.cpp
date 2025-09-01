#include "../include/start_new_process_dialog.hpp"

StartNewProcess::StartNewProcess(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Start New Process!");

    command = new QLineEdit(this);
    executeAsRoot = new QCheckBox(this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Command: "), command);
    formLayout->addRow(tr("Execute as Root:"), executeAsRoot);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setFixedSize(400, 200);
    logL("Start new process Window Invoked!");
}
