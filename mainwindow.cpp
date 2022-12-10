#include "mainwindow.h"
#include "iodialog.h"
#include "texteditor.h"
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
    ui->lang_ori_select->setCurrentIndex(10);
    ui->lang_tar_select->setCurrentIndex(1);
    lang_ori=10;
    lang_tar=1;
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,0);
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,0);
    // 表格可拖动
    ui->maintable->setMouseTracking(true);
    ui->maintable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->maintable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
    ui->maintable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    texteditor* deleagater = new texteditor;
    //ui->maintable->setItemDelegateForColumn(0,deleagater);
    ui->maintable->setItemDelegateForColumn(1,deleagater);
    ui->maintable->setItemDelegateForColumn(3,deleagater);
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

void MainWindow::updatestat(mydata *i)
{
    if(!i->lang[lang_ori].isEmpty()){
        if(i->version[lang_ori]>i->version[lang_tar])
            i->stat=1;
        if(i->version[lang_ori]==i->version[lang_tar])
            i->stat=2;
    }else{
        if(!i->lang[lang_tar].isEmpty()){
            if(i->version[lang_ori]>i->version[lang_tar])
                i->stat=3;
        }else{
            i->stat=0;
        }
    }
    i->stat_display->setData(0,stat_code[i->stat]);
}

