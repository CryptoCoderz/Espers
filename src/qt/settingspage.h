#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include "clientmodel.h"
#include "walletmodel.h"
#include "core/main.h"
#include "core/wallet.h"
#include "primitives/base58.h"
#include <QWidget>

namespace Ui {
class SettingsPage;
}
class WalletModel;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = 0);
    ~SettingsPage();
    
    void setModel(WalletModel *model);
    
public slots:
    //
private slots:
	//
    void on_signmesage_clicked();

    void on_blockexplorer_clicked();

    void on_options_clicked();

    void on_debugwindow_clicked();

    void on_backupwallet_clicked();

    void on_show_autobackups_clicked();

    void on_encryptwallet_clicked();

    void on_open_datafolder_clicked();

    void on_changepassphrase_clicked();

    void on_editconf_clicked();

    void on_lockwallet_clicked();

    void on_aboutcoin_clicked();

    void on_aboutqt_clicked();

private:
    Ui::SettingsPage *ui;
    WalletModel *model;
    
};

#endif // SETTINGSPAGE_H
