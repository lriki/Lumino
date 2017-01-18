
#include "Global.h"
#include "Parser.h"
#include "DotNet/DotNetPInvokeLibGenerator.h"
#include "DotNet/DotNetClassLibGenerator.h"
#include "DotNet/DotNetCommon.h"
#include "WrapperIF/WrapperIFGenerator.h"

PathName		g_templateDir;
SymbolDatabase	g_database;

int main()
{
	List<PathName> files =
	{
		"../../../../include/Lumino/Engine.h",
		"../../../../include/Lumino/Audio/Sound.h",
		"../../../../include/Lumino/Base/GeometryStructs.h",
		"../../../../external/Lumino.Core/include/Lumino/Math/Vector3.h",
		"../../../../include/Lumino/Binding/Common.h",
	};

	g_templateDir = LUMINO_ROOT_DIR"/tools/LWIG/";

	
	HeaderParser parser(&g_database);
	parser.ParseFiles(files/*, &database*/);

	g_database.Link();

	DotNetCommon::Initialize();

	{
		WrapperIFGenerator gen;
		gen.Generate(&g_database);
	}
	{
		DotNetPInvokeLibGenerator g;
		g.Generate();
	}
	{
		CSStructsGenerator g;
		g.Generate();
	}

	//
	//
	//{
	//	DotNetPInvokeLibGenerator gen;
	//	gen.Generate(&database);
	//}
	//
	//{
	//	DotNetClassLibGenerator gen;
	//	gen.Generate(&database);
	//}
	
	return 0;
}

