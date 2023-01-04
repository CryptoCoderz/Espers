// Copyright (c) 2016-2023 The Espers developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "contractui.h"
#include "ui_contractui.h"

#include "clientmodel.h"
#include "walletmodel.h"

#include <string>

#include <QPainter>

ContractUI::ContractUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContractUI)
{
    ui->setupUi(this);

    setFixedSize(880, 500);
}

void ContractUI::setModel(WalletModel *model)
{
    if(model) {
        this->model = model;
        setWindowTitle(QString("Fractal Contract Interface"));
    }
}

ContractUI::~ContractUI()
{
    delete ui;
}

void ContractUI::on_aplyBTN_clicked()
{

}

void ContractUI::on_exitBTN_clicked()
{

}

void ContractUI::on_manageBTN_clicked()
{

}

void ContractUI::on_rnderBTN_clicked()
{

}

void ContractUI::on_viewcBTN_clicked()
{

}

void ContractUI::on_clclhstmethodRADIO_clicked()
{

}

void ContractUI::on_cxnmethodRADIO_clicked()
{

}
