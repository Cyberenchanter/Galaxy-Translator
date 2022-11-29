#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionNew_File_triggered();

    void on_plainTextEdit_textChanged();

    void on_actionOpen_triggered();

    bool on_actionSave_triggered();

    bool on_actionSave_As_triggered();

    void on_actionClose_triggered();

    void on_PlainEdit_modificationChanged(bool arg1);

    void on_PlainEdit_textChanged();

private:
    Ui::MainWindow *ui;
    bool save();
    bool saveAs();
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);
    void closeEvent(QCloseEvent *event);
    QString curFile;
};
#endif // MAINWINDOW_H
