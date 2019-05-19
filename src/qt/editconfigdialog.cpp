#include "editconfigdialog.h"
#include "ui_editconfigdialog.h"
#include "clientmodel.h"

#include "consensus/version.h"

EditConfigDialog::EditConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditConfigDialog),
    model(0)
{
    ui->setupUi(this);
}

void EditConfigDialog::setModel(ClientModel *model)
{
    if(model)
    {
        this->model = model;
        ui->configText->setText(this->model->getConfigFileContent());
    }
}

EditConfigDialog::~EditConfigDialog()
{
    delete ui;
}

void EditConfigDialog::accept()
{
    if (this->model)
    {
        this->model->setConfigFileContent(ui->configText->toPlainText());
    }
    QDialog::accept();
}
