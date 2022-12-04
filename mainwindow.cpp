#include "mainwindow.h"
#include <QtWidgets>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCurrentFile(QString());
    ui->actionNew_File->setShortcuts(QKeySequence::New);
    ui->actionOpen->setShortcuts(QKeySequence::Open);
    ui->actionSave->setShortcuts(QKeySequence::Save);
    ui->actionSave_As->setShortcuts(QKeySequence::SaveAs);
    ui->actionClose->setShortcuts(QKeySequence::Close);
    ui->actionCut->setShortcuts(QKeySequence::Cut);
    ui->actionCopy->setShortcuts(QKeySequence::Copy);
    ui->actionPaste->setShortcuts(QKeySequence::Paste);
    //connect(ui->actionCut, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::cut);
    //connect(ui->actionCopy, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::copy);
    //connect(ui->actionPaste, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::paste);
    //disconnect(ui->actionCut, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::cut);
    // toolbar
    ui->toolButton_new->setIcon(QIcon(":/images/new.png"));
    ui->toolButton_open->setIcon(QIcon(":/images/open.png"));
    ui->toolButton_paste->setIcon(QIcon(":/images/paste.png"));
    ui->toolButton_cut->setIcon(QIcon(":/images/cut.png"));
    ui->toolButton_save->setIcon(QIcon(":/images/save.png"));
    ui->toolButton_copy->setIcon(QIcon(":/images/copy.png"));
    for(int i=0;i<MAXLANGUAGE;i++){
        ui->lang_ori_select->addItem(lang_code[i]);
        ui->lang_tar_select->addItem(lang_code[i]);
    }
    ui->lang_ori_select->setCurrentIndex(1);
    ui->lang_tar_select->setCurrentIndex(10);
    lang_ori=1;
    lang_tar=10;
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,0);
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetComboBoxItemEnabled(QComboBox * comboBox, int index, bool enabled)
{
    if(index<0)
        return;
    auto * model = qobject_cast<QStandardItemModel*>(comboBox->model());
    assert(model);
    if(!model) return;

    auto * item = model->item(index);
    assert(item);
    if(!item) return;
    item->setEnabled(enabled);
}

bool MainWindow::maybeSave()
{
    if (!ui->maintable->isWindowModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return on_actionSave_triggered();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::inittable()
{
    rowcount=0;
    ui->maintable->setRowCount(0);
}

void MainWindow::updatestat()
{
    qint64 len=mlist.size();
    for(int i=0;i<len;i++){
        if(!mlist[i].lang[lang_ori].isEmpty()){
            if(mlist[i].version[lang_ori]>mlist[i].version[lang_tar])
                mlist[i].stat=1;
            if(mlist[i].version[lang_ori]==mlist[i].version[lang_tar])
                mlist[i].stat=2;
        }else{
            if(!mlist[i].lang[lang_tar].isEmpty()){
                if(mlist[i].version[lang_ori]>mlist[i].version[lang_tar])
                    mlist[i].stat=3;
            }
        }
    }
}

void MainWindow::parsefile(const QString &buf)
{
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    QString buf;
    while (in.readLineInto(&buf)){
        parsefile(buf);
    }
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    ui->statusbar->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
//! [44] //! [45]
{
    QString errorMessage;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        qint64 len=mlist.size();
        for(qint64 i=0;i<len;i++){
            for(int j=0;j<=MAXLANGUAGE*2;j++){
                if(j==0)
                    out<<"{"<<"}";
                if(j>0&&j<=MAXLANGUAGE)
                    out<<"{"<<mlist[i].lang[j-1]<<"}";
                if(j>MAXLANGUAGE&&j<=MAXLANGUAGE*2){
                    out<<"{"<<mlist[i].version[j-1-MAXLANGUAGE]<<"}";
                }
            }
            Qt::endl(out);
        }
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    ui->statusbar->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
//! [3] //! [4]
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}
void MainWindow::import_gamestring(const QString &filename,int lang_index){
    QFile file(filename);
    if(!file.exists())
        return;
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(filename), file.errorString()));
        return;
    }

    QTextStream in(&file);
    int index;
    QString buf,id,text;
    while (in.readLineInto(&buf)){
        index=0;
        while(buf[index] != '=')
            index++;
        id=buf.left(index);
        text=buf.right(buf.length()-1-index);
        mymap[id].lang[lang_index]=text;
        if(lang_index == lang_ori){
            mymap[id].version[lang_index]++;
        }
    }
}
void MainWindow::updaterow(mydata dat,int row){
    ui->maintable->setItem(row,0,dat.id);
    ui->maintable->setItem(row,1,dat.ori);
    ui->maintable->setItem(row,2,dat.stat_display);
    ui->maintable->setItem(row,3,dat.tar);
}
void MainWindow::import_project(){
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
    QString path;
    for(int i=0;i<MAXLANGUAGE;i++){
        path=dir+lang_code[i]+".SC2Data/LocalizedData/GameStrings.txt";
        import_gamestring(path,i);
    }
    int index=0;
    for(auto i=mymap.begin();i!=mymap.end();++i){
        ui->maintable->insertRow(index);
        i.value().id = new QTableWidgetItem(i.key());
        i.value().ori = new QTableWidgetItem(i.value().lang[lang_ori]);
        i.value().tar = new QTableWidgetItem(i.value().lang[lang_tar]);
        i.value().stat_display = new QTableWidgetItem(stat_code[i.value().stat]);
        updaterow(i.value(),index);
        index++;
    }
    #ifndef QT_NO_CURSOR
        QGuiApplication::restoreOverrideCursor();
    #endif
}
void MainWindow::on_actionNew_File_triggered()
{
    if(maybeSave()){
        QFileInfo fileinfo = QFileInfo(QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("ComponentList (*.SC2Components *.SC2Mod)")));
        if(fileinfo.suffix()=="SC2Components"){
            dir=fileinfo.absolutePath();
        }else{
            if(fileinfo.suffix()=="SC2Mod"){
                dir=fileinfo.absoluteFilePath();
            }
        }
        dir+="/";
        import_project();
    }
}



void MainWindow::on_actionOpen_triggered()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}


bool MainWindow::on_actionSave_triggered()
{
    if (curFile.isEmpty()) {
        return on_actionSave_As_triggered();
    } else {
        return saveFile(curFile);
    }
}


bool MainWindow::on_actionSave_As_triggered()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}


void MainWindow::on_actionClose_triggered()
{
    close();
}


void MainWindow::on_lang_ori_select_activated(int index)
{
    if(lang_ori==index)
        return;
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,1);
    lang_ori=index;
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,0);
}


void MainWindow::on_lang_tar_select_activated(int index)
{
    if(lang_tar==index)
        return;
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,1);
    lang_tar=index;
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,0);
}


void MainWindow::on_actionNew_Item_triggered()
{
    ui->maintable->insertRow(ui->maintable->currentRow());

}

