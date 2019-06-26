﻿
#include "../Core/SymbolDatabase.hpp"
#include "../Core/HeaderParser2.hpp"
#include "../Core/Generators/FlatCGenerator.hpp"


class Test_Struct : public ::testing::Test {};

#define TEST_ROOT u"C:/Proj/LN/Lumino/"

TEST_F(Test_Struct, Basic)
{
    auto diag = ln::makeObject<ln::DiagnosticsManager>();

	auto pidb = ln::makeRef<PIDatabase>();
	HeaderParser2 parser;
	parser.addIncludePath(TEST_ROOT "include");
	parser.addIncludePath(TEST_ROOT "include");
	parser.addForceIncludeFile(TEST_ROOT "include/LuminoCore/Base/Common.hpp");
	parser.parse(LN_LOCALFILE("Assets/Struct/TestStruct.hpp"), pidb, diag);


    auto db = ln::makeRef<SymbolDatabase>(diag);
	db->initTypes(pidb);
    db->linkTypes();

	auto struct1 = db->findTypeSymbol(u"ln::TestStruct1");
	printf("");
 //   ASSERT_EQ(u"float", struct1->declaredFields[0]->type->fullName());
 //   ASSERT_EQ(u"x", struct1->declaredFields[0]->name);
 //   ASSERT_EQ(u"float", struct1->declaredFields[1]->type->fullName());
 //   ASSERT_EQ(u"y", struct1->declaredFields[1]->name);

	FlatCHeaderGenerator g1;
	g1.setup(db, LN_LOCALFILE("../Core/Generators/Templates"), LN_LOCALFILE(""), u"LN", "LN");
	g1.generate();
}
