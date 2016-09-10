#include "generationpage.h"
#include "ui_generationpage.h"

#include "optionsmodel.h"
#include "bitcoingui.h"
#include "csvmodelwriter.h"
#include "guiutil.h"
// For mining function
#include "init.h"

#ifdef USE_QRCODE
#include "qrcodedialog.h"
#endif

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
// for hashmeter
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QProcess>
#include <QString>

#include <QStandardPaths>


GenerationPage::GenerationPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenerationPage),
    clientModel(0),
    walletModel(0)
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
        ui->soloGPU->setEnabled(false);
        ui->c_hashrate_6->setEnabled(false);
        ui->c_hashrate_7->setEnabled(false);
        ui->c_hashrate_8->setEnabled(false);
        ui->c_hashrate_9->setEnabled(false);
        ui->pushButton_12->setEnabled(false);
        ui->pushButton_9->setEnabled(false);
        ui->pushButton_10->setEnabled(false);
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

GenerationPage::~GenerationPage()
{
    delete ui;
}

void GenerationPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
}

void GenerationPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

// Mining Button
int sMine = 0; // Change text per click
// Actual function
void GenerationPage::on_pushButton_7_clicked()
{
    if (sMine < 1)
    {
        this->ui->pushButton_7->setText("Stop Mining");
        sMine ++;
        GenerateBitcoins(GetBoolArg("-gen", true), pwalletMain);
        // PreCreate Hashmeter files
        // Hash/s
        boost::filesystem::path meterPath;
        meterPath = GetDefaultDataDir() / "Hashes.sec";
        FILE* meterFile = fopen(meterPath.string().c_str(), "w");
        fprintf(meterFile, "0 Hash/s\n");
        fclose(meterFile);
        // kHash/s
        boost::filesystem::path meter2Path;
        meter2Path = GetDefaultDataDir() / "kHashes.sec";
        FILE* meter2File = fopen(meter2Path.string().c_str(), "w");
        fprintf(meter2File, "0 Hash/s\n");
        fclose(meter2File);
        // mHash/s
        boost::filesystem::path meter3Path;
        meter3Path = GetDefaultDataDir() / "mHashes.sec";
        FILE* meter3File = fopen(meter3Path.string().c_str(), "w");
        fprintf(meter3File, "0 Hash/s\n");
        fclose(meter3File);

    }

    else if (sMine >= 1)
    {
        this->ui->pushButton_7->setText("Start Mining");
        sMine --;
        GenerateBitcoins(GetBoolArg("-gen", false), pwalletMain);
        // Set Hashmeter to Zero
        // Hash/s check
        if (dHashesPerSec < 1000)
        {
            boost::filesystem::path ConfPath;
            ConfPath = GetDefaultDataDir() / "Hashes.sec";
            FILE* ConfFile = fopen(ConfPath.string().c_str(), "w");
            fprintf(ConfFile, "0 Hash/s\n");
            fclose(ConfFile);
        }
        // kHash/s check
        else if (dHashesPerSec > 999)
        {
            boost::filesystem::path ConfPath;
            ConfPath = GetDefaultDataDir() / "kHashes.sec";
            FILE* ConfFile = fopen(ConfPath.string().c_str(), "w");
            fprintf(ConfFile, "0 Hash/s\n");
            fclose(ConfFile);
        }
        // mHash/s check
        else if (dHashesPerSec > 999999)
        {
            boost::filesystem::path ConfPath;
            ConfPath = GetDefaultDataDir() / "mHashes.sec";
            FILE* ConfFile = fopen(ConfPath.string().c_str(), "w");
            fprintf(ConfFile, "0 Hash/s\n");
            fclose(ConfFile);
        }
    }

}

void GenerationPage::on_pushButton_8_clicked()
{
    // Hashmeter-------
    // Hash/s check
    if (dHashesPerSec < 1000)
    {
        //
#ifdef Q_OS_LINUX
        QString curtxt = QDir::homePath() + "/.Espers/Hashes.sec";
#elif defined(Q_OS_MAC)
        QString curtxt = QDir::homePath() + QDir::toNativeSeparators("/Library/Application Support/Espers/Hashes.sec");
#else // Windows
        QString curtxt = QDir::homePath() + "/AppData/Roaming/Espers/Hashes.sec";
#endif
        QFile fileV(curtxt);
        if(!fileV.open(QIODevice::ReadOnly | QIODevice::Text))

            // error out if not accesable
            QMessageBox::information(0,"info",fileV.errorString());

        QTextStream inV(&fileV);
        this->ui->c_hashrate->setPlainText(inV.readAll());
    }
    // kHash/s check
    else if (dHashesPerSec > 999)
    {
        //
#ifdef Q_OS_LINUX
        QString curtxt = QDir::homePath() + "/.Espers/kHashes.sec";
#elif defined(Q_OS_MAC)
        QString curtxt = QDir::homePath() + QDir::toNativeSeparators("/Library/Application Support/Espers/kHashes.sec");
#else //Windows
        QString curtxt = QDir::homePath() + "/AppData/Roaming/Espers/kHashes.sec";
#endif
        QFile fileV(curtxt);
        if(!fileV.open(QIODevice::ReadOnly | QIODevice::Text))

            // error out if not accesable
            QMessageBox::information(0,"info",fileV.errorString());

        QTextStream inV(&fileV);
        this->ui->c_hashrate->setPlainText(inV.readAll());
    }
    // mHash/s check
    else if (dHashesPerSec > 999999)
    {
        //
#ifdef Q_OS_LINUX
        QString curtxt = QDir::homePath() + "/.Espers/mHashes.sec";
#elif defined(Q_OS_MAC)
        QString curtxt = QDir::homePath() +  QDir::toNativeSeparators("/Library/Application Support/Espers/mHashes.sec");
#else //Windows
        QString curtxt = QDir::homePath() + "/AppData/Roaming/Espers/mHashes.sec";
#endif
        QFile fileV(curtxt);
        if(!fileV.open(QIODevice::ReadOnly | QIODevice::Text))

            // error out if not accesable
            QMessageBox::information(0,"info",fileV.errorString());

        QTextStream inV(&fileV);
        this->ui->c_hashrate->setPlainText(inV.readAll());
    }

}
