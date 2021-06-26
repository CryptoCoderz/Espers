#ifndef NFTUI_H
#define NFTUI_H

#include <QDialog>

#include "clientmodel.h"

#include <QDir>
#include <QFile>

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

void on_btnPATH_clicked();
void on_genprevBTN_clicked();
void on_cancelBTN_clicked();
void on_createBTN_clicked();
};

#endif // NFTUI_H
