#pragma once

#include <QtReforce/QStm>
#include <QList>
#include "./qrmk_global.h"
#include "./qrmk_signature.h"

namespace QRmk{

class SignaturesPvt;
class Q_RMK_EXPORT Signatures : public QStm::ObjectWrapper
{
    Q_OBJECT
    Q_STM_OBJECT_WRAPPER(Signatures)

    Q_PROPERTY(QVariantList items READ items WRITE setItems RESET resetItems NOTIFY itemsChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle RESET resetTitle NOTIFY titleChanged)
    Q_PROPERTY(QStringList declaration READ declaration WRITE setDeclaration RESET resetDeclaration NOTIFY declarationChanged)
    Q_PROPERTY(QString local READ local WRITE setLocal RESET resetLocal NOTIFY localChanged)
public:

    struct Area{
    public:
        QVariant width, height;
        explicit Area(const QVariant &width, const QVariant &height)
        {
            this->width=width;
            this->height=height;
        }
    };


    //!
    //! \brief Signatures
    //! \param parent
    //!
    explicit Signatures(QObject *parent = nullptr);

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
    //! \brief pageArea
    //! \return
    //!
    Area &pageArea() const;
    Signatures &pageArea(const Area &newArea);
    Signatures &pageArea(const QVariant &width, const QVariant &height);
    Signatures &resetPageArea();

    //!
    //! \brief signature
    //! \param fieldName
    //! \return
    //!
    Signature &signature(const QString &document);
    const Signatures &signature(const QString &document, const QVariant &values)const;

    //!
    //! \brief remove
    //! \param fieldName
    //!
    void remove(const QString &document);

    //!
    //! \brief list
    //! \return
    //!
    QList<Signature*> &signatures()const;

    //!
    //! \brief items
    //! \return
    //!
    const QVariantList &items() const;
    Signatures &items(const QVariant &newItems);
    Signatures &setItems(const QVariant &newItems);
    Signatures &resetItems();

    //!
    //! \brief title
    //! \return
    //!
    QString title() const;
    Signatures &title(const QString &newTitle);
    Signatures &setTitle(const QString &newTitle);
    Signatures &resetTitle();

    //!
    //! \brief declaration
    //! \return
    //!
    QStringList declaration() const;
    Signatures &declaration(const QStringList &newDeclaration);
    Signatures &declaration(const QString &newDeclaration);
    Signatures &setDeclaration(const QStringList &newDeclaration);
    Signatures &resetDeclaration();

    //!
    //! \brief local
    //! \return
    //!
    QString local() const;
    Signatures &local(const QString &newLocal);
    Signatures &setLocal(const QString &newLocal);
    Signatures &resetLocal();
    QString localFormatted() const;

    //!
    //! \brief width
    //! \return
    //!
    QVariant width() const;
    Signatures &width(const QVariant &newWidth);
    Signatures &resetWidth();

signals:
    void itemsChanged();
    void titleChanged();
    void declarationChanged();
    void localChanged();

private:
    SignaturesPvt *p=nullptr;
};

}

