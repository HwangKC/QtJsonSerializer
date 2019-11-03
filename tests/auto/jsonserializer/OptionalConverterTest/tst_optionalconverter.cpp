#include <QtTest>
#include <QtJsonSerializer>

#include "typeconvertertestbase.h"

#include <QtJsonSerializer/private/qjsonstdoptionalconverter_p.h>

#include <optional>

using ExtendedOptional = std::optional<std::pair<int, bool>>;

Q_DECLARE_METATYPE(std::optional<int>)
Q_DECLARE_METATYPE(ExtendedOptional)
Q_DECLARE_METATYPE(std::optional<OpaqueDummy>)

class OptionalConverterTest : public TypeConverterTestBase
{
	Q_OBJECT

protected:
	void initTest() override;
	QJsonTypeConverter *converter() override;
	void addConverterData() override;
	void addMetaData() override;
	void addCommonSerData() override;
	void addSerData() override;

private:
	QJsonStdOptionalConverter _converter;
};

void OptionalConverterTest::initTest()
{
	QJsonSerializer::registerOptionalConverters<int>();
	QJsonSerializer::registerOptionalConverters<std::pair<int, bool>>();
	QJsonSerializer::registerOptionalConverters<OpaqueDummy>();

	QMetaType::registerEqualsComparator<std::optional<int>>();
}

QJsonTypeConverter *OptionalConverterTest::converter()
{
	return &_converter;
}

void OptionalConverterTest::addConverterData()
{
	QTest::newRow("optional") << static_cast<int>(QJsonTypeConverter::Standard);
}

void OptionalConverterTest::addMetaData()
{
	QTest::newRow("basic") << qMetaTypeId<std::optional<int>>()
						   << static_cast<QCborTag>(QCborSerializer::NoTag)
						   << QCborValue::Double
						   << true
						   << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("extended") << qMetaTypeId<std::optional<std::pair<int, bool>>>()
							  << static_cast<QCborTag>(QCborSerializer::NoTag)
							  << QCborValue::Null
							  << true
							  << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("invalid") << qMetaTypeId<QList<int>>()
							 << static_cast<QCborTag>(QCborSerializer::NoTag)
							 << QCborValue::Integer
							 << false
							 << QJsonTypeConverter::DeserializationCapabilityResult::Negative;
}

void OptionalConverterTest::addCommonSerData()
{
	QTest::newRow("basic") << QVariantHash{}
						   << TestQ{{QMetaType::Int, 5, 1}}
						   << static_cast<QObject*>(nullptr)
						   << qMetaTypeId<std::optional<int>>()
						   << QVariant::fromValue(std::optional<int>{5})
						   << QCborValue{1}
						   << QJsonValue{1};
	QTest::newRow("nullopt") << QVariantHash{}
							 << TestQ{{QMetaType::Nullptr, QVariant::fromValue(nullptr), QJsonValue::Null}}
							 << static_cast<QObject*>(nullptr)
							 << qMetaTypeId<std::optional<int>>()
							 << QVariant::fromValue<std::optional<int>>(std::nullopt)
							 << QCborValue{QCborValue::Null}
							 << QJsonValue{QJsonValue::Null};
}

void OptionalConverterTest::addSerData()
{
	QTest::newRow("invalid.unconvertible") << QVariantHash{}
										   << TestQ{}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<std::optional<OpaqueDummy>>()
										   << QVariant::fromValue(std::optional<OpaqueDummy>{OpaqueDummy{}})
										   << QCborValue{}
										   << QJsonValue{QJsonValue::Undefined};
}

QTEST_MAIN(OptionalConverterTest)

#include "tst_optionalconverter.moc"