void MainWindow::parsefile(const QString &buf)
{
    mydata ans;
    //ans.id=ans.ori=ans.tar=ans.stat_display=nullptr;
    bool stat=0;
    qint64 len=buf.length(),index=0;
    QString version,id;
    for(qint64 i=0;i<len;i++){
        if(buf[i]=='}'){
            stat=!stat;
            if(index>MAXLANGUAGE&&index<=MAXLANGUAGE*2){
                ans.version[index-MAXLANGUAGE-1]=version.toLongLong();
                version.clear();
            }
            index++;
        }
        if(stat){
            if(index==0)
                id+=buf[i];
            if(index>0&&index<=MAXLANGUAGE)
                ans.lang[index-1]+=buf[i];
            if(index>MAXLANGUAGE&&index<=MAXLANGUAGE*2){
                version+=buf[i];
            }

        }
        if(buf[i]=='{'){
            stat=!stat;
        }
    }
    if(id.isEmpty())
        return;
    mymap.insert(id,ans);
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
    int index=ui->maintable->rowCount();
    for(auto i=mymap.begin();i!=mymap.end();++i){
        if(i.value().id==nullptr){
            ui->maintable->insertRow(index);
            i.value().id = new QTableWidgetItem(i.key());
            i.value().id->setFlags(i.value().id->flags() & (~Qt::ItemIsEditable));
            i.value().ori = new QTableWidgetItem(i.value().lang[lang_ori]);
            i.value().tar = new QTableWidgetItem(i.value().lang[lang_tar]);
            i.value().stat_display = new QTableWidgetItem();
            i.value().stat_display->setFlags(i.value().stat_display->flags() & (~Qt::ItemIsEditable));
            updaterow(i.value(),index);
            index++;
        }else{
            i.value().ori->setText(i.value().lang[lang_ori]);
            i.value().tar->setText(i.value().lang[lang_tar]);
        }
        updatestat(&i.value());
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
    savetable();
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        for(auto i=mymap.begin();i!=mymap.end();++i){
            for(int j=0;j<=MAXLANGUAGE*2;j++){
                if(j==0)
                    out<<"{"<<i.key()<<"}";
                if(j>0&&j<=MAXLANGUAGE)
                    out<<"{"<<i.value().lang[j-1]<<"}";
                if(j>MAXLANGUAGE&&j<=MAXLANGUAGE*2){
                    out<<"{"<<i.value().version[j-1-MAXLANGUAGE]<<"}";
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
void MainWindow::updaterow(mydata &dat,int row){
    ui->maintable->setItem(row,0,dat.id);
    ui->maintable->setItem(row,1,dat.ori);
    ui->maintable->setItem(row,2,dat.stat_display);
    ui->maintable->setItem(row,3,dat.tar);
    if(row>=row2dat.size()){
        row2dat.push_back(&dat);
    }else{
        row2dat[row]=&dat;
    }
}
void MainWindow::import_gamestring(const QString &filename,int lang_index,iooptions &opt){
    if(opt.lang_state[lang_index]<=0||opt.lang_state[lang_index]>2)
        return;
    QFile file(filename);
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
        if(opt.misc[0]){
            if(id.startsWith("Abil"))
                continue;
            if(id.startsWith("Effect"))
                continue;
        }
        text=buf.right(buf.length()-1-index);
        if(opt.lang_state[lang_index]==1){
            if(mymap[id].lang[lang_index].isEmpty())
                mymap[id].lang[lang_index]=text;
        }else{
            if(mymap[id].lang[lang_index]!=text){
                mymap[id].lang[lang_index]=text;
                mymap[id].version[lang_index]++;
            }
        }
    }
}
void MainWindow::import_project(iooptions &option){
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
    QString path;
    for(int i=0;i<MAXLANGUAGE;i++){
        path=option.dir+lang_code[i]+".SC2Data/LocalizedData/GameStrings.txt";
        import_gamestring(path,i,option);
    }
    int index=ui->maintable->rowCount();
    for(auto i=mymap.begin();i!=mymap.end();++i){
        if(i.value().id==nullptr){
            ui->maintable->insertRow(index);
            i.value().id = new QTableWidgetItem(i.key());
            i.value().id->setFlags(i.value().id->flags() & (~Qt::ItemIsEditable));
            i.value().ori = new QTableWidgetItem(i.value().lang[lang_ori]);
            i.value().tar = new QTableWidgetItem(i.value().lang[lang_tar]);
            i.value().stat_display = new QTableWidgetItem();
            i.value().stat_display->setFlags(i.value().stat_display->flags() & (~Qt::ItemIsEditable) & (~Qt::ItemIsSelectable));
            updaterow(i.value(),index);
            index++;
        }else{
            i.value().ori->setText(i.value().lang[lang_ori]);
            i.value().tar->setText(i.value().lang[lang_tar]);
        }
        updatestat(&i.value());
    }
    #ifndef QT_NO_CURSOR
        QGuiApplication::restoreOverrideCursor();
    #endif
}

void MainWindow::export_project(iooptions &option){
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
    QString path;
    for(int i=0;i<MAXLANGUAGE;i++){
        if(option.lang_state[i]<=0||option.lang_state[i]>1)
            continue;
        path=option.dir+lang_code[i]+".SC2Data/LocalizedData/GameStrings.txt";
        QFile file(path);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot write to file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(path), file.errorString()));
            return;
        }
        QTextStream out(&file);
        for(auto j=mymap.begin();j!=mymap.end();++j){
//            if(option.misc[0]){
//                if(j.key().startsWith("Abil"))
//                    continue;
//                if(j.key().startsWith("Effect"))
//                    continue;
//            }
            out<<j.key()<<"="<<j.value().lang[i];
            Qt::endl(out);
        }
        file.close();
    }
    #ifndef QT_NO_CURSOR
        QGuiApplication::restoreOverrideCursor();
    #endif
}

void MainWindow::savetable()
{
    for(auto i=mymap.begin();i!=mymap.end();++i){
        i.value().lang[lang_ori]=i.value().ori->text();
        i.value().lang[lang_tar]=i.value().tar->text();
    }
}
void MainWindow::preptableforupdate(bool is)
{
    if(is){
        ui->maintable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
        ui->maintable->blockSignals(true);
    }else{
        ui->maintable->blockSignals(false);
        ui->maintable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    }
}
void MainWindow::on_actionNew_File_triggered()
{
//    if(maybeSave()){
//        ui->maintable->blockSignals(true);
//        QFileInfo fileinfo = QFileInfo(QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("ComponentList (*.SC2Components *.SC2Mod)")));
//        if(fileinfo.suffix()=="SC2Components"){
//            dir=fileinfo.absolutePath();
//        }else{
//            if(fileinfo.suffix()=="SC2Mod"){
//                dir=fileinfo.absoluteFilePath();
//            }
//        }
//        dir+="/";
//        import_project();
//        ui->maintable->blockSignals(false);
//    }
    IODialog iodiag;
    iooptions options;
    if(iodiag.exec()==QDialog::Accepted){
        preptableforupdate(true);
        iodiag.getoptions(options);
        import_project(options);
        preptableforupdate(false);
    }
}



void MainWindow::on_actionOpen_triggered()
{
    if (maybeSave()) {
        preptableforupdate(true);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Galaxy Translator Project (*.galaxytrans)"));
        if (!fileName.isEmpty())
            loadFile(fileName);
        preptableforupdate(false);
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
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("Galaxy Translator Project (*.galaxytrans)");
    dialog.setDefaultSuffix("galaxytrans");
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
    preptableforupdate(true);
    savetable();
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,1);
    lang_ori=index;
    SetComboBoxItemEnabled(ui->lang_tar_select,lang_ori,0);
    for(auto i=mymap.begin();i!=mymap.end();++i){
        i.value().ori->setText(i.value().lang[lang_ori]);
        updatestat(&i.value());
    }
    preptableforupdate(false);
}


void MainWindow::on_lang_tar_select_activated(int index)
{
    if(lang_tar==index)
        return;
    preptableforupdate(true);
    savetable();
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,1);
    lang_tar=index;
    SetComboBoxItemEnabled(ui->lang_ori_select,lang_tar,0);
    for(auto i=mymap.begin();i!=mymap.end();++i){
        i.value().tar->setText(i.value().lang[lang_tar]);
        updatestat(&i.value());
    }
    preptableforupdate(false);
}


