#ifndef BITCOINGUI_H
#define BITCOINGUI_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include <stdint.h>

class TransactionTableModel;
class ClientModel;
class WalletModel;
class TransactionView;
class OverviewPage;
class ClientControlPage;
class MessagePage;
class BlockBrowser;
class SettingsPage;
class FractalUI;
class AddressBookPage;
class SendCoinsDialog;
class SignVerifyMessageDialog;
class Notificator;
class RPCConsole;
class BitcoinGUI;

QT_BEGIN_NAMESPACE
class QLabel;
class QModelIndex;
//class QProgressBar;
class QStackedWidget;
QT_END_NAMESPACE

extern BitcoinGUI* guiref;
extern bool settingsLock;
extern bool settingsChangePass;
extern bool settingsRelock;
extern bool settingsUncrypted;

/**
  Bitcoin GUI main class. This class represents the main window of the Bitcoin UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class BitcoinGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit BitcoinGUI(QWidget *parent = 0);
    ~BitcoinGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel *walletModel);

    ClientModel* getClientModel() {
        return this->clientModel;
    }

    WalletModel* getWalletModel() {
        return this->walletModel;
    }

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    ClientModel *clientModel;
    WalletModel *walletModel;

    QToolBar *toolbar;

    QStackedWidget *centralStackedWidget;

    OverviewPage *overviewPage;
    QWidget *transactionsPage;
    AddressBookPage *addressBookPage;
    AddressBookPage *receiveCoinsPage;
    SendCoinsDialog *sendCoinsPage;
    ClientControlPage *clientcontrolPage;
    MessagePage *messagePage;
    QLabel* netLabel;
    BlockBrowser *blockbrowser;
    FractalUI *fractalUI;
    SettingsPage *settingsPage;
    SignVerifyMessageDialog *signVerifyMessageDialog;

    //QLabel *labelEncryptionIcon;
    //QLabel *labelStakingIcon;
    //QLabel *labelConnectionsIcon;
    //QLabel *labelBlocksIcon;
    //QLabel *progressBarLabel;
    //QProgressBar *progressBar;

    QMenuBar *appMenuBar;
    QAction *overviewAction;
    QAction *clientcontrolAction;
    QAction *messageAction;
    QAction *blockbrowserAction;
    QAction *fractalUIAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *addressBookAction;
    QAction *signMessageAction;
    QAction *verifyMessageAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *exportAction;
    QAction *encryptWalletAction;
    QAction *backupWalletAction;
    QAction *changePassphraseAction;
    QAction *unlockWalletAction;
    QAction *lockWalletAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;
    QAction *showBackupsAction;
    QAction *editConfigAction;
    QAction *editConfigExtAction;
    QAction *openDataDirAction;
    QAction *settingsAction;
    QAction *importPrivateKeyAction;

    QSystemTrayIcon *trayIcon;
    Notificator *notificator;
    TransactionView *transactionView;
    RPCConsole *rpcConsole;

    //QMovie *syncIconMovie;
    /** Keep track of previous number of blocks, to detect progress */
    int prevBlocks;

    uint64_t nWeight;

    /** Create the main UI actions. */
    void createActions();
    /** Create the menu bar and sub-menus. */
    //void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray (notification) icon */
    void createTrayIcon();

    void clearWidgets();

public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    static void setEncryptionStatus(int status);
    void setWalletUnlockStakingOnly();
    void setUnencrypted();
    void setUnlocked();
    void setLocked();
    void aboutQtExt_Internal();
    static void aboutQtExt_Static();
    static void gotoHistoryPage_static();

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] modal     true to use a message box, false to use a notification
       @param[in] style     style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
    */
    void message(const QString &title, const QString &message, bool modal, unsigned int style);
    /** Asks the user whether to pay the transaction fee or to cancel the transaction.
       It is currently not possible to pass a return value to another thread through
       BlockingQueuedConnection, so an indirected pointer is used.
       https://bugreports.qt-project.org/browse/QTBUG-10440

      @param[in] nFeeRequired       the required fee
      @param[out] payFee            true to pay the fee, false to not pay the fee
    */
    void askFee(qint64 nFeeRequired, bool *payFee);
    void handleURI(QString strURI);

private slots:
    /** Switch to overview (home) page */
    void gotoOverviewPage();
    /** Switch to clientcontrol(management) page */
    void gotoClientControlPage();
    /** Switch to message(encrypted-emailing) page */
    void gotoMessagePage();
    /** Switch to chain info(explorer) page */
    void gotoBlockBrowser();
    /** Switch to settings page */
    void gotoSettingsPage();
    /** Switch to fractalui(websites on the blockchain) page */
    void gotoFractalUI();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to address book page */
    void gotoAddressBookPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to send coins page */
    void gotoSendCoinsPage();

    /** Show Sign/Verify Message dialog and switch to sign message tab */
    void gotoSignMessageTab(QString addr = "");
    /** Show Sign/Verify Message dialog and switch to verify message tab */
    void gotoVerifyMessageTab(QString addr = "");

    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();
#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif
    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void incomingTransaction(const QModelIndex & parent, int start);
    /** Encrypt the wallet */
    void encryptWallet();
    /** Backup the wallet */
    void backupWallet();
    /** Import a private key */
    void importPrivateKey();
    /** Change encrypted wallet passphrase */
    void changePassphrase();
    /** Ask for passphrase to unlock wallet temporarily */
    void unlockWallet();
    /** Lock local wallet */
    void lockWallet();
    /** Edit the Espers.conf file (internally) */
    void editConfig();
    /** Edit the Espers.conf file (externally) */
    void editConfigExt();
    /** Open the data directory */
    void openDataDir();
    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);
    /** simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();
    /** Update local wallet weight */
    void updateWeight();
    /** Update local wallet staking icon */
    //void updateStakingIcon();

    /** called by a timer to check if fRequestShutdown has been set **/
    void detectShutdown();
};

#endif // BITCOINGUI_H
