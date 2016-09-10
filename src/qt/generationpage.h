#ifndef GENERATIONPAGE_H
#define GENERATIONPAGE_H

#include <QDialog>

namespace Ui {
    class GenerationPage;
}
class ClientModel;
class WalletModel;

/** GenerationPage page widget */
class GenerationPage : public QWidget
{
    Q_OBJECT

public:
    explicit GenerationPage(QWidget *parent = 0);
    ~GenerationPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

public slots:

signals:

private:
    Ui::GenerationPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;


private slots:
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
};

#endif // GENERATIONPAGE_H
