#ifndef NFTUI_H
#define NFTUI_H

#include <QDialog>

#include "clientmodel.h"
#include "core/main.h"
#include "core/wallet.h"
#include "primitives/base58.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QSlider>

double getBlockHardness(int);
double getTxTotalValue(std::string);
double convertCoins(int64_t);
double getTxFees(std::string);
int getBlockTime(int);
int getBlocknBits(int);
int getBlockNonce(int);
int blocksInPastHours(int);
int getBlockHashrate(int);
std::string getInputs(std::string);
std::string getOutputs(std::string);
std::string getBlockHash(int);
std::string getBlockMerkle(int);
bool addnode(std::string);
const CBlockIndex* getBlockIndex(int);
int64_t getInputValue(CTransaction, CScript);

namespace Ui {
    class NftUI;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class NftUI : public QDialog
{
    Q_OBJECT

public:
    explicit NftUI(QWidget *parent = 0);
    ~NftUI();
    void setModel(WalletModel *model);

public slots:

signals:

private:
    Ui::NftUI *ui;
    WalletModel *model;
ClientModel *clientModel;

private slots:

};

#endif // NFTUI_H
