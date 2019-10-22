#include <QtTest>
#include <QtJsonSerializer>
#include <QColor>
#include <QFont>
#include <QJsonDocument>

#include "testconverter.h"

using TestTuple = std::tuple<int, QString, QList<int>>;
using TestPair = std::pair<bool, int>;
using TestVariant = std::variant<bool, int, double>;
Q_DECLARE_METATYPE(TestTuple)
Q_DECLARE_METATYPE(TestPair)
Q_DECLARE_METATYPE(std::optional<int>)
Q_DECLARE_METATYPE(TestVariant)

class AliasHelper : public QJsonSerializerBase
{
public:
	QByteArray getNameHelper(int propertyType) const {
		return getCanonicalTypeName(propertyType);
	}

	std::variant<QCborValue, QJsonValue> serializeGeneric(const QVariant &value) const override {
		return QCborValue::fromVariant(value);
	}

	QVariant deserializeGeneric(const std::variant<QCborValue, QJsonValue> &value, int, QObject *) const override {
		return std::get<QCborValue>(value).toVariant();
	}

protected:
	bool jsonMode() const override {
		return true;
	}
	QCborTag typeTag(int) const override {
		return static_cast<QCborTag>(-1);
	}
	QList<int> typesForTag(QCborTag) const override {
		return {};
	}
};

class SerializerTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testAliasName();
	void testVariantConversions_data();
	void testVariantConversions();

	void testSerialization_data();
	void testSerialization();
	void testDeserialization_data();
	void testDeserialization();

	void testDeviceSerialization();
	void testExceptionTrace();

private:
	AliasHelper *helper;
	QJsonSerializer *jsonSerializer = nullptr;
	QCborSerializer *cborSerializer = nullptr;

	void addCommonData();
	void resetProps();
};

void SerializerTest::initTestCase()
{
	//aliases
	qRegisterMetaType<IntAlias>("IntAlias");
	qRegisterMetaType<ListAlias>();
	QJsonSerializer::registerInverseTypedef<ListAlias>("QList<TestObject*>");

	// converters
	QJsonSerializer::registerPointerConverters<TestObject>();
	QJsonSerializer::registerListConverters<TestObject*>();
	QJsonSerializer::registerListConverters<QList<int>>();
	QJsonSerializer::registerMapConverters<TestObject*>();
	QJsonSerializer::registerMapConverters<QMap<QString, int>>();
	QJsonSerializer::registerPairConverters<int, QString>();
	QJsonSerializer::registerPairConverters<bool, bool>();
	QJsonSerializer::registerPairConverters<QList<bool>, bool>();
	QJsonSerializer::registerListConverters<QPair<bool, bool>>();

	QJsonSerializer_registerTupleConverters_named(int, QString, QList<int>);
	QJsonSerializer_registerStdPairConverters_named(bool, int);
	QJsonSerializer::registerOptionalConverters<int>();
	QJsonSerializer_registerVariantConverters_named(bool, int, double);

	//register list comparators, needed for test only!
	QMetaType::registerEqualsComparator<QList<bool>>();
	QMetaType::registerEqualsComparator<QList<int>>();
	QMetaType::registerEqualsComparator<QList<TestObject*>>();
	QMetaType::registerEqualsComparator<QList<QList<int>>>();
	QMetaType::registerEqualsComparator<QMap<QString, int>>();
	QMetaType::registerEqualsComparator<QMap<QString, TestObject*>>();
	QMetaType::registerEqualsComparator<QMap<QString, QMap<QString, int>>>();
	QMetaType::registerEqualsComparator<QPair<QVariant, QVariant>>();
	QMetaType::registerEqualsComparator<QPair<int, QString>>();
	QMetaType::registerEqualsComparator<QPair<QList<bool>, bool>>();
	QMetaType::registerEqualsComparator<QList<QPair<bool, bool>>>();

	QMetaType::registerEqualsComparator<TestTuple>();
	QMetaType::registerEqualsComparator<TestPair>();
	QMetaType::registerEqualsComparator<std::optional<int>>();
	QMetaType::registerEqualsComparator<std::variant<bool, int, double>>();

	helper = new AliasHelper{};
	jsonSerializer = new QJsonSerializer{this};
	jsonSerializer->addJsonTypeConverter<TestEnumConverter>();
	jsonSerializer->addJsonTypeConverter<TestWrapperConverter>();
	cborSerializer = new QCborSerializer{this};
	cborSerializer->addJsonTypeConverter<TestEnumConverter>();
	cborSerializer->addJsonTypeConverter<TestWrapperConverter>();
}

