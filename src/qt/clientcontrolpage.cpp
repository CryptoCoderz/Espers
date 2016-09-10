// Copyright (c) 2016 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "clientcontrolpage.h"
#include "ui_clientcontrolpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

// For reading client version
#include <QDir>
#include <QTextStream>
#include <QProcess>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

ClientControlPage::ClientControlPage(QWidget *parent) :
    ui(new Ui::ClientControlPage),
    model(0)
{
    ui->setupUi(this);
    // CHECK IF STANDALONE
    QFileInfo check_standalone(QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client");
    if(!check_standalone.exists())
    {
        // C.C.S
        ui->chck4_upd8->setEnabled(false);
        ui->dwngrd_opt->setEnabled(false);
        ui->checkBoxupd8->setEnabled(false);
        ui->minCLIE->setEnabled(false);
        ui->CS_submit->setEnabled(false);
        ui->br_input->setEnabled(false);
        ui->BR_submit->setEnabled(false);
        ui->optin_test->setEnabled(false);
        ui->AU_apply->setEnabled(false);
    }
    else if(check_standalone.exists())
    {
        // READ INSTALLED VERSION
        QFile file(":/version");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            // error out if not accesable
            QMessageBox::information(0,"info",file.errorString());
        QTextStream in(&file);
        ui->instVER->setPlainText(in.readAll());

        // READ LIVE (CURRENT) VERSION
        //open a file
        QString curtxt = QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/version.check";
        QFile fileV(curtxt);
        if(!fileV.open(QIODevice::ReadOnly | QIODevice::Text))
            // error out if not accesable
            QMessageBox::information(0,"info",fileV.errorString());
        QTextStream inV(&fileV);
        ui->liveVER->setPlainText(inV.readAll());

        QFileInfo check_call(QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/upd8.skip");
        if (check_call.exists() && check_call.isFile())
        {
            ui->checkBoxupd8->setChecked(false);
        }

        // Non-client Call
        else if(!check_call.exists())
        {
             ui->checkBoxupd8->setChecked(true);
        }
    }

}

ClientControlPage::~ClientControlPage()
{
    delete ui;
}


void ClientControlPage::on_chck4_upd8_clicked()
{
    QString genCheck = QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/version.call";
    QFile genCall(genCheck);
    if(genCall.open(QIODevice::ReadWrite))
    {
    QTextStream saturate(&genCall);
    saturate << "Temp File that flags verCheck.exe" << endl;
    }

    QProcess *process = new QProcess();
    QString fileX = QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/getVer.exe";
    process->start(fileX);
}

void ClientControlPage::on_dwngrd_opt_clicked()
{
    QMessageBox::information(this, "No Available Versions",
                       "v0.8.4.0 cannot be downgraded, only later versions can.",
                       QMessageBox::Ok );
}

void ClientControlPage::on_CS_submit_clicked()
{
    if(ui->minCLIE->isChecked())
    {
    QMessageBox::information(this, "Already Installed",
                       "You are already running the Minimalistic Client",
                       QMessageBox::Ok );
    this->ui->cliePREV->setText("Select a Client to Preview");

  //  if(ui->radioButtonViewLocalData->isChecked())
   //     {
        ui->minCLIE->setChecked(false);
        ui->minCLIE->setCheckable(false);
        ui->minCLIE->update();
        ui->minCLIE->setCheckable(true);
    //    }

    }
    else
    {
    QMessageBox::information(this, "Nothing Selected",
                        "Please make a selection first.",
                        QMessageBox::Ok );
    }
}

void ClientControlPage::on_BR_submit_clicked()
{
    QMessageBox::information(this, "Coming in v0.8.4.1",
                       "Please email your issues to CryptoCoderz@gmail.com",
                       QMessageBox::Ok );
}

void ClientControlPage::on_AU_apply_clicked()
{
    if(ui->checkBoxupd8->isChecked())
    {
        QString rmSKIP = QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/upd8.skip";
        if (QFile::exists(rmSKIP)) {
            QFile::remove(rmSKIP);
        }
    }
    else
    {
    // Skip Updating
    QString upSkip = QDir::homePath() + "/AppData/Roaming/CryptoCoderz/ESP-Client/upd8.skip";
    QFile skipCall(upSkip);
    if(skipCall.open(QIODevice::ReadWrite))
    {
    QTextStream saturate(&skipCall);
    saturate << "Temp File that flags verCheck.exe" << endl;
    }
    }
}

void ClientControlPage::on_minCLIE_clicked()
{
    if(ui->minCLIE->isChecked())
    {
    QPixmap pix(":/images/0840m");
    this->ui->cliePREV->setPixmap(pix);
    }
    else
    {
        //Do Nothing
    }
}
