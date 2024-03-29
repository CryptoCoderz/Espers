#ifndef CLIENTMODEL_H
#define CLIENTMODEL_H

#include <QObject>

class OptionsModel;
class AddressTableModel;
class BanTableModel;
class PeerTableModel;
class TransactionTableModel;
class CWallet;

QT_BEGIN_NAMESPACE
class QDateTime;
class QTimer;
QT_END_NAMESPACE

/** Model for Bitcoin network client. */
class ClientModel : public QObject
{
    Q_OBJECT

public:
    explicit ClientModel(OptionsModel *optionsModel, QObject *parent = 0);
    ~ClientModel();

    OptionsModel *getOptionsModel();
    PeerTableModel *getPeerTableModel();
    BanTableModel *getBanTableModel();

    int getNumConnections() const;
    QString getXNodeCountString() const;
    int getNumBlocks() const;
    int getNumBlocksAtStartup();

    quint64 getTotalBytesRecv() const;
    quint64 getTotalBytesSent() const;

    QDateTime getLastBlockDate() const;

    //! Return true if client connected to testnet
    bool isTestNet() const;
    //! Return true if core is doing initial block download
    bool inInitialBlockDownload() const;
    //! Return true if core is importing blocks
    bool isImporting() const;
    //! Return warnings to be displayed in status bar
    QString getStatusBarWarnings() const;

    QString formatFullVersion() const;
    QString formatBuildDate() const;
    bool isReleaseVersion() const;
    QString clientName() const;
    QString formatClientStartupTime() const;

    //! Get and set the content of the Espers.conf file
    QString getConfigFileContent() const;
    void setConfigFileContent(const QString &content);

private:
    OptionsModel *optionsModel;
    PeerTableModel *peerTableModel;
    BanTableModel *banTableModel;

    int cachedNumBlocks;
    int numBlocksAtStartup;
    QString cachedXNodeCountString;

    QTimer *pollTimer;
    QTimer *pollXnTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

signals:
    void numConnectionsChanged(int count);
    void numBlocksChanged(int count);
    void strXNodesChanged(const QString &strXNodes);
    void alertsChanged(const QString &warnings);
    void bytesChanged(quint64 totalBytesIn, quint64 totalBytesOut);
    void statusWalletLockChanged(bool unLock);

    //! Asynchronous message notification
    void message(const QString &title, const QString &message, bool modal, unsigned int style);

public slots:
    void updateTimer();
    void updateXnTimer();
    void updateNumConnections(int numConnections);
    void updateAlert(const QString &hash, int status);
    void updateBanlist();
};

#endif // CLIENTMODEL_H
