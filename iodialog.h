#ifndef IODIALOG_H
#define IODIALOG_H
#include <QDialog>
#include <QtWidgets>
#include "mainwindow.h"
namespace Ui {
class IODialog;
}

class IODialog : public QDialog
{
    Q_OBJECT

public slots:
    void getoptions(MainWindow::iooptions &options);
public:
    explicit IODialog(QWidget *parent = nullptr);
    ~IODialog();

private slots:
    void on_comp_select_clicked();


private:
    Ui::IODialog *ui;
    QString dir;
    QComboBox *combo[MAXLANGUAGE];
    QString lang_code[MAXLANGUAGE]={"deDE","enUS","esES","esMX","frFR","itIT","koKR","plPL","ptBR","ruRU","zhCN","zhTW"};
    void updatedir();
};

#endif // IODIALOG_H
