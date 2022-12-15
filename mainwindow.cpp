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
    ui->toolButton_import->setIcon(QIcon(":/images/import.png"));
    ui->toolButton_export->setIcon(QIcon(":/images/export.png"));
    ui->toolButton_open->setIcon(QIcon(":/images/open.png"));
    ui->toolButton_save->setIcon(QIcon(":/images/save.png"));
    ui->toolButton_copy->setIcon(QIcon(":/images/copy.png"));
    ui->toolButton_approve->setIcon(QIcon(":/images/approve.png"));
    ui->toolButton_search->setIcon(QIcon(":/images/search.png"));
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
    ui->maintable->setWordWrap(true);
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
    if (!isWindowModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Warning"),
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

bool MainWindow::maybeReload()
{
    if (ui->maintable->rowCount()==0)
        return false;
    const QMessageBox::StandardButton ret
        = QMessageBox::information(this, tr("Confirm Reload"),
                               tr("Do you want to reload the current project?\n"
                                  "Select no to import a new project."),
                               QMessageBox::Yes | QMessageBox::No);
    switch (ret) {
    case QMessageBox::Yes:
        return true;
    case QMessageBox::No:
        return false;
    default:
        break;
    }
    return false;
}

void MainWindow::inittable()
{
    ui->maintable->setRowCount(0);
}

void MainWindow::updatestat(mydata *i)
{
    i->stat=0;
    if(!i->lang[lang_ori].isEmpty()){
        if(i->version[lang_ori]>i->version[lang_tar])
            i->stat=1;
        if(i->version[lang_ori]==i->version[lang_tar])
            i->stat=2;
    }else{
        if(!i->lang[lang_tar].isEmpty()){
            if(i->version[lang_ori]>i->version[lang_tar])
                i->stat=3;
        }
    }
    i->stat_display->setText(stat_code(i->stat));
}

void MainWindow::parsefile(const QString &buf)
{
    mydata ans;
    //ans.id=ans.ori=ans.tar=ans.stat_display=nullptr;
    bool stat=0;
    qint64 len=buf.length(),index=0;
    QString version,id;
    QList<interlink> *interl=nullptr;
    interlink tem;
    for(qint64 i=0;i<len;i++){
        if(buf[i]=='}'){
            stat=!stat;
            if(index>MAXLANGUAGE&&index<=MAXLANGUAGE*2){
                ans.version[index-MAXLANGUAGE-1]=version.toLongLong();
                version.clear();
            }else
                if(index>MAXLANGUAGE*2){
                    if(interl==nullptr)
                        interl=new QList<interlink>;
                    if(index%2)
                        tem.type=version.toInt();
                    else{
                        tem.id=version;
                        interl->push_back(tem);
                    }
                    version.clear();
                }
            index++;
        }
        if(stat){
            if(index==0)
                id+=buf[i];
            if(index>0&&index<=MAXLANGUAGE)
                ans.lang[index-1]+=buf[i];
            if(index>MAXLANGUAGE){
                version+=buf[i];
            }

        }
        if(buf[i]=='{'){
            stat=!stat;
        }
    }
    if(id.isEmpty())
        return;
    ans.interl=interl;
    mymap.insert(id,ans);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);
    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = tr("untitled");
    setWindowTitle(QString());
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
            updaterow(&i.value(),index);
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
            if(i.value().interl!=nullptr){
                for(int j=0,l=i.value().interl->size();j<l;j++){
                    out<<"{"<<i.value().interl->at(j).type<<"}";
                    out<<"{"<<i.value().interl->at(j).id<<"}";
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
void MainWindow::updaterow(mydata *dat,int row){
    ui->maintable->setItem(row,0,dat->id);
    ui->maintable->setItem(row,1,dat->ori);
    ui->maintable->setItem(row,2,dat->stat_display);
    ui->maintable->setItem(row,3,dat->tar);
    if(row>=row2dat.size()){
        row2dat.push_back(dat);
    }else{
        row2dat[row]=dat;
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
        if(opt.lang_state[lang_index]==2){
            if(mymap[id].lang[lang_index]!=text){
                mymap[id].version[lang_index]++;
            }
        }
        mymap[id].lang[lang_index]=text;
    }
}
QString MainWindow::validatetogetrealid(const QString &fullstring,const QString &prefix){
    if(fullstring.length()<=prefix.length())
        return QString();
    int len=prefix.length();
    for(int i=0;i<len;i++){
        if(fullstring[i]!=prefix[i])
            return QString();
    }
    return fullstring.right(fullstring.length()-len);
}
QString MainWindow::getkeyfromxml(QString &fullstring,const QString pattern){
    int index=fullstring.indexOf(pattern);
    if(index<0)
        return QString();
    index+=pattern.length()+1;
    int index2=index,len=fullstring.length();
    while(index2<len&&fullstring[index2]!='\"')
        index2++;
    if(index2>index)
        return fullstring.sliced(index,index2-index);
    return QString();
}
void MainWindow::setup_connection(const QString &a,const QString &b,const int type){
    if(mymap[a].interl==nullptr)
        mymap[a].interl= new QList<interlink>;
    if(mymap[b].interl==nullptr)
        mymap[b].interl= new QList<interlink>;
    mymap[a].interl->push_back(interlink{type,b});
    mymap[b].interl->push_back(interlink{-type,a});
}
void MainWindow::parsegamedata(const QString &dir){
    //behavior and button interal name-tooltip connection
    QString buf;
    for(auto i=mymap.begin();i!=mymap.end();++i){
        buf=validatetogetrealid(i.key(),"Behavior/Name/");
        if(buf!=QString()&&mymap.contains("Behavior/Tooltip/"+buf)){
            setup_connection(i.key(),"Behavior/Tooltip/"+buf,1);
            continue;
        }
        buf=validatetogetrealid(i.key(),"Button/Name/");
        if(buf!=QString()&&mymap.contains("Button/Tooltip/"+buf)){
            setup_connection(i.key(),"Button/Tooltip/"+buf,2);
            continue;
        }
    }
    //fetch button unit(train) connection from abildata
    QFile file(dir+"Base.SC2Data/GameData/AbilData.xml");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        QString buf,unit,button;
        int flag=0;
        while (in.readLineInto(&buf)){
            buf=buf.simplified();
            if(buf.startsWith("</CAbil")){
                flag=0;
                continue;
            }
            if(flag<0)
                continue;
            if(buf.startsWith("<CAbil")){
                if(buf.startsWith("<CAbilWarpTrain")||buf.startsWith("<CAbilBuild"))
                    flag=1;
                else{
                    if(buf.startsWith("<CAbilTrain"))
                        flag=2;
                    else{
                        if(buf.startsWith("<CAbilResearch"))
                            flag=5;
                        else
                            flag=-1;
                    }
                }
                continue;
            }
            if(flag==1){
                if(buf.startsWith("<InfoArray")){
                    flag=3;
                    //unit.clear();
                    button.clear();
                    unit=getkeyfromxml(buf,"Unit=");
                }
                continue;
            }
            if(flag==3){
                if(buf.startsWith("<Button")){
                    button=getkeyfromxml(buf,"DefaultButtonFace=");
                }
                else{
                    if(buf.startsWith("</InfoArray")){
                        flag=1;
                        if(!unit.isEmpty()&&!button.isEmpty())
                            if(mymap.contains("Unit/Name/"+unit)&&mymap.contains("Button/Name/"+button))
                                setup_connection("Button/Name/"+button,"Unit/Name/"+unit,3);
                    }
                }
                continue;
            }
            if(flag==2){
                if(buf.startsWith("<InfoArray")){
                    flag=4;
                    unit.clear();
                    button.clear();
                }
                continue;
            }
            if(flag==4){
                if(buf.startsWith("<Button")){
                    button=getkeyfromxml(buf,"DefaultButtonFace=");
                }else{
                    if(buf.startsWith("<Unit")){
                        unit=getkeyfromxml(buf,"value=");
                    }else{
                        if(buf.startsWith("</InfoArray")){
                            flag=2;
                            if(!unit.isEmpty()&&!button.isEmpty())
                                if(mymap.contains("Unit/Name/"+unit)&&mymap.contains("Button/Name/"+button))
                                    setup_connection("Button/Name/"+button,"Unit/Name/"+unit,3);
                        }
                    }
                }
                continue;
            }
            if(flag==5){
                if(buf.startsWith("<InfoArray")){
                    flag=6;
                    //unit.clear();
                    button.clear();
                    unit=getkeyfromxml(buf,"Upgrade=");
                }
                continue;
            }
            if(flag==6){
                if(buf.startsWith("<Button")){
                    button=getkeyfromxml(buf,"DefaultButtonFace=");
                }
                else{
                    if(buf.startsWith("</InfoArray")){
                        flag=5;
                        if(!unit.isEmpty()&&!button.isEmpty())
                            if(mymap.contains("Upgrade/Name/"+unit)&&mymap.contains("Button/Name/"+button))
                                setup_connection("Button/Name/"+button,"Upgrade/Name/"+unit,4);
                    }
                }
                continue;
            }
        }
    }
    //fetch unit related datas
    file.close();
    file.setFileName(dir+"Base.SC2Data/GameData/UnitData.xml");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        QString buf,unit,button;
        while (in.readLineInto(&buf)){
            buf=buf.simplified();
            button.clear();
            if(buf.startsWith("</CUnit")){
                unit.clear();
                continue;
            }
            if(buf.startsWith("<CUnit")){
                unit=getkeyfromxml(buf,"id=");
                continue;
            }
            if(buf.startsWith("<BehaviorArray")){
                button=getkeyfromxml(buf,"Link=");
                if(mymap.contains("Unit/Name/"+unit)&&mymap.contains("Behavior/Name/"+button))
                    setup_connection("Unit/Name/"+unit,"Behavior/Name/"+button,5);
                continue;
            }
            if(buf.startsWith("<LayoutButtons")){
                button=getkeyfromxml(buf,"Face=");
                if(mymap.contains("Unit/Name/"+unit)&&mymap.contains("Button/Name/"+button))
                    setup_connection("Unit/Name/"+unit,"Button/Name/"+button,6);
                continue;
            }
            if(buf.startsWith("<WeaponArray")){
                button=getkeyfromxml(buf,"Link=");
                if(mymap.contains("Unit/Name/"+unit)&&mymap.contains("Weapon/Name/"+button))
                    setup_connection("Unit/Name/"+unit,"Weapon/Name/"+button,7);
                continue;
            }
        }
    }
    file.close();
}

void MainWindow::import_project(iooptions &option){
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
    QString path;
    for(int i=0;i<MAXLANGUAGE;i++){
        if(option.lang_state[i]==0)
            continue;
        path=option.dir+lang_code[i]+".SC2Data/LocalizedData/GameStrings.txt";
        import_gamestring(path,i,option);
    }
    if(option.misc[1])
        parsegamedata(option.dir);
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
            updaterow(&i.value(),index);
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

void MainWindow::searchtable(const QString key){
    preptableforupdate(true);
    bool fullsearch=key.isEmpty();
    //savetable();
    for(int i=0;i<ui->maintable->rowCount();i++){
        for(int j=0;j<4;j++){
            ui->maintable->takeItem(i,j);
        }
    }
    //ui->maintable->setRowCount(0);
    search_res.clear();
    row2dat.clear();
    for(auto i=mymap.begin();i!=mymap.end();++i){
        if(search_stat[i.value().stat]){
            if(fullsearch){
                search_res.push_back(&i.value());
            }else{
                if(search_area[0]){
                    if(i.key().contains(key)){
                        search_res.push_back(&i.value());
                        continue;
                    }
                }
                if(search_area[1]){
                    if(i.value().ori->text().contains(key)){
                        search_res.push_back(&i.value());
                        continue;
                    }
                }
                if(search_area[3]){
                    if(i.value().tar->text().contains(key)){
                        search_res.push_back(&i.value());
                    }
                }
            }
        }
    }
    int cnt=search_res.size();
    ui->maintable->setRowCount(cnt);
    for(int j=0;j<cnt;j++){
        updaterow(search_res[j],j);
    }
    preptableforupdate(false);
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
    if(maybeReload()){
        IODialog iodiag;
        iooptions options;
        if(iodiag.exec()==QDialog::Accepted){
            preptableforupdate(true);
            iodiag.getoptions(options);
            import_project(options);
            setWindowModified(true);
            preptableforupdate(false);
            ui->statusbar->showMessage(tr("Project Reloaded"), 2000);
        }
    }
    else
        if(maybeSave()){
            IODialog iodiag;
            iooptions options;
            if(iodiag.exec()==QDialog::Accepted){
                preptableforupdate(true);
                //ui->maintable->clearContents();
                ui->maintable->setRowCount(0);
                ui->relevant_strings->clear();
                mymap.clear();
                row2dat.clear();
                iodiag.getoptions(options);
                import_project(options);
                setWindowTitle(options.dir);
                setWindowModified(true);
                preptableforupdate(false);
                ui->statusbar->showMessage(tr("Project Imported"), 2000);
            }
        }
}



void MainWindow::on_actionOpen_triggered()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Galaxy Translator Project (*.galaxytrans)"));
        if (!fileName.isEmpty()){
            preptableforupdate(true);
            ui->maintable->setRowCount(0);
            ui->relevant_strings->clear();
            mymap.clear();
            row2dat.clear();
            loadFile(fileName);
            setCurrentFile(fileName);
            preptableforupdate(false);
            ui->statusbar->showMessage(tr("File opened"), 2000);
        }
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
    dialog.setNameFilter(tr("Galaxy Translator Project (*.galaxytrans)"));
    dialog.setDefaultSuffix("galaxytrans");
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}


void MainWindow::on_actionClose_triggered()
{
    if(ui->maintable->rowCount()>0){
        if(maybeSave()){
            preptableforupdate(true);
            ui->maintable->setRowCount(0);
            ui->relevant_strings->clear();
            mymap.clear();
            row2dat.clear();
            setCurrentFile(QString());
            preptableforupdate(false);
        }
    }else
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
    setWindowModified(true);
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
                setWindowModified(true);
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
    /*int h=0,w=ui->maintable->columnWidth(column);
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
    ui->maintable->setRowHeight(row,h);*/
    ui->maintable->resizeRowToContents(row);
    ui->maintable->setRowHeight(row,ui->maintable->rowHeight(row)+10);
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
                case 0: text+="ID ";text+=row2dat[j]->id->text();text+='\n';break;
                case 1: text+=tr("Source Language ");text+=row2dat[j]->ori->text();text+='\n';break;
                case 3: text+=tr("Target Language ");text+=row2dat[j]->tar->text();text+='\n';break;
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


void MainWindow::on_toolButton_search_clicked()
{
    bool chk1=true,chk2=true;
    for(int i=0;i<4;i++){
        if(past_search_area[i]!=search_area[i]){
            chk1=false;
        }
        if(past_search_stat[i]!=search_stat[i]){
            chk2=false;
            break;
        }
    }
    if(ui->searchboxtext->toPlainText()==past_search_key&&chk2&&(chk1||past_search_key.isEmpty()))
        return;
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
    past_search_key=ui->searchboxtext->toPlainText();
    for(int i=0;i<4;i++){
        past_search_area[i]=search_area[i];
        past_search_stat[i]=search_stat[i];
    }
    searchtable(past_search_key);
    #ifndef QT_NO_CURSOR
        QGuiApplication::restoreOverrideCursor();
    #endif
}


void MainWindow::on_checkBox_id_clicked()
{
    search_area[0]=!search_area[0];
}


void MainWindow::on_checkBox_ori_clicked()
{
    search_area[1]=!search_area[1];
}


void MainWindow::on_checkBox_tar_clicked()
{
    search_area[3]=!search_area[3];
}


void MainWindow::on_checkBox_nt_clicked()
{
    search_stat[1]=!search_stat[1];
}


void MainWindow::on_checkBox_tr_clicked()
{
    search_stat[2]=!search_stat[2];
}


void MainWindow::on_checkBox_pd_clicked()
{
    search_stat[3]=!search_stat[3];
}


void MainWindow::on_maintable_itemSelectionChanged()
{
    ui->relevant_strings->clear();
    auto list=ui->maintable->selectedRanges();
    if(list.size()<=0||list.size()>3)
        return;
    int row=list[0].topRow();
    for(int i=0,len=list.size();i<len;i++){
        if(list[i].topRow()!=row||list[i].bottomRow()!=row)
            return;
    }
    if(row2dat[row]->interl==nullptr)
        return;
    auto link=row2dat[row]->interl;
    QModelIndex stat2index[2][MAXLINKSTAT+1];
    int rstat;
    bool is_negative;
    //memset(stat2index,-1,sizeof(stat2index));
    for(int i=0,len=link->size();i<len;i++){
        is_negative=link->at(i).type<0;
        rstat=abs(link->at(i).type);
        if(!stat2index[is_negative][rstat].isValid()){
            QTreeWidgetItem *topitem=new QTreeWidgetItem;
            topitem->setText(0,link_stat(is_negative,rstat));
            ui->relevant_strings->addTopLevelItem(topitem);
            stat2index[is_negative][rstat]=ui->relevant_strings->indexFromItem(topitem,0);
        }
        QTreeWidgetItem *item=new QTreeWidgetItem;
        item->setText(0,mymap[link->at(i).id].ori->text());
        item->setToolTip(0,link->at(i).id);
        ui->relevant_strings->itemFromIndex(stat2index[is_negative][rstat])->addChild(item);
    }
}


void MainWindow::on_relevant_strings_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString key=item->toolTip(0);
    if(!key.isEmpty()&&mymap.contains(key)){
        ui->maintable->selectRow(mymap[key].id->row());
    }
}


void MainWindow::on_actionCopy_source_to_target_triggered()
{
    //preptableforupdate(true);
    auto tmp=ui->maintable->selectedRanges();
    mydata *k;
    for(auto i=tmp.begin();i!=tmp.end();++i){
        for(int j=i->topRow();j<=i->bottomRow();j++){
            k=row2dat[j];
            k->tar->setText(k->ori->text());
        }
    }
    //preptableforupdate(false);
}

