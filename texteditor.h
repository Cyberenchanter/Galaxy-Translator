#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H
#include <QStyledItemDelegate>
#include <QtWidgets>
class texteditor : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit texteditor(QObject *parent=0);
    //创建编辑器
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //为编辑器设置数据
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    //将数据写入到模型
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    //更新编辑器布局
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    signals:

    public slots:
};

#endif // TEXTEDITOR_H