void MainWindow::on_maintable_cellChanged(int row, int column)
{
    if(column==1){
        row2dat[row]->version[lang_ori]++;
        updatestat(row2dat[row]);
    }
}

void MainWindow::on_actionApprove_triggered()
{
    preptableforupdate(true);
    auto tmp=ui->maintable->selectedRanges();
    mydata *k;
    for(auto i=tmp.begin();i!=tmp.end();++i){
        for(int j=i->topRow();j<=i->bottomRow();j++){
            k=row2dat[j];
            if(k->stat==1){
                k->version[lang_tar]=k->version[lang_ori];
                updatestat(k);
            }else{
                if(k->stat==3){
                    k->lang[lang_tar]=QString();
                    k->tar->setText(QString());
                    k->version[lang_tar]=k->version[lang_ori];
                    updatestat(k);
                }
            }
        }
    }
    preptableforupdate(false);
}

void MainWindow::on_maintable_cellDoubleClicked(int row, int column)
{
    if(column!=1&&column!=3)
        return;
    int h=0,w=ui->maintable->columnWidth(column);
    QTableWidgetItem *item;
    QLabel tem;
    tem.setMaximumWidth(w);
    tem.setMinimumWidth(w);
    tem.setWordWrap(true);
    item=row2dat[row]->ori;
    tem.setText(item->text());
    tem.adjustSize();
    h=tem.height();
    item=row2dat[row]->tar;
    tem.setText(item->text());
    tem.adjustSize();
    if(tem.height()>h)
        h=tem.height();
    h+=30;
    if(h<=ui->maintable->rowHeight(row))
        return;
    ui->maintable->setRowHeight(row,h);
}


void MainWindow::on_actionExport_Project_triggered()
{
    savetable();
    IODialog iodiag(this,true);
    iooptions options;
    if(iodiag.exec()==QDialog::Accepted){
        iodiag.getoptions(options);
        export_project(options);
    }
}


void MainWindow::on_actionCopy_triggered()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString text;
    auto tmp=ui->maintable->selectedRanges();
    if(tmp.size()<=0)
        return;
    if(tmp.size()==1){
        auto tmp2=tmp.begin();
        if(tmp2->topRow()==tmp2->bottomRow()&&tmp2->leftColumn()==tmp2->rightColumn()){
            switch(tmp2->leftColumn()){
            case 0: text=row2dat[tmp2->topRow()]->id->text();break;
            case 1: text=row2dat[tmp2->topRow()]->ori->text();break;
            case 3: text=row2dat[tmp2->topRow()]->tar->text();break;
            }
            clipboard->setText(text, QClipboard::Clipboard);
            if (clipboard->supportsSelection()) {
                clipboard->setText(text, QClipboard::Selection);
            }
            #if defined(Q_OS_LINUX)
                QThread::msleep(1); //workaround for copied text not being available...
            #endif
            return;
        }
    }
    for(auto i=tmp.begin();i!=tmp.end();++i){
        for(int j=i->topRow();j<=i->bottomRow();j++){
            for(int k=i->leftColumn();k<=i->rightColumn();k++){
                switch(k){
                case 0: text+=QString("ID \n");text+=row2dat[j]->id->text();text+='\n';break;
                case 1: text+=QString("Source Language \n");text+=row2dat[j]->ori->text();text+='\n';break;
                case 3: text+=QString("Target Language \n");text+=row2dat[j]->tar->text();text+='\n';break;
                }
            }
        }
    }
    clipboard->setText(text, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) {
        clipboard->setText(text, QClipboard::Selection);
    }
    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

