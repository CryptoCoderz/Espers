#ifndef GENERATEPAGE_H
#define GENERATEPAGE_H

#include <QWidget>

#include "clientmodel.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QSlider>

double getPoSHardness(int);
double convertPoSCoins(int64_t);
int getPoSTime(int);
int PoSInPastHours(int);
const CBlockIndex* getPoSIndex(int);

namespace Ui {
    class GeneratePage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class GeneratePage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneratePage(QWidget *parent = 0);
    ~GeneratePage();
    void setModel(WalletModel *model);

public slots:

    void blockCalled();
    void updatePoSstat(bool);

signals:

private:
    Ui::GeneratePage *ui;
        WalletModel *model;
        ClientModel *clientModel;

private slots:

};

#endif // GENEREATEPAGE_H
