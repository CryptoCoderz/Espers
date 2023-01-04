// Copyright (c) 2016-2023 The Espers developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "nftui.h"
#include "ui_nftui.h"

#include "clientmodel.h"
#include "walletmodel.h"

#include <string>

#include <QPainter>

NftUI::NftUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NftUI)
{
    ui->setupUi(this);

    setFixedSize(880, 500);
}

void NftUI::setModel(WalletModel *model)
{
    if(model) {
        this->model = model;
        setWindowTitle(QString("Launch an NFT"));
    }
}

NftUI::~NftUI()
{
    delete ui;
}

void NftUI::on_btnPATH_clicked()
{

}

void NftUI::on_genprevBTN_clicked()
{

}

void NftUI::on_cancelBTN_clicked()
{

}

void NftUI::on_createBTN_clicked()
{

}
