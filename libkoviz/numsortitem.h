#ifndef NUMSORTITEM_H
#define NUMSORTITEM_H

#include <QStandardItem>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMetaType>
#endif

class NumSortItem : public QStandardItem
{
public:
    NumSortItem(const QString& text) : QStandardItem(text) {}

    inline virtual int userType() const
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QMetaType::Double;
#else
    return QVariant::Double;
#endif
    }

    bool operator<(const QStandardItem &other) const override
    {
        const int role = model() ? model()->sortRole() : Qt::DisplayRole;
        const QVariant l = data(role), r = other.data(role);
        return l.toDouble() < r.toDouble();
    }
};

#endif // NUMSORTITEM_H
