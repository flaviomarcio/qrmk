#pragma once

#include "../../qstm/src/qstm_object_wrapper.h"
#include "./qrmk_global.h"

namespace QRmk{
class SignaturePvt;
class Q_RMK_EXPORT Signature : public QStm::ObjectWrapper
{
    Q_OBJECT
    Q_STM_OBJECT_WRAPPER(Signature)

    Q_PROPERTY(DocumentType documentType READ documentType WRITE documentType RESET resetDocumentType NOTIFY documentTypeChanged)
    Q_PROPERTY(QString document READ document WRITE document RESET resetDocument NOTIFY documentChanged)
    Q_PROPERTY(QString name READ name WRITE name RESET resetName NOTIFY nameChanged)
    Q_PROPERTY(QStringList extraLines READ extraLines WRITE extraLines RESET resetExtraLines NOTIFY extraLinesChanged)
public:
    enum class DocumentType{
        None, CNPJ, CPF
    };

    Q_ENUM(DocumentType)

    //!
    //! \brief Signature
    //! \param parent
    //!
    explicit Signature(QObject *parent = nullptr);

    //!
    //! \brief documentType
    //! \return
    //!
    DocumentType documentType() const;
    Signature &documentType(const DocumentType &newDocumentType);
    Signature &documentType(const QVariant &newDocumentType);
    Signature &resetDocumentType();

    //!
    //! \brief document
    //! \return
    //!
    QString document() const;
    Signature &document(const QVariant &newDocument);
    Signature &resetDocument();
    QString documentFormatted() const;

    //!
    //! \brief name
    //! \return
    //!
    QString name() const;
    Signature &name(const QString &newName);
    Signature &resetName();
    QString nameFormatted();

    //!
    //! \brief extraLines
    //! \return
    //!
    QStringList extraLines() const;
    Signature &extraLines(const QStringList &newExtraLines);
    Signature &resetExtraLines();

signals:
    void documentTypeChanged();

    void documentChanged();

    void nameChanged();

    void extraLinesChanged();

private:
    SignaturePvt *p=nullptr;
};

}

