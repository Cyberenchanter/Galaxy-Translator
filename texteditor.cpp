#include "texteditor.h"

texteditor::texteditor(QObject *parent)
    : QStyledItemDelegate{parent}
{

}
//创建编辑器，当视图需要一个编辑器时，它通知委托来为被修改的项目提供一个编辑器部件
QWidget *texteditor::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPlainTextEdit *editor = new QPlainTextEdit(parent);
    //设置滚动条出现与否
    editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    return editor;
}
void texteditor::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QPlainTextEdit newTextEdit= static_cast<QPlainTextEdit>(editor);
    newTextEdit.setPlainText( value );
}
void texteditor::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPlainTextEdit newTextEdit= static_cast<QPlainTextEdit>(editor);
    QString value = newTextEdit.toPlainText();
    model->setData(index, value, Qt::EditRole);
}
void texteditor::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
