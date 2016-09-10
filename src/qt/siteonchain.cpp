// Copyright (c) 2016 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "siteonchain.h"
#include "ui_siteonchain.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

SiteOnChain::SiteOnChain(QWidget *parent) :
    ui(new Ui::SiteOnChain),
    model(0)
{
    ui->setupUi(this);
}

SiteOnChain::~SiteOnChain()
{
    delete ui;
}
