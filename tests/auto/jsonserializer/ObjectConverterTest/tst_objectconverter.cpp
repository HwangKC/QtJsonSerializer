#include <QtTest>
#include <QtJsonSerializer>

#include "typeconvertertestbase.h"
#include "testobject.h"

#include <QtJsonSerializer/private/qjsonobjectconverter_p.h>

class ObjectConverterTest : public TypeConverterTestBase
{
	Q_OBJECT

protected:
	void initTest() override;

	QJsonTypeConverter *converter() override;
	void addConverterData() override;
	void addMetaData() override;
	void addCommonSerData() override;
	void addSerData() override;
	void addDeserData() override;

	bool compare(int type, QVariant &actual, QVariant &expected, const char *aName, const char *eName, const char *file, int line) override;

private:
	QJsonObjectConverter _converter;
};

void ObjectConverterTest::initTest()
{
	qRegisterMetaType<TestObject*>();
	qRegisterMetaType<StaticPolyObject*>();
	qRegisterMetaType<StaticNonPolyObject*>();
	qRegisterMetaType<DynamicPolyObject*>();
	qRegisterMetaType<DerivedTestObject*>();
	qRegisterMetaType<BrokenObject*>();

	QJsonSerializer::registerPointerConverters<TestObject>();
}

QJsonTypeConverter *ObjectConverterTest::converter()
{
	return &_converter;
}

void ObjectConverterTest::addConverterData()
{
	QTest::newRow("object") << static_cast<int>(QJsonTypeConverter::Standard);
}

void ObjectConverterTest::addMetaData()
{
	QTest::newRow("basic") << qMetaTypeId<TestObject*>()
						   << QJsonTypeConverter::NoTag
						   << QCborValue::Map
						   << true
						   << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("null") << qMetaTypeId<TestObject*>()
						  << QJsonTypeConverter::NoTag
						  << QCborValue::Null
						  << true
						  << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("base") << static_cast<int>(QMetaType::QObjectStar)
						  << QJsonTypeConverter::NoTag
						  << QCborValue::Map
						  << true
						  << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("tracking") << qMetaTypeId<QPointer<TestObject>>()
							  << QJsonTypeConverter::NoTag
							  << QCborValue::Map
							  << true
							  << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("shared") << qMetaTypeId<QSharedPointer<TestObject>>()
							<< QJsonTypeConverter::NoTag
							<< QCborValue::Map
							<< true
							<< QJsonTypeConverter::DeserializationCapabilityResult::Positive;

	QTest::newRow("generic") << qMetaTypeId<TestObject*>()
							 << static_cast<QCborTag>(QCborSerializer::GenericObject)
							 << QCborValue::Array
							 << true
							 << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("constructed.array") << qMetaTypeId<TestObject*>()
									   << static_cast<QCborTag>(QCborSerializer::ConstructedObject)
									   << QCborValue::Array
									   << true
									   << QJsonTypeConverter::DeserializationCapabilityResult::Positive;
	QTest::newRow("constructed.null") << qMetaTypeId<TestObject*>()
									  << static_cast<QCborTag>(QCborSerializer::ConstructedObject)
									  << QCborValue::Null
									  << true
									  << QJsonTypeConverter::DeserializationCapabilityResult::Positive;

	QTest::newRow("invalid.type1") << qMetaTypeId<OpaqueDummyGadget*>()
								   << QJsonTypeConverter::NoTag
								   << QCborValue::Map
								   << false
								   << QJsonTypeConverter::DeserializationCapabilityResult::Negative;
	QTest::newRow("invalid.type2") << qMetaTypeId<OpaqueDummyGadget>()
								   << QJsonTypeConverter::NoTag
								   << QCborValue::Map
								   << false
								   << QJsonTypeConverter::DeserializationCapabilityResult::Negative;
	QTest::newRow("invalid.type") << qMetaTypeId<TestObject*>()
								  << static_cast<QCborTag>(QCborSerializer::ConstructedObject)
								  << QCborValue::Map
								  << true
								  << QJsonTypeConverter::DeserializationCapabilityResult::Negative;
	QTest::newRow("invalid.tag") << qMetaTypeId<TestObject*>()
								 << static_cast<QCborTag>(QCborSerializer::Enum)
								 << QCborValue::Map
								 << true
								 << QJsonTypeConverter::DeserializationCapabilityResult::WrongTag;

	QTest::newRow("guessed.generic") << static_cast<int>(QMetaType::UnknownType)
									 << static_cast<QCborTag>(QCborSerializer::GenericObject)
									 << QCborValue::Array
									 << false
									 << QJsonTypeConverter::DeserializationCapabilityResult::Guessed;
	QTest::newRow("guessed.constructed") << static_cast<int>(QMetaType::UnknownType)
										 << static_cast<QCborTag>(QCborSerializer::ConstructedObject)
										 << QCborValue::Array
										 << false
										 << QJsonTypeConverter::DeserializationCapabilityResult::Guessed;
}

