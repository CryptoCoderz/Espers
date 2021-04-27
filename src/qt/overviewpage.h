#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include "util/util.h"

#include <QTimer>
#include <QWidget>

#include "core/wallet.h"

double getPoSHardness(int);
double convertPoSCoins(int64_t);
int getPoSTime(int);
int PoSInPastHours(int);
const CBlockIndex* getPoSIndex(int);

class ClientModel;
class WalletModel;
class TxViewDelegate;
class TransactionFilterProxy;
//
class OverviewPage;

namespace Ui {
    class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE
//
extern OverviewPage* ovrvwref;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void showOutOfSyncWarning(bool fShow);
    void ShowSynchronizedMessage(bool fSyncFinish);

public slots:
    void setBalance(const CAmount& balance, const CAmount& stake, const CAmount& unconfirmedBalance, const CAmount& immatureBalance);

    void updatePoSstat(bool);
    void setCntBlocks(int pseudo);
    void setCntConnections(int count);
    void setLockStatus();
    void setLockIconLocked();
    void setLockIconUnlocked();

signals:
    void transactionClicked(const QModelIndex &index);

private:
    QTimer *timer;
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CAmount currentBalance;
    CAmount currentStake;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentAnonymizedBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchOnlyStake;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;
    int nDisplayUnit;

    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void updateAlerts(const QString &warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
    void on_viewQR_clicked();
    void on_vwalltx_clicked();
};

#endif // OVERVIEWPAGE_H
