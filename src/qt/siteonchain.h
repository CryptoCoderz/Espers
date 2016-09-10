#ifndef SITEONCHAIN_H
#define SITEONCHAIN_H

#include <QWidget>

namespace Ui {
    class SiteOnChain;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class SiteOnChain : public QWidget
{
    Q_OBJECT

public:
    explicit SiteOnChain(QWidget *parent = 0);
    ~SiteOnChain();
    void setModel(SiteOnChain *model);

public slots:

signals:

private:
    Ui::SiteOnChain *ui;
        WalletModel *model;

private slots:

};

#endif // GENEREATEPAGE_H