void ObjectConverterTest::addCommonSerData()
{
	QTest::newRow("basic") << QVariantHash{}
						   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
						   << static_cast<QObject*>(nullptr)
						   << qMetaTypeId<TestObject*>()
						   << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
						   << QCborValue{QCborMap{
									{QStringLiteral("key"), 1},
									{QStringLiteral("value"), 2}
								}}
						   << QJsonValue{QJsonObject{
									{QStringLiteral("key"), 1},
									{QStringLiteral("value"), 2}
								}};
	QTest::newRow("tracking") << QVariantHash{}
							  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
							  << static_cast<QObject*>(nullptr)
							  << qMetaTypeId<QPointer<TestObject>>()
							  << QVariant::fromValue<QPointer<TestObject>>(new TestObject{10, 0.1, 11, this})
							  << QCborValue{QCborMap{
									 {QStringLiteral("key"), 1},
									 {QStringLiteral("value"), 2}
								 }}
							  << QJsonValue{QJsonObject{
										{QStringLiteral("key"), 1},
										{QStringLiteral("value"), 2}
									}};
	QTest::newRow("shared") << QVariantHash{}
							<< TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
							<< static_cast<QObject*>(nullptr)
							<< qMetaTypeId<QSharedPointer<TestObject>>()
							<< QVariant::fromValue(QSharedPointer<TestObject>::create(10, 0.1, 11))
							<< QCborValue{QCborMap{
								   {QStringLiteral("key"), 1},
								   {QStringLiteral("value"), 2}
							   }}
							<< QJsonValue{QJsonObject{
									{QStringLiteral("key"), 1},
									{QStringLiteral("value"), 2}
								}};

	QTest::newRow("null.basic") << QVariantHash{}
								<< TestQ{}
								<< static_cast<QObject*>(nullptr)
								<< qMetaTypeId<TestObject*>()
								<< QVariant::fromValue<TestObject*>(nullptr)
								<< QCborValue{QCborValue::Null}
								<< QJsonValue{QJsonValue::Null};
	QTest::newRow("null.tracking") << QVariantHash{}
								   << TestQ{}
								   << static_cast<QObject*>(nullptr)
								   << qMetaTypeId<QPointer<TestObject>>()
								   << QVariant::fromValue<QPointer<TestObject>>(nullptr)
								   << QCborValue{QCborValue::Null}
								   << QJsonValue{QJsonValue::Null};
	QTest::newRow("null.shared") << QVariantHash{}
								 << TestQ{}
								 << static_cast<QObject*>(nullptr)
								 << qMetaTypeId<QSharedPointer<TestObject>>()
								 << QVariant::fromValue<QSharedPointer<TestObject>>(nullptr)
								 << QCborValue{QCborValue::Null}
								 << QJsonValue{QJsonValue::Null};

	QTest::newRow("name.keep") << QVariantHash{{QStringLiteral("keepObjectName"), true}}
							   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::QString, QStringLiteral("TestObject"), 3}}
							   << static_cast<QObject*>(nullptr)
							   << qMetaTypeId<TestObject*>()
							   << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
							   << QCborValue{QCborMap{
									  {QStringLiteral("objectName"), 3},
									  {QStringLiteral("key"), 1},
									  {QStringLiteral("value"), 2},
								  }}
							   << QJsonValue{QJsonObject{
										{QStringLiteral("key"), 1},
										{QStringLiteral("value"), 2},
										{QStringLiteral("objectName"), 3}
									}};
	QTest::newRow("name.nokeep") << QVariantHash{{QStringLiteral("keepObjectName"), false}}
								 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
								 << static_cast<QObject*>(nullptr)
								 << qMetaTypeId<TestObject*>()
								 << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
								 << QCborValue{QCborMap{
										{QStringLiteral("key"), 1},
										{QStringLiteral("value"), 2}
									}}
								 << QJsonValue{QJsonObject{
										  {QStringLiteral("key"), 1},
										  {QStringLiteral("value"), 2}
									  }};

	QTest::newRow("poly.enabled.static") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Enabled)}}
										 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
										 << static_cast<QObject*>(nullptr)
										 << qMetaTypeId<TestObject*>()
										 << QVariant::fromValue<TestObject*>(new StaticPolyObject{10, 0.1, 11, true, this})
										 << QCborValue{QCborMap{
												   {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra1"), 3}
											   }}
										 << QJsonValue{QJsonObject{
												   {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra1"), 3}
											   }};
	QTest::newRow("poly.enabled.dynamic.enabled") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Enabled)}}
												  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
												  << static_cast<QObject*>(nullptr)
												  << qMetaTypeId<TestObject*>()
												  << QVariant::fromValue<TestObject*>(new DynamicPolyObject{10, 0.1, 11, true, true, this})
												  << QCborValue{QCborMap{
															{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
															{QStringLiteral("key"), 1},
															{QStringLiteral("value"), 2},
															{QStringLiteral("extra3"), 3}
														}}
												  << QJsonValue{QJsonObject{
															{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
															{QStringLiteral("key"), 1},
															{QStringLiteral("value"), 2},
															{QStringLiteral("extra3"), 3}
														}};

	QTest::newRow("poly.forced.basic") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
									   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
									   << static_cast<QObject*>(nullptr)
									   << qMetaTypeId<TestObject*>()
									   << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
									   << QCborValue{QCborMap{
												{QStringLiteral("@class"), QStringLiteral("TestObject")},
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2}
											}}
									   << QJsonValue{QJsonObject{
												{QStringLiteral("@class"), QStringLiteral("TestObject")},
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2}
											}};
	QTest::newRow("poly.forced.static") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
										<< TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
										<< static_cast<QObject*>(nullptr)
										<< qMetaTypeId<TestObject*>()
										<< QVariant::fromValue<TestObject*>(new StaticPolyObject{10, 0.1, 11, true, this})
										<< QCborValue{QCborMap{
												  {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												  {QStringLiteral("key"), 1},
												  {QStringLiteral("value"), 2},
												  {QStringLiteral("extra1"), 3}
											  }}
										<< QJsonValue{QJsonObject{
												  {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												  {QStringLiteral("key"), 1},
												  {QStringLiteral("value"), 2},
												  {QStringLiteral("extra1"), 3}
											  }};
	QTest::newRow("poly.forced.nonstatic") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
										   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant::fromValue<TestObject*>(new StaticNonPolyObject{10, 0.1, 11, true, this})
										   << QCborValue{QCborMap{
													 {QStringLiteral("@class"), QStringLiteral("StaticNonPolyObject")},
													 {QStringLiteral("key"), 1},
													 {QStringLiteral("value"), 2},
													 {QStringLiteral("extra2"), 3}
												 }}
										   << QJsonValue{QJsonObject{
													 {QStringLiteral("@class"), QStringLiteral("StaticNonPolyObject")},
													 {QStringLiteral("key"), 1},
													 {QStringLiteral("value"), 2},
													 {QStringLiteral("extra2"), 3}
												 }};
	QTest::newRow("poly.forced.dynamic.enabled") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
												 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
												 << static_cast<QObject*>(nullptr)
												 << qMetaTypeId<TestObject*>()
												 << QVariant::fromValue<TestObject*>(new DynamicPolyObject{10, 0.1, 11, true, true, this})
												 << QCborValue{QCborMap{
														   {QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
														   {QStringLiteral("key"), 1},
														   {QStringLiteral("value"), 2},
														   {QStringLiteral("extra3"), 3}
													   }}
												 << QJsonValue{QJsonObject{
														   {QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
														   {QStringLiteral("key"), 1},
														   {QStringLiteral("value"), 2},
														   {QStringLiteral("extra3"), 3}
													   }};
	QTest::newRow("poly.forced.dynamic.disabled") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
												  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
												  << static_cast<QObject*>(nullptr)
												  << qMetaTypeId<TestObject*>()
												  << QVariant::fromValue<TestObject*>(new DynamicPolyObject{10, 0.1, 11, true, false, this})
												  << QCborValue{QCborMap{
															{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
															{QStringLiteral("key"), 1},
															{QStringLiteral("value"), 2},
															{QStringLiteral("extra3"), 3}
														}}
												  << QJsonValue{QJsonObject{
															{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
															{QStringLiteral("key"), 1},
															{QStringLiteral("value"), 2},
															{QStringLiteral("extra3"), 3}
														}};
	QTest::newRow("poly.forced.derived") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
										 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
										 << static_cast<QObject*>(nullptr)
										 << qMetaTypeId<TestObject*>()
										 << QVariant::fromValue<TestObject*>(new DerivedTestObject{10, 0.1, 11, true, this})
										 << QCborValue{QCborMap{
												   {QStringLiteral("@class"), QStringLiteral("DerivedTestObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra4"), 3}
											   }}
										 << QJsonValue{QJsonObject{
												   {QStringLiteral("@class"), QStringLiteral("DerivedTestObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra4"), 3}
											   }};
}

void ObjectConverterTest::addSerData()
{
	QTest::newRow("poly.disabled.static") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Disabled)}}
										  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
										  << static_cast<QObject*>(nullptr)
										  << qMetaTypeId<TestObject*>()
										  << QVariant::fromValue(new StaticPolyObject{10, 0.1, 11, true, this})
										  << QCborValue{QCborMap{
												 {QStringLiteral("key"), 1},
												 {QStringLiteral("value"), 2}
											 }}
										  << QJsonValue{QJsonObject{
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2}
											   }};
	QTest::newRow("poly.disabled.dynamic") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Disabled)}}
										   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant::fromValue(new DynamicPolyObject{10, 0.1, 11, true, true, this})
										   << QCborValue{QCborMap{
												  {QStringLiteral("key"), 1},
												  {QStringLiteral("value"), 2}
											  }}
										   << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2}
												}};

	QTest::newRow("poly.enabled.nonstatic") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Enabled)}}
											<< TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}}
											<< static_cast<QObject*>(nullptr)
											<< qMetaTypeId<TestObject*>()
											<< QVariant::fromValue<TestObject*>(new StaticNonPolyObject{10, 0.1, 11, true, this})
											<< QCborValue{QCborMap{
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2}
											   }}
											<< QJsonValue{QJsonObject{
													  {QStringLiteral("key"), 1},
													  {QStringLiteral("value"), 2}
												  }};
	QTest::newRow("poly.enabled.dynamic.disabled") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Enabled)}}
												   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
												   << static_cast<QObject*>(nullptr)
												   << qMetaTypeId<TestObject*>()
												   << QVariant::fromValue<TestObject*>(new DynamicPolyObject{10, 0.1, 11, true, false, this})
												   << QCborValue{QCborMap{
														  {QStringLiteral("key"), 1},
														  {QStringLiteral("value"), 2}
													  }}
												   << QJsonValue{QJsonObject{
															 {QStringLiteral("key"), 1},
															 {QStringLiteral("value"), 2}
														 }};
	QTest::newRow("poly.enabled.derived") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Enabled)}}
										  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 0.1, 2}, {QMetaType::Bool, true, 3}}
										  << static_cast<QObject*>(nullptr)
										  << qMetaTypeId<TestObject*>()
										  << QVariant::fromValue<TestObject*>(new DerivedTestObject{10, 0.1, 11, true, this})
										  << QCborValue{QCborMap{
												 {QStringLiteral("key"), 1},
												 {QStringLiteral("value"), 2}
											 }}
										  << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2}
												}};

	QTest::newRow("stored.ignore") << QVariantHash{{QStringLiteral("ignoreStoredAttribute"), true}}
								   << TestQ{
										{QMetaType::Int, 10, 1},
										{QMetaType::Double, 0.1, 2},
										{QMetaType::Int, 42, 3}
								   }
								   << static_cast<QObject*>(nullptr)
								   << qMetaTypeId<TestObject*>()
								   << QVariant::fromValue(new TestObject{10, 0.1, 42, this})
								   << QCborValue{QCborMap{
										  {QStringLiteral("key"), 1},
										  {QStringLiteral("value"), 2},
										  {QStringLiteral("zhidden"), 3}
									  }}
								   << QJsonValue{QJsonObject{
											{QStringLiteral("key"), 1},
											{QStringLiteral("value"), 2},
											{QStringLiteral("zhidden"), 3}
										}};
}

