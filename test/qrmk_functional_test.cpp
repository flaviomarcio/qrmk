#ifndef Q_RMK_DateUtilTestUnit_H
#define Q_RMK_DateUtilTestUnit_H

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include "./qrmk_test_unit.h"
#include "../src/qrmk.h"
#include "../qstm/src/qstm_util_variant.h"

namespace QRmk {

class ERP_Request_Report_Test_V1 : public SDKGoogleTestUnit {
public:
    Maker maker;

    explicit ERP_Request_Report_Test_V1():SDKGoogleTestUnit(){
        this->prepare();
    }
private:
    void prepare()
    {

        auto makeItemsSimple=[]()
        {
            Q_DECLARE_VU;
            QVariantList __return;
            int LastRow=0;

            for(int customerId=1; customerId<=10;customerId++){
                for(int day=1; day<=10; day++){
                    for(int i=1; i<=1; i++){
                        QVariantHash v;
                        int month=QDate::currentDate().month();
                        int year=QDate::currentDate().year();
                        auto dt=QDate{year, month, day};

                        v.insert("dt", dt);
                        v.insert("document01","48816017000101");
                        v.insert("document02","48816017000102");
                        v.insert("document03","48816017000103");
                        v.insert("uuid", vu.toUuid(LastRow++));
                        v.insert("customer_uuid", vu.toUuid(customerId));
                        v.insert("customer_name", "Name: "+QString::number(customerId).rightJustified(15,'0'));
                        v.insert("value",((LastRow*1.5)+i/3));
                        __return.append(v);
                    }
                }
            }


            return __return;
        };

        auto makeHeaders=[](Headers &headers)
        {
            headers
                    .header("dt")
                    .title("Date")
                    .align(Header::Center)
                    .dataType(Header::Date)
                    .width("10%");
            headers
                    .header("uuid")
                    .title("ID")
                    .align(Header::Center)
                    .dataType(Header::Uuid)
                    .visible(false);

            headers
                    .header("customer_uuid")
                    .title("ID")
                    .align(Header::Center)
                    .dataType(Header::Uuid)
                    .visible(false);

            headers
                    .header("customer_name")
                    .title("Customer name")
                    .align(Header::Start)
                    .dataType(Header::String)
                    .format("${document01} - ${customer_name}")
                    .width("55%");

            headers
                    .header("value")
                    .title("Value")
                    .align(Header::End)
                    .dataType(Header::Currency)
                    .width("15%");

            headers
                    .header("enabled")
                    .title("Status")
                    .align(Header::Center)
                    .dataType(Header::Boolean)
                    .width("10%");

            headers
                    .header("document01")
                    .format("##.###.###/####-##;0")
                    .dataType(Header::String)
                    .visible(false);

            headers
                    .header("document02")
                    .format("###x###x###/##;0")
                    .dataType(Header::String)
                    .visible(false);

            headers
                    .header("document03")
                    .format("###-###-###+##;_")
                    .dataType(Header::String)
                    .visible(false);
        };

        auto makeSummary=[](Headers &headers)
        {
            headers
                    .header("customer_name")
                    .computeMode(Header::Text);

            headers
                    .header("value")
                    .computeMode(Header::Sum);
        };

        auto makeFilters=[](Headers &)
        {
            return QVariantHash{
                {"dt",
                 QVariantHash{
                     {"title","Date"},
                     {"field","dt"},
                     {"value",QVariantHash{{"start","2021-01-01"},{"end","2021-01-15"}}},
                 }
                },
                {"uuid",
                 QVariantHash{
                     {"title","Id"},
                     {"field","uuid"},
                     {"value",QUuid::createUuid()},
                 }
                }
            };
        };

        auto makerSignature=[](Signatures &signatures)
        {
            auto declaration=QStringList
            {
                    "<p>Recebi da <strong>Empresa de Serviços</strong> LTDA com CNPJ: ",
                    "888.888.88/0001-88, a importância total de <strong>R$ 505,82 ( QUINHENTOS E CINCO ",
                    "REAIS E OITENTA E DOIS CENTAVOS )</strong> valor este discriminado acima</p> "
            };

            auto local="Sant Lois";

            signatures
                    .pageArea(Signatures::Area{"20%","30%"})
                    .title("Recibo")
                    .declaration(declaration)
                    .local(local);

            signatures
                    .signature("${document01}-One")
                    .documentType(Signature::CNPJ)
                    .name("CNPJ ${customer_name}");

            signatures
                    .signature("${document02}-Two")
                    .documentType(Signature::CPF)
                    .name("CPF ${customer_name}");

            signatures
                    .signature("${document03}-three")
                    .documentType(Signature::CPF)
                    .name("CPF ${customer_name}");
        };


        maker
                .title("Report test 01")
                .extraPageInfo({"${customer_name}-${uuid}","${uuid}-${customer_name}"})
                .owner("Company test")
                .items(makeItemsSimple())
                .filters(makeFilters)
                .headers(makeHeaders)
                .summary(makeSummary)
                .signature(makerSignature)
                .groupingFields({"customer_uuid"})
                .groupingDisplay({"Customer: ${customer_name}"})
                ;

    }

};


TEST_F(ERP_Request_Report_Test_V1, reportSimple)
{
    EXPECT_TRUE(QFile::exists(maker.make(Maker::PDF).outFileName()))<<"failure: maker.make(Maker::PDF)";
    EXPECT_TRUE(QFile::exists(maker.make(Maker::CSV).outFileName()))<<"failure: maker.make(Maker::CSV)";
    EXPECT_TRUE(QFile::exists(maker.make(Maker::TXT).outFileName()))<<"failure: maker.make(Maker::TXT)";
}

TEST_F(ERP_Request_Report_Test_V1, reportRecords)
{
    auto vList=maker.makeRecords();

    QFile file(QString("%1.json").arg(__func__));
    file.open(QFile::Truncate | QFile::WriteOnly);
    file.write(QJsonDocument::fromVariant(vList).toJson(QJsonDocument::Indented));
    file.flush();
    file.close();


    EXPECT_TRUE(!vList.isEmpty())<<"failure: maker.makeRecords()";
}

}

#endif // Q_RMK_DateUtilTestUnit_H
