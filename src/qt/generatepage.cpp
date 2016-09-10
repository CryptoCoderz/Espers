// Copyright (c) 2016 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "generatepage.h"
#include "ui_generatepage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

GeneratePage::GeneratePage(QWidget *parent) :
    ui(new Ui::GeneratePage),
    model(0)
{
    ui->setupUi(this);
    /*
        // CHECK IF STANDALONE
        QFileInfo check_standalone(QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client");
        if(!check_standalone.exists())
        { */
            // Generation Page (Will be enabled in supplementary 0805 update)
            ui->CPUcbox->setEnabled(false);
            ui->pushButton_7->setEnabled(false);
            ui->GPUamnt->setEnabled(false);
            ui->GPUmdl->setEnabled(false);
            ui->checkCPUcnt->setEnabled(false);
            ui->c_hashrate_3->setEnabled(false);
            ui->c_hashrate_2->setEnabled(false);
            ui->c_hashrate_5->setEnabled(false);
            ui->c_hashrate_4->setEnabled(false);
            ui->pushButton_11->setEnabled(false);
            ui->pushButton_7->setEnabled(false);
            ui->pushButton_8->setEnabled(false);
            ui->mnlGPU->setEnabled(false);
            ui->c_hashrate_12->setEnabled(false);
          //  ui->soloGPU->setEnabled(false);
          //  ui->c_hashrate_6->setEnabled(false);
          //  ui->c_hashrate_7->setEnabled(false);
          //  ui->c_hashrate_8->setEnabled(false);
          //  ui->c_hashrate_9->setEnabled(false);
          //  ui->pushButton_12->setEnabled(false);
          //  ui->pushButton_9->setEnabled(false);
          //  ui->pushButton_10->setEnabled(false);
      /*  }

        else
        {
        //Combo Box List (CPU)
        for(int ccount = 1; ccount < 25; ccount++)
        {
            ccount--;
            if(ccount == 0)
            {
                ui->CPUcbox->addItem("All Cores");
            }
            ccount++;
            ui->CPUcbox->addItem( QString::number(ccount)); // + " Core"
        }
        //Combo Box Amount (GPU)
        for(int gamnt = 1; gamnt < 9; gamnt++)
        {
            ui->GPUamnt->addItem( QString::number(gamnt)); // + " GPUs"
        }

        //Combo Box Model (GPU)
        ui->GPUmdl->addItem("Select");
        ui->GPUmdl->addItem("NVIDIA");
        ui->GPUmdl->addItem("AMD R9 390(x)");
        ui->GPUmdl->addItem("AMD R9 380(x)");
        ui->GPUmdl->addItem("AMD R9 370(x)");
        ui->GPUmdl->addItem("AMD R9 290(x)");
        ui->GPUmdl->addItem("AMD R9 280(x)");
        ui->GPUmdl->addItem("AMD R9 270(x)");
        ui->GPUmdl->addItem("AMD HD8990");
        ui->GPUmdl->addItem("AMD HD8970");
        ui->GPUmdl->addItem("AMD HD8870");
        ui->GPUmdl->addItem("AMD HD7990");
        ui->GPUmdl->addItem("AMD HD7970");
        ui->GPUmdl->addItem("AMD HD7870");
        ui->GPUmdl->addItem("AMD HD6990");
        ui->GPUmdl->addItem("AMD HD6970");
        ui->GPUmdl->addItem("AMD HD6870");
        ui->GPUmdl->addItem("AMD HD5970");
        ui->GPUmdl->addItem("AMD HD5870");
        } */
}

GeneratePage::~GeneratePage()
{
    delete ui;
}
