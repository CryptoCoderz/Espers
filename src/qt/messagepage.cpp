// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "messagepage.h"
#include "ui_messagepage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

MessagePage::MessagePage(QWidget *parent) :
    ui(new Ui::MessagePage),
    model(0)
{
    ui->setupUi(this);
}

MessagePage::~MessagePage()
{
    delete ui;
}
