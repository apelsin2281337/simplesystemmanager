#include <addautostartdialog.hpp>

AddAutostartDialog::AddAutostartDialog(QWidget *parent) : QDialog(parent){
    setWindowTitle("Add Autostart Entry!");
    nameEdit = new QLineEdit(this);
    execEdit = new QLineEdit(this);
    commentEdit = new QLineEdit(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Name: ", nameEdit);
    formLayout->addRow("Executable:", execEdit);
    formLayout->addRow("Comment:", commentEdit);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setFixedSize(400, 200);
}
