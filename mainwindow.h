#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAXLANGUAGE 12
#include <QMainWindow>
#include <QtWidgets>

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
    struct iooptions{
        QString dir;
        int lang_state[MAXLANGUAGE];
        bool misc[2];
    };

private slots:
    void on_actionNew_File_triggered();

    void on_actionOpen_triggered();

    bool on_actionSave_triggered();

    bool on_actionSave_As_triggered();

    void on_actionClose_triggered();

    void on_lang_ori_select_activated(int index);

    void on_lang_tar_select_activated(int index);

    void on_maintable_cellChanged(int row, int column);

    void on_actionApprove_triggered();

    void on_maintable_cellDoubleClicked(int row, int column);

    void on_actionExport_Project_triggered();

    void on_actionCopy_triggered();

    void on_toolButton_search_clicked();

    void on_checkBox_id_clicked();

    void on_checkBox_ori_clicked();

    void on_checkBox_tar_clicked();

    void on_checkBox_nt_clicked();

    void on_checkBox_tr_clicked();

    void on_checkBox_pd_clicked();

private:
    Ui::MainWindow *ui;
    void inittable();
    void parsefile(const QString &buf);
    bool save();
    bool saveAs();
    bool maybeSave();
    void import_gamestring(const QString &fileName,int lang_index,iooptions &opt);
    void import_project(iooptions &option);
    void export_project(iooptions &option);
    void savetable();
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);
    void closeEvent(QCloseEvent *event);
    void SetComboBoxItemEnabled(QComboBox * comboBox, int index, bool enabled);
    bool search_area[4]={1,1,0,1},search_stat[4]={0,1,1,1};
    bool past_search_area[4]={1,1,0,1},past_search_stat[4]={0,1,1,1};
    QString past_search_key;
    QString curFile;
    QString lang_code[MAXLANGUAGE]={"deDE","enUS","esES","esMX","frFR","itIT","koKR","plPL","ptBR","ruRU","zhCN","zhTW"};
    QString stat_code[4]={"Unknown","Needs Translation","Translated","Pending Deletion"};
    int rowcount,lang_ori=-1,lang_tar=-1;
    struct mydata{
        qint64 version[MAXLANGUAGE]={};
        QString lang[MAXLANGUAGE];
        int stat=0;
        QTableWidgetItem *id=nullptr,*stat_display=nullptr,*ori=nullptr,*tar=nullptr;
    };
    void preptableforupdate(bool is);
    void updatestat(mydata *dat);
    void updaterow(mydata *dat,int row);
    void searchtable(const QString key);
    QMap<QString,mydata> mymap;
    QList<mydata *> row2dat,search_res;
};
#endif // MAINWINDOW_H
