#ifndef IODIALOG_H
#define IODIALOG_H

#include <QDialog>

namespace Ui {
class IODialog;
}

class IODialog : public QDialog
{
    Q_OBJECT

public:
    explicit IODialog(QWidget *parent = nullptr);
    ~IODialog();

private:
    Ui::IODialog *ui;
};

#endif // IODIALOG_H