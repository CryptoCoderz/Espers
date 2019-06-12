#include "overviewpage.h"
#include "ui_overviewpage.h"
#include "bitcoinunits.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "core/main.h"
#include "core/wallet.h"
#include "primitives/base58.h"
#include "transactionrecord.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "util/init.h"
#include "rpc/rpcserver.h"
#include "rpc/rpcclient.h"
#include "consensus/kernel.h"

using namespace json_spirit;

#include <sstream>
#include <string>

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 48
#define NUM_ITEMS 10

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
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
#if QT_VERSION < 0x050000
        if(qVariantCanConvert<QColor>(value))
#else
        if(value.canConvert(QMetaType::QColor))
#endif
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

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
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");
    ui->labelNetStatus->setText("(" + tr("out of sync") + ")");

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
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
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance + immatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;

    setCntConnections(model->getNumConnections());
    connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(setCntConnections(int)));

    setCntBlocks(model->getNumBlocks());
    connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setCntBlocks(int)));
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

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // start the display unit, uses the default ("ESP")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, walletModel->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = walletModel->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
    ui->labelNetStatus->setVisible(fShow);
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
        if((GetTime() - pindexBest->GetBlockTime()) < 45 * 60)
        {
            QSyncPercentage = "100";
            nSyncPercentage = 100;
        }
        ui->lbTime->setText(QTime);
        ui->stken->setText(QStaking);
        ui->urweight_2->setText(QStakePercentage + "%");
        ui->netweight_2->setText(QNetPercentage + "%");
        ui->sncStatus->setText(QSyncPercentage + "%");
        if(nSyncPercentage == 100)
            ui->clStat->setText("<font color=\"green\">" + QSynced + "</font>");
        if(nSyncPercentage != 100)
        {
            if(clientModel->getNumConnections() > 0)
                ui->clStat->setText("<font color=\"orange\">" + QSyncing + "</font>");
        }
        return;
    }
}

void OverviewPage::setCntBlocks(int pseudo)
{
    pseudo = pindexBest->GetBlockTime();

    if(pseudo > Params().GenesisBlock().GetBlockTime())
        updatePoSstat(true);

    return;
}

void OverviewPage::setCntConnections(int count)
{
    ui->lbcon->setText(QString::number(count));
}
