#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include <QWidget>

namespace Ui {
    class MessagePage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class MessagePage : public QWidget
{
    Q_OBJECT

public:
    explicit MessagePage(QWidget *parent = 0);
    ~MessagePage();
    void setModel(WalletModel *model);

public slots:

signals:

private:
    Ui::MessagePage *ui;
        WalletModel *model;

private slots:

};

#endif // MESSAGEPAGE_H
