#include "tokenui.h"
#include "ui_tokenui.h"

#include "bitcoinunits.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "optionsmodel.h"

#include <QPixmap>
#include <QUrl>

TokenUI::TokenUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenUI),
    model(0)
{
    ui->setupUi(this);

    setWindowTitle(QString("Fractal Token Creation"));

    ui->chkReqPayment->setVisible(true);
    ui->lblAmount->setVisible(true);
    ui->lnReqAmount->setVisible(true);

    ui->lnLabel->setText("label");

    ui->btnSaveAs->setEnabled(false);
}

TokenUI::~TokenUI()
{
    delete ui;
}

void TokenUI::setModel(OptionsModel *model)
{
    this->model = model;
}

void TokenUI::on_lnReqAmount_textChanged()
{
    //
}

void TokenUI::on_btnSaveAs_clicked()
{
    //
}
