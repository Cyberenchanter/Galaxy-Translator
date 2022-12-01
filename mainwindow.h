#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAXLANGUAGE 12
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
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


    void on_comboBox_textActivated(const QString &arg1);

    void on_comboBox_2_textActivated(const QString &arg1);

private:
    Ui::MainWindow *ui;
    void inittable();
    void parsefile(const QString &buf);
    bool save();
    bool saveAs();
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);
    void closeEvent(QCloseEvent *event);
    QString curFile;
    int rowcount;
    struct mydata{
        qint64 no,version[MAXLANGUAGE];
        QString id,lang[MAXLANGUAGE];
    };
    QList<mydata> mlist;
};
#endif // MAINWINDOW_H
