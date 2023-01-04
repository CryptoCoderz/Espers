// Copyright (c) 2020-2023 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "settingspage.h"
#include "ui_settingspage.h"
#include "core/main.h"
#include "primitives/base58.h"
#include "clientmodel.h"
#include "walletmodel.h"

#include "signverifymessagedialog.h"
#include "blockbrowser.h"
#include "optionsdialog.h"
#include "rpcconsolesettings.h"
#include "bitcoingui.h"
#include "guiutil.h"
#include "askpassphrasedialog.h"
#include "editconfigdialog.h"
#include "aboutdialog.h"

#include "overviewpage.h"

#include <QDesktopServices>
#include <QFileDialog>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

    setFixedSize(1200, 700);

}

void SettingsPage::setModel(WalletModel *model)
{
    this->model = model;

    if(settingsStatus)
    {
        ui->lockwallet->setText("Unlock Wallet");
    }
    else
    {
        ui->lockwallet->setText("Lock Wallet");
    }
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::on_signmesage_clicked()
{
    SignVerifyMessageDialog sgnvrfyDialog(this);
    sgnvrfyDialog.setModel(guiref->getWalletModel());
    sgnvrfyDialog.exec();
}

void SettingsPage::on_blockexplorer_clicked()
{
    BlockBrowser blkbrowser(this);
    blkbrowser.setModel(model);
    blkbrowser.exec();
}

void SettingsPage::on_options_clicked()
{
    OptionsDialog dlg;
    dlg.setModel(guiref->getClientModel()->getOptionsModel());
    dlg.exec();
}

void SettingsPage::on_debugwindow_clicked()
{
    RPCConsolesettings rpccon;
    rpccon.setClientModel(guiref->getClientModel());
    rpccon.exec();
}

void SettingsPage::on_backupwallet_clicked()
{
    QString saveDir;
#if QT_VERSION < 0x050000
        saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
        saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!model->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void SettingsPage::on_show_autobackups_clicked()
{
    GUIUtil::showBackups();
}

void SettingsPage::on_encryptwallet_clicked()
{
    if(!guiref->getWalletModel())
        return;

    if(settingsLock)
    {
        QMessageBox::warning(this, tr("Already Encrypted"), tr("This wallet is already encrypted!"));
        return;
    }

    AskPassphraseDialog dlg(AskPassphraseDialog::Encrypt, this);
    dlg.setModel(guiref->getWalletModel());
    dlg.exec();

    BitcoinGUI::setEncryptionStatus(guiref->getWalletModel()->getEncryptionStatus());
}

void SettingsPage::on_open_datafolder_clicked()
{
    boost::filesystem::path path = GetDataDir();
    QString pathString = QString::fromStdString(path.string());
    QDesktopServices::openUrl(QUrl::fromLocalFile(pathString));
}

void SettingsPage::on_changepassphrase_clicked()
{
    if(!settingsChangePass)
    {
        QMessageBox::warning(this, tr("Wallet Locked or not Encrypted"), tr("Wallet is either locked or not encrypted in the first place."));
        return;
    }

    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(guiref->getWalletModel());
    dlg.exec();
}

void SettingsPage::on_editconf_clicked()
{
    EditConfigDialog dlg;
    dlg.setModel(guiref->getClientModel());
    dlg.exec();
}

void SettingsPage::on_lockwallet_clicked()
{
    if(!guiref->getWalletModel())
        return;

    if(settingsUncrypted)
    {
        QMessageBox::warning(this, tr("Not Encrypted"), tr("You must first encrypt your wallet in order to lock/unlock it!"));
        return;
    }

    if(settingsStatus)
    {
        // Unlock wallet when requested by wallet model
        QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Unlock for Staking Only"),
                             tr("Would you like to unlock for staking only?"),
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No);

        if(retval == QMessageBox::Yes)
        {
            AskPassphraseDialog::Mode mode = AskPassphraseDialog::UnlockStaking;
            AskPassphraseDialog dlg(mode, this);
            dlg.setModel(guiref->getWalletModel());
            dlg.exec();
        }
        if(retval == QMessageBox::No)
        {
            AskPassphraseDialog::Mode mode = AskPassphraseDialog::Unlock;
            AskPassphraseDialog dlg(mode, this);
            dlg.setModel(guiref->getWalletModel());
            dlg.exec();
        }
        guiref->getWalletModel()->setWalletLocked(false);
        ui->lockwallet->setText("Lock Wallet");

    }
    else
    {
        guiref->getWalletModel()->setWalletLocked(true);
        ui->lockwallet->setText("Unlock Wallet");
    }
}

void SettingsPage::on_aboutcoin_clicked()
{
    AboutDialog dlg;
    dlg.setModel(guiref->getClientModel());
    dlg.exec();
}

void SettingsPage::on_aboutqt_clicked()
{
    BitcoinGUI::aboutQtExt_Static();
}
