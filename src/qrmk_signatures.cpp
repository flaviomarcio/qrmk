#include "./qrmk_signatures.h"
#include "../qstm/src/qstm_util_variant.h"
#include <QString>
#include <QLocale>

static const auto __30P="30%";

namespace QRmk{

class SignaturesPvt:public QObject{
public:
    QStringList order;
    QVariantList vList;
    Signatures *parent=nullptr;
    QList<Signature *> list;
    QHash<QString, Signature*> collection;
    Signatures::Area pageArea=Signatures::Area{"10%", "50%"};
    QString title;
    QStringList declaration;
    QString local;
    QVariant width=__30P;

    explicit SignaturesPvt(Signatures *parent):QObject{parent}
    {
        this->parent=parent;
    }

    void clear()
    {
        order.clear();
        vList.clear();
        list.clear();
        auto aux=collection.values();
        collection.clear();
        qDeleteAll(aux);
    }

    Signature &add(const QString &document)
    {
        auto name=document.trimmed().toLower();
        auto &item=collection[name];
        if(!item){
            item=new Signature{this};
            item->document(name);
            if (!this->order.contains(name))
                this->order.append(name);
        }
        return *item;
    }

    void remove(const QString &fieldName)
    {
        auto name=fieldName.trimmed().toLower();
        this->order.removeAll(name);
        auto item=collection[name];
        if(item){
            collection.remove(name);
            delete item;
        }

    }
    QVariantList &toList()
    {
        vList.clear();
        for(auto &field : list){
            if(field->document().isEmpty())
                continue;
            vList.append(field->toHash());
        }
        return vList;
    }
};

Signatures::Signatures(QObject *parent)
    : QStm::ObjectWrapper{parent}
{
    this->p=new SignaturesPvt{this};
}

bool Signatures::contains(const QString &fieldName)
{
    return p->collection.contains(fieldName.trimmed().toLower());
}

bool Signatures::isEmpty() const
{
    return p->collection.isEmpty();
}

Signatures::Area &Signatures::pageArea() const
{
    return p->pageArea;
}

Signatures &Signatures::pageArea(const Area &newArea)
{
    p->pageArea=newArea;
    return *this;
}

Signatures &Signatures::pageArea(const QVariant &width, const QVariant &height)
{
    return this->pageArea(Area{width,height});
}

Signatures &Signatures::resetPageArea()
{
    return this->pageArea(Area{"10%", "50%"});
}

Signature &Signatures::signature(const QString &document)
{
    return p->add(document);
}

const Signatures &Signatures::signature(const QString &document, const QVariant &values) const
{
    if(document.trimmed().isEmpty())
        return *this;
    auto field=&p->add(document);
    if(field)
        field->mergeFrom(values);
    return *this;
}

void Signatures::remove(const QString &document)
{
    p->remove(document);
}

QList<Signature *> &Signatures::signatures() const
{
    p->list.clear();
    for(auto &name : p->order){
        auto field=p->collection.value(name);
        if(!field)
            continue;
        p->list.append(field);
    }
    if(p->list.isEmpty()){
        auto vList=p->collection.values();
        for(auto&field:vList)
            p->list.append(field);
    }
    return p->list;
}

const QVariantList &Signatures::items() const
{
    return p->toList();
}

Signatures &Signatures::items(const QVariant &newItems)
{
    return this->setItems(newItems);
}

Signatures &Signatures::setItems(const QVariant &newItems)
{
    p->clear();
    Q_DECLARE_VU;
    auto vList=vu.toList(newItems);
    for(auto &v : vList){
        auto item=Signature::from(v, this);
        if(!item || item->document().isEmpty())continue;
        p->collection.insert(item->document(), item);
        if(!p->order.contains(item->document()))
            p->order.append(item->document());
    }
    emit itemsChanged();
    return *this;
}

Signatures &Signatures::resetItems()
{
    return setItems({});
}

QString Signatures::local() const
{
    return p->local;
}

Signatures &Signatures::local(const QString &newLocal)
{
    return this->setLocal(newLocal);
}

Signatures &Signatures::setLocal(const QString &newLocal)
{
    if (p->local == newLocal.trimmed())
        return *this;
    p->local = newLocal.trimmed();
    emit localChanged();
    return *this;
}

Signatures &Signatures::resetLocal()
{
    return setLocal({});
}

QString Signatures::localFormatted() const
{
    if(p->local.isEmpty())
        return {};


    auto brz=QLocale{QLocale::Portuguese, QLocale::Brazil};



    auto dt=QDate::currentDate();

    static const auto __formatMonth=tr("MMMM");

    auto __day=QString::number(dt.day()).rightJustified(2,'0');
    auto __month=brz.toString(dt,__formatMonth);
    auto __year=QString::number(dt.year());

    static const auto __format=tr("%1, %2 de %3 de %4.");
    return __format.arg(p->local, __day, __month, __year);

}

QStringList Signatures::declaration() const
{
    return p->declaration;
}

Signatures &Signatures::declaration(const QStringList &newDeclaration)
{
    return this->setDeclaration(newDeclaration);
}

Signatures &Signatures::declaration(const QString &newDeclaration)
{
    return this->setDeclaration({newDeclaration});
}

Signatures &Signatures::setDeclaration(const QStringList &newDeclaration)
{
    if (p->declaration == newDeclaration)
        return *this;
    p->declaration = newDeclaration;
    emit declarationChanged();
    return *this;
}

Signatures &Signatures::resetDeclaration()
{
    return setDeclaration({});
}

QString Signatures::title() const
{
    return p->title;
}

Signatures &Signatures::title(const QString &newTitle)
{
    return this->setTitle(newTitle);
}

Signatures &Signatures::setTitle(const QString &newTitle)
{
    if (p->title == newTitle)
        return *this;
    p->title = newTitle;
    emit titleChanged();
    return *this;
}

Signatures &Signatures::resetTitle()
{
    return setTitle({});
}

QVariant Signatures::width() const
{
    if(p->width.toString().trimmed().isEmpty())
        return __30P;
    return p->width;
}

Signatures &Signatures::width(const QVariant &newWidth)
{
    if (p->width == newWidth)
        return *this;
    p->width = newWidth;
    return *this;
}

Signatures &Signatures::resetWidth()
{
    return width(__30P);
}

} // namespace QOrm
