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

#include "fractal/fractalcontract.h"
// Added for reading fractal contracts
#include "fractal/fractalengine.h"

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

void FractalUI::on_cCON_clicked()
{
    if (ui->contractTypeCombo->currentText() == "NFT")
    {
        create_smartCONTRACT("image.jpg", "nftGENESIS001", 3);
    } else {
        create_smartCONTRACT("this is only a test", "tokenGENESIS001", 0);
        // Inform user
        QMessageBox::information(this, "Fractal Encoder",
                        "The Fractal platform has written the contract data.",
                        QMessageBox::Ok );
    }
}

void FractalUI::on_netTokensBtn_clicked()
{
    read_contractDATA("tokenGENESIS001", 0);

    if(fextTokenDecodeSuccess) {
        // Inform user of success
        QMessageBox::information(this, "DeOb Decode Success",
                        "The DeOb system was successfully decoded the test genesis Token data!",
                        QMessageBox::Ok );
    } else {
        // Inform user of failure
        QMessageBox::warning(this, "DeOb Decode Failure",
                        "The DeOb system was unable to decode the test genesis Token data...",
                        QMessageBox::Ok );
    }
}
