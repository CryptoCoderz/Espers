// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "fractalui.h"
#include "ui_fractalui.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

FractalUI::FractalUI(QWidget *parent) :
    ui(new Ui::FractalUI),
    model(0)
{
    ui->setupUi(this);
}

FractalUI::~FractalUI()
{
    delete ui;
}
