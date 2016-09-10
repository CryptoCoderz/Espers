#ifndef GENERATEPAGE_H
#define GENERATEPAGE_H

#include <QWidget>

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

signals:

private:
    Ui::GeneratePage *ui;
        WalletModel *model;

private slots:

};

#endif // GENEREATEPAGE_H
