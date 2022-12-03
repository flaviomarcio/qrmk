#pragma once

#include <QtReforce/QStm>
#include <QList>
#include "./qrmk_global.h"
#include "./qrmk_header.h"

namespace QRmk{

class HeadersPvt;
class Q_RMK_EXPORT Headers : public QStm::ObjectWrapper
{
    Q_OBJECT
    Q_STM_OBJECT_WRAPPER(Headers)
    Q_PROPERTY(QVariantList items READ items WRITE setItems RESET resetItems NOTIFY itemsChanged)

public:
    explicit Headers(QObject *parent = nullptr);

    //!
    //! \brief contains
    //! \param fieldName
    //! \return
    //!
    bool contains(const QString &fieldName);

    //!
    //! \brief isEmpty
    //! \return
    //!
    bool isEmpty()const;

    //!
    //! \brief item
    //! \param fieldName
    //! \return
    //!
    Header &header(const QString &fieldName);
    const Headers &header(const QString &fieldName, const QVariant &values)const;

    //!
    //! \brief remove
    //! \param fieldName
    //!
    void remove(const QString &fieldName);

    //!
    //! \brief list
    //! \return
    //!
    const QList<Header*> &list()const;

    //!
    //! \brief items
    //! \return
    //!
    const QVariantList &items() const;
    Headers &items(const QVariant &newItems);
    Headers &setItems(const QVariant &newItems);
    Headers &resetItems();

signals:
    void itemsChanged();
private:
    HeadersPvt *p=nullptr;
};

}

