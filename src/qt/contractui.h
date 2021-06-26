#ifndef CONTRACTUI_H
#define CONTRACTUI_H

#include <QDialog>

#include "clientmodel.h"

#include <QDir>
#include <QFile>

namespace Ui {
    class ContractUI;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class ContractUI : public QDialog
{
    Q_OBJECT

public:
    explicit ContractUI(QWidget *parent = 0);
    ~ContractUI();
    void setModel(WalletModel *model);

public slots:

signals:

private:
    Ui::ContractUI *ui;
    WalletModel *model;
ClientModel *clientModel;

private slots:

void on_aplyBTN_clicked();
void on_exitBTN_clicked();
void on_manageBTN_clicked();
void on_rnderBTN_clicked();
void on_viewcBTN_clicked();
void on_clclhstmethodRADIO_clicked();
void on_cxnmethodRADIO_clicked();
};

#endif // CONTRACTUI_H
