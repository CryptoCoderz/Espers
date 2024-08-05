#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "bitcoingui.h"

#include "qrcodedialog.h"

#include "util/init.h"
#include "rpc/rpcserver.h"
#include "rpc/rpcclient.h"
#include "consensus/kernel.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QScrollArea>
#include <QScroller>
#include <QSettings>
#include <QTimer>
#include <QMovie>
#include <QPixmap>

#include <QMessageBox>

#define DECORATION_SIZE 64
#define ICON_OFFSET 16
#define NUM_ITEMS 6

int64_t balAmount = 0;

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::ESP)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        mainRect.moveLeft(ICON_OFFSET);
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace - ICON_OFFSET, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(value.canConvert<QColor>())
        {
            foreground = qvariant_cast<QColor>(value);
        }

        balAmount = amount;

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address, &boundingRect);

        //if (index.data(TransactionTableModel::WatchonlyRole).toBool())
        //{
            //QIcon iconWatchonly = qvariant_cast<QIcon>(index.data(TransactionTableModel::WatchonlyDecorationRole));
            //QRect watchonlyRect(boundingRect.right() + 5, mainRect.top()+ypad+halfheight, 16, halfheight);
            //iconWatchonly.paint(painter, watchonlyRect);
        //}

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    currentBalance(-1),
    currentStake(-1),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    currentWatchOnlyBalance(-1),
    currentWatchOnlyStake(-1),
    currentWatchUnconfBalance(-1),
    currentWatchImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    // Recent transactions
    ui->listTransactions_2->setItemDelegate(txdelegate);
    ui->listTransactions_2->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions_2->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions_2->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listTransactions_2->setMinimumWidth(300);

    connect(ui->listTransactions_2, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelTransactionsStatus->setText(tr("Synchronizing... Please wait."));
    QMovie *SYNCmovie = new QMovie(":/gifs/syncgif");
    ui->syncLabel->setMovie(SYNCmovie);
    SYNCmovie->stop();// Initially set stopped
    ui->syncLabel->setVisible(true);

    // Always show these labels (Sync Status)
    ui->labelTransactionsStatus->setVisible(true);
    //ui->syncstatusGIF->setVisible(true);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    //currentWatchOnlyBalance = watchOnlyBalance;
    //currentWatchOnlyStake = watchOnlyStake;
    //currentWatchUnconfBalance = watchUnconfBalance;
    //currentWatchImmatureBalance = watchImmatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance + immatureBalance));
    //ui->labelWatchAvailable->setText(BitcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyBalance));
    //ui->labelWatchStake->setText(BitcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyStake));
    //ui->labelWatchPending->setText(BitcoinUnits::floorWithUnit(nDisplayUnit, watchUnconfBalance));
    //ui->labelWatchImmature->setText(BitcoinUnits::floorWithUnit(nDisplayUnit, watchImmatureBalance));
    //ui->labelWatchTotal->setText(BitcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyBalance + watchOnlyStake + watchUnconfBalance + watchImmatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = true;
    //bool showWatchOnlyImmature = watchImmatureBalance != 0;

    // for symmetry reasons also show immature label when the watch-only one is shown
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
    //ui->labelWatchImmature->setVisible(showWatchOnlyImmature); // show watch-only immature balance

    //static int cachedTxLocks = 0;

    //if(cachedTxLocks != nCompleteTXLocks){
    //    cachedTxLocks = nCompleteTXLocks;
    //    ui->listTransactions_2->update();
    //}
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    //ui->labelSpendable->setVisible(showWatchOnly);      // show spendable label (only when watch-only is active)
    //ui->labelWatchonly->setVisible(showWatchOnly);      // show watch-only label
    //ui->labelESPtotalimg->setVisible(showWatchOnly);    // show watch-only balance separator line
    //ui->labelWatchStake->setVisible(showWatchOnly);    // show watch-only balance separator line
    //ui->labelWatchAvailable->setVisible(showWatchOnly); // show watch-only available balance
    //ui->labelWatchPending->setVisible(showWatchOnly);   // show watch-only pending balance
    //ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly){
        //ui->labelWatchImmature->hide();
    }
    else{
        ui->labelBalance->setIndent(20);
        ui->labelStake->setIndent(20);
        ui->labelUnconfirmed->setIndent(20);
        ui->labelImmature->setIndent(20);
        ui->labelTotal->setIndent(20);
    }
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());

        // Update network status display
        setCntConnections(model->getNumConnections());
        connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(setCntConnections(int)));
        // ''
        setCntBlocks(model->getNumBlocks());
        connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setCntBlocks(int)));
        //
        connect(model, SIGNAL(statusWalletLockChanged(bool)), this, SLOT(setLockStatus()));
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions_2->setModel(filter);
        ui->listTransactions_2->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

        //updateWatchOnlyLabels(model->haveWatchOnly());
        //connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));

        // lockstatusLabel
        // Initialize status icon
        if(settingsStatus)
        {
            setLockIconLocked();
        } else
        {
            setLockIconUnlocked();
        }
    }

    // update the display unit, to not use the default ("ESP")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {

        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if(currentBalance != -1)
            setBalance(currentBalance, walletModel->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = nDisplayUnit;

       ui->listTransactions_2->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::ShowSynchronizedMessage(bool fSyncFinish)
{
    if(fSyncFinish) {
        ui->labelTransactionsStatus->setText(tr("Synchronized"));
        ui->syncLabel->clear();
        ui->syncLabel->setPixmap(QPixmap(":/icons/synced"));
    } else {
        ui->labelTransactionsStatus->setText(tr("Synchronizing... Please wait"));
        QMovie *SYNCmovie = new QMovie(":/gifs/syncgif");
        ui->syncLabel->setMovie(SYNCmovie);
        SYNCmovie->start();// Set syncing animation
    }
}

void OverviewPage::on_viewQR_clicked()
{
    QRCodeDialog qrpage("", "test", true, this);
    qrpage.setModel(walletModel->getOptionsModel());
    qrpage.exec();
}

//
// Network logic rerporting
//
//

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

void OverviewPage::updatePoSstat(bool stat)
{
    if(stat)
    {
        uint64_t nWeight = 0;
        if (pwalletMain)
            nWeight = pwalletMain->GetStakeWeight();
        uint64_t nNetworkWeight = GetPoSKernelPS();
        bool staking = nLastCoinStakeSearchInterval && nWeight;
        uint64_t nExpectedTime = staking ? (BLOCK_SPACING * nNetworkWeight / nWeight) : 0;
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

        int nHeight = pindexBest->nHeight;

        // Net weight percentage
        double nStakePercentage = (double)nWeight / (double)nNetworkWeight * 100;
        double nNetPercentage = (100 - (double)nStakePercentage);
        if(nWeight > nNetworkWeight)
        {
            nStakePercentage = (double)nNetworkWeight / (double)nWeight * 100;
            nNetPercentage = (100 - (double)nStakePercentage);
        }
        // Sync percentage
        // (1498887616 - 1498884018 / 100) / (1498887616 - 1467348018 / 100) * 100
        double nTimeLapse = (GetTime() - pindexBest->GetBlockTime()) / 100;
        double nTimetotalLapse = (GetTime() - Params().GenesisBlock().GetBlockTime()) / 100;
        double nSyncPercentage = 100 - (nTimeLapse / nTimetotalLapse * 100);

        // Sync percentage override based on average block spacing
        CBlockIndex* pindexScan = pindexBest;
        int scanDepth = 6;
        int startDepth = 0;
        int nDayTime = 24 * 60 * 60;
        int nLastBlockLapse = GetTime() - pindexScan->GetBlockTime();
        int64_t averageSpacing;
        int64_t syncSpacing;

        // If close to genesis block, skip average block scanning
        if((GetTime() - pindexScan->GetBlockTime() + nDayTime) >= (GetTime() - Params().GenesisBlock().GetBlockTime())) {
            syncSpacing = 45 * 60;
            startDepth = 6;
        } else {
            // Initialize average spacing value
            averageSpacing = (pindexScan->GetBlockTime() - pindexScan->pprev->GetBlockTime());
        }

        // Loop to find average block spacing (an average of 7 blocks)
        while(startDepth < scanDepth) {

            // Calculate average block spacing
            pindexScan = pindexScan->pprev;
            averageSpacing += (pindexScan->GetBlockTime() - pindexScan->pprev->GetBlockTime());

            // Derrive average on final loop
            if(startDepth == 5) {
                // Set sync spacing value
                syncSpacing = (averageSpacing / 7) * 5;
            }

            // Move up per loop
            startDepth ++;
        }

        QString QStakePercentage = QString::number(nStakePercentage, 'f', 2);
        QString QNetPercentage = QString::number(nNetPercentage, 'f', 2);
        QString QSyncPercentage = QString::number(nSyncPercentage, 'f', 2);
        QString QTime = clientModel->getLastBlockDate().toString();
        QString QBlockHeight = QString::number(nHeight);
        QString QExpect = QString::number(nExpectedTime, 'f', 0);
        QString QStaking = "Disabled";
        QString QStakeEN = "Not Staking";
        QString QSynced = "Ready (Synced)";
        QString QSyncing = "Not Ready (Syncing)";
        QString QFailed = "Not Connected";
        ui->estnxt->setText(QExpect + Qseconds);
        ui->stkstat->setText(QStakeEN);
        ui->lbHeight->setText(QBlockHeight);
        ui->clStat->setText("<font color=\"red\">" + QFailed + "</font>");
        if(nExpectedTime == 0)
        {
            QExpect = "Not Staking";
            ui->estnxt->setText(QExpect);
        }
        if(staking)
        {
            QStakeEN = "Staking";
            ui->stkstat->setText(QStakeEN);
        }
        if(GetBoolArg("-staking", true))
        {
            QStaking = "Enabled";
        }
        if(nLastBlockLapse < syncSpacing)
        {
            QSyncPercentage = "100";
            nSyncPercentage = 100;
        }
        if(nSyncPercentage == 100)
            ui->clStat->setText("<font color=\"green\">" + QSynced + "</font>");
        if(nSyncPercentage != 100)
        {
            // Don't show 100% if it's not (possible due to float value)
            if(QSyncPercentage == "100.00") {
                // If QString floats to 100% (innacurate) show realistic % value
                QSyncPercentage = "99.99";
            }
            if(clientModel->getNumConnections() > 0) {
                ui->clStat->setText("<font color=\"orange\">" + QSyncing + "</font>");
            }
        }
        ui->lbTime->setText(QTime);
        ui->stken->setText(QStaking);
        ui->urweight_2->setText(QStakePercentage + "%");
        ui->netweight_2->setText(QNetPercentage + "%");
        ui->sncStatus->setText(QSyncPercentage + "%");
        return;
    }
}

void OverviewPage::setCntBlocks(int pseudo)
{
    pseudo = pindexBest->GetBlockTime();

    if(pseudo > Params().GenesisBlock().GetBlockTime())
    {
        updatePoSstat(true);
    }

    return;
}

void OverviewPage::setCntConnections(int count)
{
    ui->lbcon->setText(QString::number(count));

    int64_t pseudo = pindexBest->GetBlockTime();

    if(pseudo > Params().GenesisBlock().GetBlockTime())
    {
        updatePoSstat(true);
    }

}

void OverviewPage::on_vwalltx_clicked()
{
   BitcoinGUI::gotoHistoryPage_static();
}

void OverviewPage::setLockIconLocked(){
    ui->lockstatusLabel->setPixmap(QPixmap(":/icons/lock_closed"));
}

void OverviewPage::setLockIconUnlocked(){
    ui->lockstatusLabel->setPixmap(QPixmap(":/icons/lock_open"));
}

void OverviewPage::setLockStatus(){
    // lockstatusLabel
    if(settingsStatus)
    {
        setLockIconLocked();
    } else
    {
        setLockIconUnlocked();
    }
}