void ObjectConverterTest::addDeserData()
{
	QTest::newRow("broken") << QVariantHash{}
							<< TestQ{}
							<< static_cast<QObject*>(nullptr)
							<< qMetaTypeId<BrokenObject*>()
							<< QVariant{}
							<< QCborValue{QCborMap{}}
							<< QJsonValue{QJsonObject{}};

	QTest::newRow("poly.disabled.static") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Disabled)}}
										  << TestQ{
												   {QMetaType::Int, 10, 1},
												   {QMetaType::Double, 0.1, 2},
												   {QMetaType::UnknownType, {}, QStringLiteral("StaticPolyObject")},
												   {QMetaType::UnknownType, true, 3}
											   }
										  << static_cast<QObject*>(nullptr)
										  << qMetaTypeId<TestObject*>()
										  << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
										  << QCborValue{QCborMap{
												   {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra1"), 3}
											   }}
										  << QJsonValue{QJsonObject{
												   {QStringLiteral("@class"), QStringLiteral("StaticPolyObject")},
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("value"), 2},
												   {QStringLiteral("extra1"), 3}
											   }};
	QTest::newRow("poly.disabled.dynamic") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Disabled)}}
										   << TestQ{
													{QMetaType::Int, 10, 1},
													{QMetaType::Double, 0.1, 2},
													{QMetaType::UnknownType, {}, QStringLiteral("DynamicPolyObject")},
													{QMetaType::UnknownType, true, 3}
												}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
										   << QCborValue{QCborMap{
													{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2},
													{QStringLiteral("extra3"), 3}
												}}
										   << QJsonValue{QJsonObject{
													{QStringLiteral("@class"), QStringLiteral("DynamicPolyObject")},
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2},
													{QStringLiteral("extra3"), 3}
												}};

	QTest::newRow("poly.forced.invalid") << QVariantHash{{QStringLiteral("polymorphing"), QVariant::fromValue(QJsonSerializer::Polymorphing::Forced)}}
										 << TestQ{}
										 << static_cast<QObject*>(nullptr)
										 << qMetaTypeId<TestObject*>()
										 << QVariant{}
										 << QCborValue{QCborMap{
												  {QStringLiteral("key"), 1},
												  {QStringLiteral("value"), 2}
											  }}
										 << QJsonValue{QJsonObject{
												  {QStringLiteral("key"), 1},
												  {QStringLiteral("value"), 2}
											  }};

	QTest::newRow("validate.none") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}}
								   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::UnknownType, 24, 24}}
								   << static_cast<QObject*>(nullptr)
								   << qMetaTypeId<TestObject*>()
								   << QVariant::fromValue(new TestObject{10, 0, 11, this})
								   << QCborValue{QCborMap{
											{QStringLiteral("key"), 1},
											{QStringLiteral("extra"), 24}
										}}
								   << QJsonValue{QJsonObject{
											{QStringLiteral("key"), 1},
											{QStringLiteral("extra"), 24}
										}};
	QTest::newRow("validate.extra.invalid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::NoExtraProperties)}}
											<< TestQ{{QMetaType::Int, 10, 1}, {QMetaType::UnknownType, 24, 24}}
											<< static_cast<QObject*>(nullptr)
											<< qMetaTypeId<TestObject*>()
											<< QVariant{}
											<< QCborValue{QCborMap{
												   {QStringLiteral("key"), 1},
												   {QStringLiteral("extra"), 24}
											   }}
											<< QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("extra"), 24}
												}};
	QTest::newRow("validate.extra.valid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::NoExtraProperties)}}
										  << TestQ{{QMetaType::Int, 10, 1}}
										  << static_cast<QObject*>(nullptr)
										  << qMetaTypeId<TestObject*>()
										  << QVariant::fromValue(new TestObject{10, 0, 11, this})
										  << QCborValue{QCborMap{
												 {QStringLiteral("key"), 1},
											 }}
										  << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1}
												}};
	QTest::newRow("validate.all.invalid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::AllProperties)}}
										  << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::UnknownType, 24, 24}}
										  << static_cast<QObject*>(nullptr)
										  << qMetaTypeId<TestObject*>()
										  << QVariant{}
										  << QCborValue{QCborMap{
												 {QStringLiteral("key"), 1},
												 {QStringLiteral("extra"), 24}
											 }}
										  << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("extra"), 24}
												}};
	QTest::newRow("validate.all.valid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::AllProperties)}}
										<< TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 10.1, 2}, {QMetaType::UnknownType, 24, 24}}
										<< static_cast<QObject*>(nullptr)
										<< qMetaTypeId<TestObject*>()
										<< QVariant::fromValue(new TestObject{10, 10.1, 11, this})
										<< QCborValue{QCborMap{
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2},
												{QStringLiteral("extra"), 24}
											}}
										<< QJsonValue{QJsonObject{
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2},
												{QStringLiteral("extra"), 24}
											}};
	QTest::newRow("validate.all.name.invalid") << QVariantHash{
														{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::AllProperties)},
														{QStringLiteral("keepObjectName"), true}
													}
											   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 10.1, 2}, {QMetaType::UnknownType, 24, 24}}
											   << static_cast<QObject*>(nullptr)
											   << qMetaTypeId<TestObject*>()
											   << QVariant{}
											   << QCborValue{QCborMap{
													  {QStringLiteral("key"), 1},
													  {QStringLiteral("value"), 2},
													  {QStringLiteral("extra"), 24}
												  }}
											   << QJsonValue{QJsonObject{
														 {QStringLiteral("key"), 1},
														 {QStringLiteral("value"), 2},
														 {QStringLiteral("extra"), 24}
													 }};
	QTest::newRow("validate.all.name.valid") << QVariantHash{
													{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::AllProperties)},
													{QStringLiteral("keepObjectName"), true}
												}
											 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 10.1, 2}, {QMetaType::QString, QStringLiteral("TestObject"), 3}, {QMetaType::UnknownType, 24, 24}}
											 << static_cast<QObject*>(nullptr)
											 << qMetaTypeId<TestObject*>()
											 << QVariant::fromValue(new TestObject{10, 10.1, 11, this})
											 << QCborValue{QCborMap{
													{QStringLiteral("objectName"), 3},
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2},
													{QStringLiteral("extra"), 24}
												}}
											 << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("value"), 2},
													{QStringLiteral("objectName"), 3},
													{QStringLiteral("extra"), 24}
												}};
	QTest::newRow("validate.full.invalid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::FullPropertyValidation)}}
										   << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::UnknownType, 24, 24}}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant{}
										   << QCborValue{QCborMap{
													{QStringLiteral("key"), 1},
													{QStringLiteral("extra"), 24}
												}}
										   << QJsonValue{QJsonObject{
													{QStringLiteral("key"), 1},
													{QStringLiteral("extra"), 24}
												}};
	QTest::newRow("validate.full.valid") << QVariantHash{{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::FullPropertyValidation)}}
										 << TestQ{{QMetaType::Int, 10, 1}, {QMetaType::Double, 10.1, 2}}
										 << static_cast<QObject*>(nullptr)
										 << qMetaTypeId<TestObject*>()
										 << QVariant::fromValue(new TestObject{10, 10.1, 11, this})
										 << QCborValue{QCborMap{
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2}
											}}
										 << QJsonValue{QJsonObject{
												{QStringLiteral("key"), 1},
												{QStringLiteral("value"), 2}
											}};

	QTest::newRow("generic.base") << QVariantHash{}
								  << TestQ{
										 {QMetaType::UnknownType, 10, 1},
										 {QMetaType::UnknownType, 0.1, 2},
										 {QMetaType::UnknownType, 42, 3}
									 }
								  << static_cast<QObject*>(nullptr)
								  << qMetaTypeId<TestObject*>()
								  << QVariant::fromValue(new TestObject{10, 0.1, 42, this})
								  << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
												QCborArray{
													QStringLiteral("TestObject"),
													1, 2, 3
												}}
								  << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.default") << QVariantHash{}
									 << TestQ{
											{QMetaType::UnknownType, 10, 1},
											{QMetaType::UnknownType, 0.1, 2}
										}
									 << static_cast<QObject*>(nullptr)
									 << qMetaTypeId<TestObject*>()
									 << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
									 << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
												   QCborArray{
													   QStringLiteral("TestObject"),
													   1, 2
												   }}
									 << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.untyped") << QVariantHash{}
									 << TestQ{
											{QMetaType::UnknownType, 10, 1},
											{QMetaType::UnknownType, 0.1, 2},
											{QMetaType::UnknownType, 42, 3}
										}
									 << static_cast<QObject*>(nullptr)
									 << static_cast<int>(QMetaType::QObjectStar)
									 << QVariant::fromValue<QObject*>(new TestObject{10, 0.1, 42, this})
									 << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
												   QCborArray{
													   QStringLiteral("TestObject"),
													   1, 2, 3
												   }}
									 << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.invalid.empty") << QVariantHash{}
										   << TestQ{}
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant{}
										   << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
														 QCborArray{}}
										   << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.invalid.count") << QVariantHash{}
										   << TestQ{
												  {QMetaType::UnknownType, 10, 1},
												  {QMetaType::UnknownType, 0.1, 2},
												  {QMetaType::UnknownType, 10, 3},
												  {QMetaType::UnknownType, 10, 4},
												  {QMetaType::UnknownType, 0.1, 5}
											  }
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant{}
										   << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
														 QCborArray{
															 QStringLiteral("TestObject"),
															 1, 2, 3, 4, 5
														 }}
										   << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.invalid.type") << QVariantHash{}
										   << TestQ{
												  {QMetaType::UnknownType, 10, 1},
												  {QMetaType::UnknownType, QVariantList{}, 2},
												  {QMetaType::UnknownType, 10, 3},
											  }
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant{}
										   << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
														 QCborArray{
															 QStringLiteral("TestObject"),
															 1, 2, 3
														 }}
										   << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("generic.invalid.class") << QVariantHash{}
										   << TestQ{
												  {QMetaType::UnknownType, 10, 1},
												  {QMetaType::UnknownType, 0.1, 2},
												  {QMetaType::UnknownType, 11, 3},
											  }
										   << static_cast<QObject*>(nullptr)
										   << qMetaTypeId<TestObject*>()
										   << QVariant{}
										   << QCborValue{static_cast<QCborTag>(QCborSerializer::GenericObject),
														 QCborArray{
															 QStringLiteral("OpaqueDummyGadget"),
															 1, 2, 3
														 }}
										   << QJsonValue{QJsonValue::Undefined};

	QTest::newRow("constructed.base") << QVariantHash{}
									  << TestQ{
											 {QMetaType::UnknownType, 10, 1},
											 {QMetaType::UnknownType, 0.1, 2},
											 {QMetaType::Int, 42, 3}
										 }
									  << static_cast<QObject*>(nullptr)
									  << qMetaTypeId<TestObject*>()
									  << QVariant::fromValue(new TestObject{10, 0.1, 42, this})
									  << QCborValue{static_cast<QCborTag>(QCborSerializer::ConstructedObject),
													QCborArray{
														QCborArray{
															QStringLiteral("TestObject"),
															1, 2
														},
														QCborMap {
															{QStringLiteral("zhidden"), 3}
														}
													}}
									  << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("constructed.null1") << QVariantHash{}
									   << TestQ{}
									   << static_cast<QObject*>(nullptr)
									   << qMetaTypeId<TestObject*>()
									   << QVariant::fromValue<TestObject*>(nullptr)
									   << QCborValue{static_cast<QCborTag>(QCborSerializer::ConstructedObject), QCborValue::Null}
									   << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("constructed.null1") << QVariantHash{}
									   << TestQ{
											  {QMetaType::UnknownType, 10, 1},
											  {QMetaType::UnknownType, 0.1, 2}
										  }
									   << static_cast<QObject*>(nullptr)
									   << qMetaTypeId<TestObject*>()
									   << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
									   << QCborValue{static_cast<QCborTag>(QCborSerializer::ConstructedObject),
													 QCborArray{
														 QCborArray{
															 QStringLiteral("TestObject"),
															 1, 2
														 },
														 QCborValue::Null
													 }}
									   << QJsonValue{QJsonValue::Undefined};
	QTest::newRow("constructed.invalid") << QVariantHash{}
										 << TestQ{
												{QMetaType::UnknownType, 10, 1},
												{QMetaType::UnknownType, 0.1, 2}
											}
										 << static_cast<QObject*>(nullptr)
										 << qMetaTypeId<TestObject*>()
										 << QVariant::fromValue(new TestObject{10, 0.1, 11, this})
										 << QCborValue{static_cast<QCborTag>(QCborSerializer::ConstructedObject),
													   QCborArray{
														   QCborArray{
															   QStringLiteral("TestObject"),
															   1, 2
														   },
														   3
													   }}
										 << QJsonValue{QJsonValue::Undefined};
}

bool ObjectConverterTest::compare(int type, QVariant &actual, QVariant &expected, const char *aName, const char *eName, const char *file, int line)
{
	const auto flags = QMetaType::typeFlags(type);
	if(flags.testFlag(QMetaType::PointerToQObject) ||
	   flags.testFlag(QMetaType::TrackingPointerToQObject) ||
	   flags.testFlag(QMetaType::SharedPointerToQObject)) {
		const auto obj1 = actual.value<TestObject*>();
		const auto obj2 = expected.value<TestObject*>();
		if(obj1 != obj2) { //same object is automatically equal
			if(!QTest::qVerify(obj1, aName, "object pointer verification", file, line))
				return false;
			if(!QTest::qVerify(obj2, eName, "object pointer verification", file, line))
				return false;
			QByteArray statement {eName + QByteArrayLiteral("->compare(") + aName + QByteArrayLiteral(")")};
			return QTest::qVerify(obj2->compare(obj1), statement.constData(), "equality comparison", file, line);
		} else
			return true;
	} else
		return TypeConverterTestBase::compare(type, actual, expected, aName, eName, file, line);
}

QTEST_MAIN(ObjectConverterTest)

#include "tst_objectconverter.moc"
