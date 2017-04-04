// Copyright (c) 2016-2017 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "generatepage.h"
#include "ui_generatepage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include "transactionrecord.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "init.h"
#include "rpcserver.h"
#include "kernel.h"

using namespace json_spirit;

#include <sstream>
#include <string>

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
            // Generation Page (Will be enabled in an supplementary 0860+ update)
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

            QTimer::singleShot(20000,this,SLOT(blockCalled()));
}

GeneratePage::~GeneratePage()
{
    delete ui;
}

std::string getPoSHash(int Height)
{
    if(Height < 0) { return "351c6703813172725c6d660aa539ee6a3d7a9fe784c87fae7f36582e3b797058"; }
    int desiredheight;
    desiredheight = Height;
    if (desiredheight < 0 || desiredheight > nBestHeight)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > desiredheight)
        pblockindex = pblockindex->pprev;
    return pblockindex->phashBlock->GetHex();
}


double getPoSHardness(int height)
{
    const CBlockIndex* blockindex = getPoSIndex(height);

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;

}

const CBlockIndex* getPoSIndex(int height)
{
    std::string hex = getPoSHash(height);
    uint256 hash(hex);
    return mapBlockIndex[hash];
}

int getPoSTime(int Height)
{
    std::string strHash = getPoSHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nTime;
}

int PoSInPastHours(int hours)
{
    int wayback = hours * 3600;
    bool check = true;
    int height = pindexBest->nHeight;
    int heightHour = pindexBest->nHeight;
    int utime = (int)time(NULL);
    int target = utime - wayback;

    while(check)
    {
        if(getPoSTime(heightHour) < target)
        {
            check = false;
            return height - heightHour;
        } else {
            heightHour = heightHour - 1;
        }
    }

    return 0;
}

double convertPoSCoins(int64_t amount)
{
    return (double)amount / (double)COIN;
}

void GeneratePage::updatePoSstat(bool stat)
{
    if(stat)
    {
        uint64_t nWeight = 0;
        if (pwalletMain)
            nWeight = pwalletMain->GetStakeWeight();
        uint64_t nNetworkWeight = GetPoSKernelPS();
        bool staking = nLastCoinStakeSearchInterval && nWeight;
        uint64_t nExpectedTime = staking ? (Params().TargetSpacing() * nNetworkWeight / nWeight) : 0;
        QString Qseconds = " Second(s)";
        if(nExpectedTime > 86399)
        {
           nExpectedTime = nExpectedTime / 60 / 60 / 24;
           Qseconds = " Day(s)";
        }
        else if(nExpectedTime > 3599)
        {
           nExpectedTime = nExpectedTime / 60 / 60;
           Qseconds = " Hour(s)";
        }
        else if(nExpectedTime > 59)
        {
           nExpectedTime = nExpectedTime / 60;
           Qseconds = " Minute(s)";
        }
        ui->lbTime->show();
        ui->diffdsp->show();
        ui->hashrt->show();
        int height = pindexBest->nHeight;
        uint64_t Pawrate = GetPoSKernelPS();
        double Pawrate2 = ((double)Pawrate / 100000000);
        QString QPawrate = QString::number(Pawrate2, 'f', 2);
        ui->hashrt->setText(QPawrate + " ESP");
        if(Pawrate2 > 999999999)
        {
           Pawrate2 = (Pawrate2 / 1000000000);
           QPawrate = QString::number(Pawrate2, 'f', 2);
           ui->hashrt->setText(QPawrate + " BILLION ESP");
        }
        else if(Pawrate2 > 999999)
        {
           Pawrate2 = (Pawrate2 / 1000000);
           QPawrate = QString::number(Pawrate2, 'f', 2);
           ui->hashrt->setText(QPawrate + " MILLION ESP");
        }
        else if(Pawrate2 > 999)
        {
           Pawrate2 = (Pawrate2 / 1000);
           QPawrate = QString::number(Pawrate2, 'f', 2);
           ui->hashrt->setText(QPawrate + " THOUSAND ESP");
        }
        double hardness = getPoSHardness(height);
        uint64_t nStakePercentage = (double)nWeight / (double)nNetworkWeight * 100;
        uint64_t nNetPercentage = (100 - (double)nStakePercentage);
        if(nWeight > nNetworkWeight)
        {
            nStakePercentage = (double)nNetworkWeight / (double)nWeight * 100;
            nNetPercentage = (100 - (double)nStakePercentage);
        }
        CBlockIndex* pindex = pindexBest;
        QString QHardness = QString::number(hardness, 'f', 6);      
        QString QStakePercentage = QString::number(nStakePercentage, 'f', 2);
        QString QNetPercentage = QString::number(nNetPercentage, 'f', 2);
        QString QTime = clientModel->getLastBlockDate().toString();
        QString QExpect = QString::number(nExpectedTime, 'f', 0);
        QString QStaking = "DISABLED";
        QString QStakeEN = "NOT STAKING";
        ui->estnxt->setText(QExpect + Qseconds);
        ui->stkstat->setText(QStakeEN);
        if(!pindex->IsProofOfStake())
        {
            QHardness = "Block is PoW";
            QPawrate = "Block is PoW";
            ui->hashrt->setText(QPawrate);
        }
        if(nExpectedTime == 0)
        {
            QExpect = "NOT STAKING";
            ui->estnxt->setText(QExpect);
        }
        if(staking)
        {
            QStakeEN = "STAKING";
            ui->stkstat->setText(QStakeEN);
        }
        if(GetBoolArg("-staking", true))
        {
            QStaking = "ENABLED";
        }
        ui->lbTime->setText(QTime);
        ui->diffdsp->setText(QHardness);
        ui->stken->setText(QStaking);
        ui->urwheight->setValue(QStakePercentage.toDouble());
        ui->netweight->setValue(QNetPercentage.toDouble());
        if(nStakePercentage < 1)
        {
            ui->urwheight->setValue(1);
            ui->netweight->setValue(99);
        }
        else if(nStakePercentage > 99 && pindex->nHeight > Params().StartPoSBlock())
        {
            ui->urwheight->setValue(99);
            ui->netweight->setValue(1);
        }
// TODO: DISPLAY STAKING STATISTICS
        ui->hourlydsp->setText("0");
        ui->dailydsp->setText("0");
        ui->weeklydsp->setText("0");
        ui->monthlydsp->setText("0");

        QTimer::singleShot(10000,this,SLOT(blockCalled()));
    }
}

void GeneratePage::blockCalled()
{
    updatePoSstat(true);
}
