#include "./qrmk_headers.h"
#include "../../qstm/src/qstm_util_variant.h"
#include <QString>

namespace QRmk{

//static const auto __P="%";

class HeadersPvt:public QObject{
public:
    QStringList order;
    QVariantList vList;
    Headers *parent=nullptr;
    QList<Header *> list;
    QHash<QString, Header*> collection;

    explicit HeadersPvt(Headers *parent):QObject{parent}
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

    Header *at(const QString &fieldName)
    {
        return collection.value(fieldName.toLower());
    }

    Header &add(const QString &fieldName)
    {
        auto name=fieldName.trimmed().toLower();
        if(name.isEmpty())
            name=QUuid::createUuid().toString();

        auto &item=collection[name];
        if(!item){
            item=new Header{this};
            item->field(name);
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
            if(field->field().isEmpty())
                continue;
            vList.append(field->toHash());
        }
        return vList;
    }
};

Headers::Headers(QObject *parent)
    : QStm::ObjectWrapper{parent}
{
    this->p=new HeadersPvt{this};
}

bool Headers::contains(const QString &fieldName)
{
    return p->collection.contains(fieldName.trimmed().toLower());
}

bool Headers::isEmpty() const
{
    return p->collection.isEmpty();
}

Header *Headers::at(const QString &fieldName) const
{
    return p->at(fieldName);
}

Header &Headers::header(const QString &fieldName)
{
    return p->add(fieldName);
}

const Headers &Headers::header(const QString &fieldName, const QVariant &values) const
{
    if(fieldName.trimmed().isEmpty())
        return *this;
    auto field=&p->add(fieldName);
    if(field)
        field->mergeFrom(values);
    return *this;
}

void Headers::remove(const QString &fieldName)
{
    p->remove(fieldName);
}

const QList<Header *> &Headers::list() const
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
        for(auto &field:vList)
            p->list.append(field);
    }
    return p->list;
}

const QVariantList &Headers::items() const
{
    return p->toList();
}

Headers &Headers::items(const QVariant &newItems)
{
    return this->setItems(newItems);
}

Headers &Headers::setItems(const QVariant &newItems)
{
    p->clear();
    Q_DECLARE_VU;
    auto vList=vu.toList(newItems);
    for(auto &v : vList){
        auto item=Header::from(v, this);
        if(!item || item->field().isEmpty())continue;
        p->collection.insert(item->field(), item);
        if(!p->order.contains(item->field()))
            p->order.append(item->field());
    }
    emit itemsChanged();
    return *this;
}

Headers &Headers::resetItems()
{
    return setItems({});
}

} // namespace QOrm