void SerializerTest::cleanupTestCase()
{
	delete jsonSerializer;
	jsonSerializer = nullptr;
	delete jsonSerializer;
	jsonSerializer = nullptr;
}

void SerializerTest::testAliasName()
{
	QCOMPARE(QMetaType::typeName(qMetaTypeId<ListAlias>()), "ListAlias");
	QCOMPARE(helper->getNameHelper(qMetaTypeId<ListAlias>()), "QList<TestObject*>");
}

void SerializerTest::testVariantConversions_data()
{
	QTest::addColumn<QVariant>("data");
	QTest::addColumn<int>("targetType");
	QTest::addColumn<QVariant>("variantData");
	QTest::addColumn<bool>("iterable");

	QTest::newRow("QList<int>") << QVariant::fromValue<QList<int>>({3, 7, 13})
								<< static_cast<int>(QMetaType::QVariantList)
								<< QVariant{QVariantList{3, 7, 13}}
								<< true;
	auto to = new TestObject{this};
	QTest::newRow("QList<TestObject*>") << QVariant::fromValue<QList<TestObject*>>({nullptr, to, nullptr})
										<< static_cast<int>(QMetaType::QVariantList)
										<< QVariant{QVariantList{
											   QVariant::fromValue<TestObject*>(nullptr),
											   QVariant::fromValue(to),
											   QVariant::fromValue<TestObject*>(nullptr)
										   }}
										<< true;
	QList<int> l1 = {0, 1, 2};
	QList<int> l2 = {3, 4, 5};
	QList<int> l3 = {6, 7, 8};
	QTest::newRow("QList<QList<int>>") << QVariant::fromValue<QList<QList<int>>>({l1, l2, l3})
									   << static_cast<int>(QMetaType::QVariantList)
									   << QVariant{QVariantList{QVariant::fromValue(l1), QVariant::fromValue(l2), QVariant::fromValue(l3)}}
									   << true;
	QTest::newRow("QStringList") << QVariant::fromValue<QStringList>({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")})
								 << static_cast<int>(QMetaType::QVariantList)
								 << QVariant{QVariantList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}}
								 << true;

	QTest::newRow("QMap<QString, int>") << QVariant::fromValue<QMap<QString, int>>({
																					   {QStringLiteral("baum"), 42},
																					   {QStringLiteral("devil"), 666},
																					   {QStringLiteral("fun"), 0}
																				   })
										<< static_cast<int>(QMetaType::QVariantMap)
										<< QVariant{QVariantMap {
												{QStringLiteral("baum"), 42},
												{QStringLiteral("devil"), 666},
												{QStringLiteral("fun"), 0}
											}}
										<< false;
	QTest::newRow("QMap<QString, TestObject*>") << QVariant::fromValue<QMap<QString, TestObject*>>({
													   {QStringLiteral("baum"), nullptr},
													   {QStringLiteral("devil"), to},
													   {QStringLiteral("fun"), nullptr}
												   })
												<< static_cast<int>(QMetaType::QVariantMap)
												<< QVariant{QVariantMap {
													   {QStringLiteral("baum"), QVariant::fromValue<TestObject*>(nullptr)},
													   {QStringLiteral("devil"), QVariant::fromValue(to)},
													   {QStringLiteral("fun"), QVariant::fromValue<TestObject*>(nullptr)}
												   }}
												<< false;
	QMap<QString, int> m1 = {{QStringLiteral("v0"), 0}, {QStringLiteral("v1"), 1}, {QStringLiteral("v2"), 2}};
	QMap<QString, int> m2 = {{QStringLiteral("v3"), 3}, {QStringLiteral("v4"), 4}, {QStringLiteral("v5"), 5}};
	QMap<QString, int> m3 = {{QStringLiteral("v6"), 6}, {QStringLiteral("v7"), 7}, {QStringLiteral("v8"), 8}};
	QTest::newRow("QMap<QString, QMap<QString, int>>") << QVariant::fromValue<QMap<QString, QMap<QString, int>>>({
																													 {QStringLiteral("m1"), m1},
																													 {QStringLiteral("m2"), m2},
																													 {QStringLiteral("m3"), m3}
																												 })
													   << static_cast<int>(QMetaType::QVariantMap)
													   << QVariant{QVariantMap{
															  {QStringLiteral("m1"), QVariant::fromValue(m1)},
															  {QStringLiteral("m2"), QVariant::fromValue(m2)},
															  {QStringLiteral("m3"), QVariant::fromValue(m3)}
														  }}
													   << false;

	QSharedPointer<TestObject> sPtr(new TestObject{});
	QTest::newRow("QSharedPointer<TestObject>") << QVariant::fromValue(sPtr)
												<< qMetaTypeId<QSharedPointer<QObject>>()
												<< QVariant::fromValue(sPtr.staticCast<QObject>())
												<< false;
	QPointer<TestObject> oPtr(new TestObject{this});
	QTest::newRow("QPointer<TestObject>") << QVariant::fromValue(oPtr)
										  << qMetaTypeId<QPointer<QObject>>()
										  << QVariant::fromValue<QPointer<QObject>>(oPtr.data())
										  << false;

	QTest::newRow("QPair<int, QString>") << QVariant::fromValue<QPair<int, QString>>({42, QStringLiteral("baum")})
										 << qMetaTypeId<QPair<QVariant, QVariant>>()
										 << QVariant::fromValue(QPair<QVariant, QVariant>{42, QStringLiteral("baum")})
										 << false;

	QTest::newRow("QPair<QList<bool>, bool>") << QVariant::fromValue<QPair<QList<bool>, bool>>({{true, false, true}, false})
											  << qMetaTypeId<QPair<QVariant, QVariant>>()
											  << QVariant::fromValue(QPair<QVariant, QVariant>{
													 QVariant::fromValue(QList<bool>{true, false, true}),
													 false
												 })
											  << false;

	QTest::newRow("QList<QPair<bool, bool>>") << QVariant::fromValue<QList<QPair<bool, bool>>>({{false, true}, {true, false}})
											  << static_cast<int>(QMetaType::QVariantList)
											  << QVariant{QVariantList{
													 QVariant::fromValue(QPair<bool, bool>{false, true}),
													 QVariant::fromValue(QPair<bool, bool>{true, false})
												 }}
											  << true;

	QTest::newRow("std::tuple<int, QString, QList<int>>") << QVariant::fromValue<TestTuple>(std::make_tuple(42, QStringLiteral("Hello World"), QList<int>{1, 2, 3}))
														  << static_cast<int>(QMetaType::QVariantList)
														  << QVariant{QVariantList{42, QStringLiteral("Hello World"), QVariant::fromValue(QList<int>{1, 2, 3})}}
														  << false;
	QTest::newRow("std::pair<bool, int>") << QVariant::fromValue<TestPair>(std::make_pair(true, 20))
										  << qMetaTypeId<QPair<QVariant, QVariant>>()
										  << QVariant::fromValue(QPair<QVariant, QVariant>{true, 20})
										  << false;

	QVariant optv{42};
	QTest::newRow("std::optional<int>") << QVariant::fromValue<std::optional<int>>(42)
										<< static_cast<int>(QMetaType::QVariant)
										<< QVariant{QMetaType::QVariant, &optv}
										<< false;
	optv = QVariant::fromValue(nullptr);
	QTest::newRow("std::nullopt") << QVariant::fromValue<std::optional<int>>(std::nullopt)
								  << static_cast<int>(QMetaType::QVariant)
								  << QVariant{QMetaType::QVariant, &optv}
								  << false;

	QVariant varv1{true};
	QTest::newRow("std::variant<bool>") << QVariant::fromValue<TestVariant>(true)
										<< static_cast<int>(QMetaType::QVariant)
										<< QVariant{QMetaType::QVariant, &varv1}
										<< false;
	QVariant varv2{42};
	QTest::newRow("std::variant<int>") << QVariant::fromValue<TestVariant>(42)
									   << static_cast<int>(QMetaType::QVariant)
									   << QVariant{QMetaType::QVariant, &varv2}
									   << false;
	QVariant varv3{4.2};
	QTest::newRow("std::variant<double>") << QVariant::fromValue<TestVariant>(4.2)
										  << qMetaTypeId<QVariant>()
										  << QVariant{QMetaType::QVariant, &varv3}
										  << false;
}

void SerializerTest::testVariantConversions()
{
	QFETCH(QVariant, data);
	QFETCH(int, targetType);
	QFETCH(QVariant, variantData);
	QFETCH(bool, iterable);

	auto origType = data.userType();
	auto convData = data;
	if (iterable) {
		QVERIFY(convData.canConvert(targetType));
		if (targetType == QMetaType::QVariantList) {
			const auto variantList = variantData.toList();
			auto seqIt = convData.value<QSequentialIterable>();
			QCOMPARE(seqIt.size(), variantList.size());
			auto i = 0;
			for (const auto &itData : seqIt)
				QCOMPARE(itData, variantList[i++]);

			QVariant res{data.userType(), nullptr};
			auto writer = QSequentialWriter::getWriter(res);
			QVERIFY(writer.isValid());
			writer.reserve(variantList.size());
			for (const auto &vData : variantList)
				writer.add(vData);
			QCOMPARE(res, data);
		}
		convData = variantData;
	} else {
		QVERIFY(convData.convert(targetType));
		QCOMPARE(convData, variantData);
		QVERIFY(convData.convert(origType));
		QCOMPARE(convData, data);
	}
}

void SerializerTest::testSerialization_data()
{
	QTest::addColumn<QVariant>("data");
	QTest::addColumn<QCborValue>("cResult");
	QTest::addColumn<QJsonValue>("jResult");
	QTest::addColumn<bool>("works");
	QTest::addColumn<QVariantHash>("extraProps");

	addCommonData();
}

void SerializerTest::testSerialization()
{
	QFETCH(QVariant, data);
	QFETCH(QCborValue, cResult);
	QFETCH(QJsonValue, jResult);
	QFETCH(bool, works);
	QFETCH(QVariantHash, extraProps);

	resetProps();
	for(auto it = extraProps.constBegin(); it != extraProps.constEnd(); it++) {
		cborSerializer->setProperty(qUtf8Printable(it.key()), it.value());
		jsonSerializer->setProperty(qUtf8Printable(it.key()), it.value());
	}

	try {
		if (!cResult.isUndefined()) {
			if(works) {
				auto res = cborSerializer->serialize(data);
				QCOMPARE(res, cResult);
			} else
				QVERIFY_EXCEPTION_THROWN(cborSerializer->serialize(data), QJsonSerializationException);
		}
		if (!jResult.isUndefined()) {
			if(works) {
				auto res = jsonSerializer->serialize(data);
				QCOMPARE(res, jResult);
			} else
				QVERIFY_EXCEPTION_THROWN(jsonSerializer->serialize(data), QJsonSerializationException);
		}
	} catch(std::exception &e) {
		QFAIL(e.what());
	}
}

void SerializerTest::testDeserialization_data()
{
	QTest::addColumn<QVariant>("result");
	QTest::addColumn<QCborValue>("cData");
	QTest::addColumn<QJsonValue>("jData");
	QTest::addColumn<bool>("works");
	QTest::addColumn<QVariantHash>("extraProps");

	addCommonData();

	QTest::newRow("null.invalid.bool") << QVariant{false}
									   << QCborValue{QCborValue::Null}
									   << QJsonValue{QJsonValue::Null}
									   << false
									   << QVariantHash {
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.invalid.int") << QVariant{0}
									  << QCborValue{QCborValue::Null}
									  << QJsonValue{QJsonValue::Null}
									  << false
									  << QVariantHash {
											 {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										 };
	QTest::newRow("null.invalid.double") << QVariant{0.0}
										 << QCborValue{QCborValue::Null}
										 << QJsonValue{QJsonValue::Null}
										 << false
										 << QVariantHash {
												{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
											};
	QTest::newRow("null.invalid.string") << QVariant{QString()}
										 << QCborValue{QCborValue::Null}
										 << QJsonValue{QJsonValue::Null}
										 << false
										 << QVariantHash {
												{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
											};
	QTest::newRow("null.invalid.uuid") << QVariant{QUuid()}
									   << QCborValue{QCborValue::Null}
									   << QJsonValue{QJsonValue::Null}
									   << false
									   << QVariantHash {
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.invalid.url") << QVariant{QUrl()}
									  << QCborValue{QCborValue::Null}
									  << QJsonValue{QJsonValue::Null}
									  << false
									  << QVariantHash {
											 {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										 };
//	QTest::newRow("null.invalid.regex") << QVariant{QRegularExpression()}
//										<< QCborValue{QCborValue::Null}
//										<< QJsonValue{QJsonValue::Null}
//										<< false
//										<< QVariantHash {
//											   {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
//										   };

	QTest::newRow("null.valid.bool") << QVariant{false}
									 << QCborValue{QCborValue::Null}
									 << QJsonValue{QJsonValue::Null}
									 << true
									 << QVariantHash {
											{QStringLiteral("allowDefaultNull"), true},
											{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										};
	QTest::newRow("null.valid.int") << QVariant{0}
									<< QCborValue{QCborValue::Null}
									<< QJsonValue{QJsonValue::Null}
									<< true
									<< QVariantHash {
										   {QStringLiteral("allowDefaultNull"), true},
										   {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
									   };
	QTest::newRow("null.valid.double") << QVariant{0.0}
									   << QCborValue{QCborValue::Null}
									   << QJsonValue{QJsonValue::Null}
									   << true
									   << QVariantHash {
											  {QStringLiteral("allowDefaultNull"), true},
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.valid.string") << QVariant{QString()}
									   << QCborValue{QCborValue::Null}
									   << QJsonValue{QJsonValue::Null}
									   << true
									   << QVariantHash {
											  {QStringLiteral("allowDefaultNull"), true},
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.valid.uuid") << QVariant{QUuid()}
									 << QCborValue{QCborValue::Null}
									 << QJsonValue{QJsonValue::Null}
									 << true
									 << QVariantHash {
											{QStringLiteral("allowDefaultNull"), true},
											{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										};
	QTest::newRow("null.valid.url") << QVariant{QUrl()}
									<< QCborValue{QCborValue::Null}
									<< QJsonValue{QJsonValue::Null}
									<< true
									<< QVariantHash {
										   {QStringLiteral("allowDefaultNull"), true},
										   {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
									   };
//	QTest::newRow("null.valid.regex") << QVariant{QRegularExpression()}
//									  << QCborValue{}
//									  << QJsonValue{QJsonValue::Null}
//									  << true
//									  << QVariantHash {
//											 {QStringLiteral("allowDefaultNull"), true},
//											 {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
//										 };

	QTest::newRow("strict.bool.invalid") << QVariant{false}
										 << QCborValue{0}
										 << QJsonValue{0}
										 << false
										 << QVariantHash{};
	QTest::newRow("strict.int.double") << QVariant{4}
									   << QCborValue{4.2}
									   << QJsonValue{4.2}
									   << false
									   << QVariantHash{};
	QTest::newRow("strict.int.type") << QVariant{2}
									 << QCborValue{QStringLiteral("2")}
									 << QJsonValue{QStringLiteral("2")}
									 << false
									 << QVariantHash{};
	QTest::newRow("strict.string") << QVariant{QStringLiteral("10")}
								   << QCborValue{10}
								   << QJsonValue{10}
								   << false
								   << QVariantHash{};
	QTest::newRow("strict.nullptr") << QVariant::fromValue(nullptr)
									<< QCborValue{QCborMap{}}
									<< QJsonValue{QJsonObject{}}
									<< false
									<< QVariantHash{};
	QTest::newRow("strict.double") << QVariant{2.2}
								   << QCborValue{QStringLiteral("2.2")}
								   << QJsonValue{QStringLiteral("2.2")}
								   << false
								   << QVariantHash{};
	QTest::newRow("strict.url") << QVariant{QUrl{}}
								<< QCborValue{5}
								<< QJsonValue{5}
								<< false
								<< QVariantHash{};
	QTest::newRow("strict.uuid") << QVariant{QUuid{}}
								 << QCborValue{true}
								 << QJsonValue{true}
								 << false
								 << QVariantHash{};
//	QTest::newRow("strict.regex") << QVariant{QRegularExpression{}}
//								  << QCborValue{42}
//								  << QJsonValue{42}
//								  << false
//								  << QVariantHash{};
}

void SerializerTest::testDeserialization()
{
	QFETCH(QCborValue, cData);
	QFETCH(QJsonValue, jData);
	QFETCH(QVariant, result);
	QFETCH(bool, works);
	QFETCH(QVariantHash, extraProps);

	resetProps();
	for(auto it = extraProps.constBegin(); it != extraProps.constEnd(); it++) {
		cborSerializer->setProperty(qUtf8Printable(it.key()), it.value());
		jsonSerializer->setProperty(qUtf8Printable(it.key()), it.value());
	}

	try {
		if (!cData.isUndefined()) {
			if(works) {
				auto res = cborSerializer->deserialize(cData, result.userType(), this);
				QCOMPARE(res, result);
			} else
				QVERIFY_EXCEPTION_THROWN(cborSerializer->deserialize(cData, result.userType(), this), QJsonDeserializationException);
		}
		if (!jData.isUndefined()) {
			if(works) {
				auto res = jsonSerializer->deserialize(jData, result.userType(), this);
				QCOMPARE(res, result);
			} else
				QVERIFY_EXCEPTION_THROWN(jsonSerializer->deserialize(jData, result.userType(), this), QJsonDeserializationException);
		}
	} catch(std::exception &e) {
		QFAIL(e.what());
	}
}

void SerializerTest::testDeviceSerialization()
{
	const EnumContainer data{EnumContainer::Normal1, EnumContainer::Flag1 | EnumContainer::Flag3};
	const QCborMap cMap {
		{QStringLiteral("0"), QStringLiteral("Normal1")},
		{QStringLiteral("1"), QStringLiteral("Flag1|Flag3")},
	};
	const auto cRes = QCborValue{cMap}.toCbor();
	const auto jMap = QCborValue{cMap}.toJsonValue().toObject();
	const auto jRes = QJsonDocument{jMap}.toJson(QJsonDocument::Indented);

	// to/from bytearray
	QCOMPARE(cborSerializer->serializeTo(data), cRes);
	QCOMPARE(cborSerializer->deserializeFrom<EnumContainer>(cRes), data);

	QCOMPARE(jsonSerializer->serializeTo(data), jRes);
	QCOMPARE(jsonSerializer->deserializeFrom<EnumContainer>(jRes), data);

	// to device
	QBuffer buffer;
	QVERIFY(buffer.open(QIODevice::ReadWrite));
	cborSerializer->serializeTo(&buffer, data);
	QCOMPARE(buffer.data(), cRes);
	QVERIFY(buffer.seek(0));
	QCOMPARE(cborSerializer->deserializeFrom<EnumContainer>(&buffer), data);
	buffer.close();

	QVERIFY(buffer.open(QIODevice::ReadWrite));
	jsonSerializer->serializeTo(&buffer, data);
	QCOMPARE(buffer.data(), jRes);
	QVERIFY(buffer.seek(0));
	QCOMPARE(jsonSerializer->deserializeFrom<EnumContainer>(&buffer), data);
	buffer.close();
}

void SerializerTest::testExceptionTrace()
{
	try {
		cborSerializer->deserialize<EnumContainer>(QCborMap{
			{QStringLiteral("error"), QCborValue::Null}
		});
		QFAIL("No exception thrown");
	} catch (QJsonSerializerException &e) {
		auto trace = e.propertyTrace();
		QCOMPARE(trace.size(), 1);
		QCOMPARE(trace[0].second, QByteArray{"EnumContainer"});
		QCOMPARE(trace[0].first, QByteArray{"test"});
	}
}

void SerializerTest::addCommonData()
{
	// basic types without any converter
	QTest::newRow("bool") << QVariant{true}
						  << QCborValue{true}
						  << QJsonValue{true}
						  << true
						  << QVariantHash{};
	QTest::newRow("int") << QVariant{42}
						 << QCborValue{42}
						 << QJsonValue{42}
						 << true
						 << QVariantHash{};
	QTest::newRow("double") << QVariant{4.2}
							<< QCborValue{4.2}
							<< QJsonValue{4.2}
							<< true
							<< QVariantHash{};
	QTest::newRow("string.normal") << QVariant{QStringLiteral("baum")}
								   << QCborValue{QStringLiteral("baum")}
								   << QJsonValue{QStringLiteral("baum")}
								   << true
								   << QVariantHash{};
	QTest::newRow("string.empty") << QVariant{QString()}
								  << QCborValue{QString()}
								  << QJsonValue{QString()}
								  << true
								  << QVariantHash{};
	QTest::newRow("nullptr") << QVariant::fromValue(nullptr)
							 << QCborValue{nullptr}
							 << QJsonValue{QJsonValue::Null}
							 << true
							 << QVariantHash{};

	// advanced types
	auto id = QUuid::createUuid();
	QTest::newRow("uuid") << QVariant{id}
						  << QCborValue{id}
						  << QJsonValue{id.toString(QUuid::WithoutBraces)}
						  << true
						  << QVariantHash{};
	QTest::newRow("url.valid") << QVariant{QUrl{QStringLiteral("https://example.com/test.xml?baum=42#tree")}}
							   << QCborValue{QUrl{QStringLiteral("https://example.com/test.xml?baum=42#tree")}}
							   << QJsonValue{QStringLiteral("https://example.com/test.xml?baum=42#tree")}
							   << true
							   << QVariantHash{};
	QTest::newRow("url.invalid") << QVariant{QUrl{}}
								 << QCborValue{QUrl{}}
								 << QJsonValue{QString{}}
								 << true
								 << QVariantHash{};
//	QTest::newRow("regex.valid") << QVariant{QRegularExpression{QStringLiteral(R"__(^[Hh]ello\s+world!?$)__")}}
//								 << QCborValue{QRegularExpression{QStringLiteral(R"__(^[Hh]ello\s+world!?$)__")}}
//								 << QJsonValue{QStringLiteral(R"__(^[Hh]ello\s+world!?$)__")}
//								 << true
//								 << QVariantHash{};
//	QTest::newRow("regex.invalid") << QVariant{QRegularExpression{}}
//								   << QCborValue{QRegularExpression{}}
//								   << QJsonValue{QString{}}
//								   << true
//								   << QVariantHash{};

	// gui types
	QTest::newRow("color.rgb") << QVariant::fromValue(QColor{0xAA, 0xBB, 0xCC})
							   << QCborValue{static_cast<QCborTag>(QCborSerializer::Color), QStringLiteral("#aabbcc")}
							   << QJsonValue{QStringLiteral("#aabbcc")}
							   << true
							   << QVariantHash{};
	QTest::newRow("color.argb") << QVariant::fromValue(QColor{0xAA, 0xBB, 0xCC, 0xDD})
								<< QCborValue{static_cast<QCborTag>(QCborSerializer::Color), QStringLiteral("#ddaabbcc")}
								<< QJsonValue{QStringLiteral("#ddaabbcc")}
								<< true
								<< QVariantHash{};
	// TODO use converter
//	QTest::newRow("font") << QVariant::fromValue(QFont{QStringLiteral("Liberation Sans")})
//						  << QCborValue{static_cast<QCborTag>(QCborSerializer::Font), QStringLiteral("Liberation Sans")}
//						  << QJsonValue{QStringLiteral("Liberation Sans")}
//						  << true
//						  << QVariantHash{};


	// type converters
	QTest::newRow("converter") << QVariant::fromValue(EnumContainer{EnumContainer::Normal1, EnumContainer::FlagX})
							   << QCborValue{QCborMap{
									  {QStringLiteral("0"), QStringLiteral("Normal1")},
									  {QStringLiteral("1"), QStringLiteral("FlagX")}
								  }}
							   << QJsonValue{QJsonObject{
									  {QStringLiteral("0"), QStringLiteral("Normal1")},
									  {QStringLiteral("1"), QStringLiteral("FlagX")}
								  }}
							   << true
							   << QVariantHash{};
}

void SerializerTest::resetProps()
{
	for (auto ser : {
			 static_cast<QJsonSerializerBase*>(jsonSerializer),
			 static_cast<QJsonSerializerBase*>(cborSerializer)}) {
		jsonSerializer->setAllowDefaultNull(false);
		ser->setKeepObjectName(false);
		ser->setEnumAsString(false);
		ser->setUseBcp47Locale(true);
		ser->setValidationFlags(QJsonSerializer::ValidationFlag::StrictBasicTypes);
		ser->setPolymorphing(QJsonSerializer::Polymorphing::Enabled);
	}

	jsonSerializer->setValidateBase64(true);
}

namespace  {

template<typename Type, typename Cbor, typename Json>
static void test_type() {
	Type v;
	QIODevice *d = nullptr;

	Cbor c;
	QCborSerializer cs;
	static_assert(std::is_same<Cbor, decltype(cs.serialize(v))>::value, "Wrong CBOR value returned by expression");
	cs.serializeTo(d, v);
	cs.serializeTo(v);
	cs.deserialize(QCborValue{}, qMetaTypeId<Type>());
	cs.deserialize(QCborValue{}, qMetaTypeId<Type>(), nullptr);
	cs.deserialize<Type>(c);
	cs.deserialize<Type>(c, nullptr);
	cs.deserializeFrom(d, qMetaTypeId<Type>());
	cs.deserializeFrom(d, qMetaTypeId<Type>(), nullptr);
	cs.deserializeFrom(QByteArray{}, qMetaTypeId<Type>());
	cs.deserializeFrom(QByteArray{}, qMetaTypeId<Type>(), nullptr);
	cs.deserializeFrom<Type>(d);
	cs.deserializeFrom<Type>(d, nullptr);
	cs.deserializeFrom<Type>(QByteArray{});
	cs.deserializeFrom<Type>(QByteArray{}, nullptr);

	Json j;
	QJsonSerializer js;
	static_assert(std::is_same<Json, decltype(js.serialize(v))>::value, "Wrong JSON value returned by expression");
	js.serializeTo(d, v);
	js.serializeTo(v);
	js.deserialize(QJsonValue{}, qMetaTypeId<Type>());
	js.deserialize(QJsonValue{}, qMetaTypeId<Type>(), nullptr);
	js.deserialize<Type>(j);
	js.deserialize<Type>(j, nullptr);
	js.deserializeFrom(d, qMetaTypeId<Type>());
	js.deserializeFrom(d, qMetaTypeId<Type>(), nullptr);
	js.deserializeFrom(QByteArray{}, qMetaTypeId<Type>());
	js.deserializeFrom(QByteArray{}, qMetaTypeId<Type>(), nullptr);
	js.deserializeFrom<Type>(d);
	js.deserializeFrom<Type>(d, nullptr);
	js.deserializeFrom<Type>(QByteArray{});
	js.deserializeFrom<Type>(QByteArray{}, nullptr);
}

Q_DECL_UNUSED void static_compile_test()
{
	test_type<QVariant, QCborValue, QJsonValue>();
	test_type<bool, QCborValue, QJsonValue>();
	test_type<int, QCborValue, QJsonValue>();
	test_type<double, QCborValue, QJsonValue>();
	test_type<QString, QCborValue, QJsonValue>();
	test_type<TestObject*, QCborMap, QJsonObject>();
	test_type<QPointer<TestObject>, QCborMap, QJsonObject>();
	test_type<QSharedPointer<TestObject>, QCborMap, QJsonObject>();
	test_type<QList<TestObject*>, QCborArray, QJsonArray>();
	test_type<QMap<QString, TestObject*>, QCborMap, QJsonObject>();
	test_type<QList<int>, QCborArray, QJsonArray>();
	test_type<QMap<QString, bool>, QCborMap, QJsonObject>();
	test_type<QPair<double, bool>, QCborArray, QJsonArray>();
	test_type<TestTuple, QCborArray, QJsonArray>();
	test_type<TestPair, QCborArray, QJsonArray>();
}

}

QTEST_MAIN(SerializerTest)

#include "tst_serializer.moc"
