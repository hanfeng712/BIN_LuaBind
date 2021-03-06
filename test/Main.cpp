#include "Public.hpp"

#if defined _DEBUG
#define ASSERT0(c)  assert((c))
#define ASSERT1(c, msg) assert(c && msg) 
#else
#define ASSERT0(c)      c
#define ASSERT1(c, msg) c
#endif

namespace bin
{
	BEGIN_SCRIPT_MODULE(exportModule)
		DEFINE_MODULE_FUNCTION(helloModule, int, (void))
		{
			printf("exportModule [helloModule] : Done\n");
			r = 1;								// return value
			std::string luaStr;
			ASSERT0(lua.Get("luaStr", luaStr));
			ASSERT0(luaStr == "luaStr");
			return 1;
		}
	END_SCRIPT_MODULE()
};

#define BEGIN_TEST_CASE(name)\
void name()\
{\
	printf("Test "#name" :\n");

#define END_TEST_CASE()\
}

BEGIN_TEST_CASE(TestExportModule)
	using namespace bin;
	CScriptHandle   scriptHandle;
	ASSERT1(scriptHandle.Init(), "Init script handle fail");

	ASSERT1(ScriptExporterManager().ExportModule("exportModule", scriptHandle), "Export exportModule fail");

	ASSERT1(scriptHandle.ExecString("luaStr = 'luaStr' \n ret = exportModule.helloModule()"), "Execute script fail");
	
	int nRet = 0;
	ASSERT1(scriptHandle.Get("ret", nRet), "CScriptHandle::Get(int) fail");

	ASSERT1(nRet, "Call exportModule.helloModule Fail");
END_TEST_CASE()

BEGIN_TEST_CASE(TestScriptHandle)
	using namespace bin;
	CScriptHandle   scriptHandle;
	ASSERT0(scriptHandle.Init());

	const char* pszLua = 
		"a = 10\n"
		"b = 'Hello World'\n"
		"c = {x = 10, y='this is y', c_a={1, 2, str='this is c_a.str', tbl={str='this is c_a.tbl'}}}\n"
		"d = {x = 20}\n"
		"function add(t0, t1) return t0.x+t1.x end\n"
		"function getStr() return c.c_a.str end\n"
		;
	ASSERT0(scriptHandle.ExecString(pszLua));
	
	int a = 0;
	ASSERT0(scriptHandle.Get("a", a));
	std::cout<<"a : "<< a << std::endl;
	std::string b = "";
	ASSERT0(scriptHandle.Get("b", b));
	std::cout<< "b : "<< b<< std::endl;
	CScriptTable c;
	CScriptTable d;
	ASSERT0(scriptHandle.Get("c", c));
	ASSERT0(c.IsReferd());
	ASSERT0(scriptHandle.Get("d", d));
	ASSERT0(d.IsReferd());
	int c_x = 0;
	ASSERT0(c.Get("x", c_x));
	std::cout<< "c.x :"<< c_x<< std::endl;
	std::string c_y;
	ASSERT0(c.Get("y", c_y));
	std::cout<< "c.y : "<< c_y<< std::endl;
	int r = 0;
	{
		int nRet = scriptHandle.CallFunc<CScriptTable, CScriptTable>("add", c, d, r);
		ASSERT0(nRet);
	}
	std::cout<<"add() : "<< r<< std::endl;

	{
		CScriptTable c_c_a;
		ASSERT0(c.Get("c_a", c_c_a));
		ASSERT0(c_c_a.IsReferd());

		int c_c_a_1 = 0;
		ASSERT0(c_c_a.Get(1, c_c_a_1));
		std::cout<< "c.c_a[1] : "<< c_c_a_1<< std::endl;
		
		std::string c_c_a_2;
		
		ASSERT0(c_c_a.Get("str", c_c_a_2));
		std::cout<< "c.c_a[2] : "<< c_c_a_2<< std::endl;

		CScriptTable c_c_a_tbl;
		ASSERT0(c_c_a.Get("tbl", c_c_a_tbl));
		ASSERT0(c_c_a_tbl.IsReferd());
		
		std::string c_c_a_tbl_str;
		ASSERT0(c_c_a_tbl.Get("str", c_c_a_tbl_str));
		std::cout<< "c.c_a.tbl.str : "<< c_c_a_tbl_str<< std::endl;
	}

	{
		CScriptTable tbl0;
		CScriptTable tbl1;
		CScriptTable funcTbl;
		{
			CScriptHandle temp;
			ASSERT0(temp.Init());

			const char* pszLua = 
				"tbl = { tbl = {str='This is a temp table'}}"
				;
			ASSERT0(temp.ExecString(pszLua));
		
			ASSERT0(temp.Get("tbl", tbl0));
			ASSERT0(tbl0.IsReferd());
			
			ASSERT0(tbl0.Get("tbl", tbl1));
			ASSERT0(tbl1.IsReferd());

			pszLua = 
				"funcTbl = {}"
				"funcTbl.add = function(x, y)\n"
				"	return x+y\n"
				"end\n"
				"funcTbl.tblAdd = function(tbl0, tbl1)\n"
				"	local retTbl = {}\n"
				"	retTbl.x = tbl0.x+tbl1.x"
				"	return retTbl\n"
				"end";
			ASSERT0(temp.ExecString(pszLua));

			ASSERT0(temp.Get("funcTbl", funcTbl));
			ASSERT0(funcTbl.IsReferd());
			int nRet = 0;
			ASSERT0((funcTbl.CallFunc<int, int>("add", 100, 200, nRet)));
			ASSERT0(nRet == 300);

			CScriptTable argTbl0;
			CScriptTable argTbl1;
		
			ASSERT0(temp.NewTable(argTbl0));
			ASSERT0(temp.NewTable(argTbl1));
			ASSERT0(argTbl0.IsReferd());
			ASSERT0(argTbl1.IsReferd());

			ASSERT0(argTbl0.Set("x", 10));
			ASSERT0(argTbl1.Set("x", 20));
		
			CScriptTable retTbl;

			ASSERT0((funcTbl.CallFunc<CScriptTable, CScriptTable>("tblAdd", argTbl0, argTbl1, retTbl)));
			ASSERT0(retTbl.Get("x", nRet));
			ASSERT0(nRet == 30);
		}

		ASSERT0(!tbl0.IsReferd());
		ASSERT0(!tbl1.IsReferd());
		ASSERT0(!funcTbl.IsReferd());
	}

	// Check string return value
	{
		const char* pszLua = 
			"return 1, \"Hello\", {a=1, b=\"Hello\"}";

		CScriptTable ret;
		ASSERT0(scriptHandle.ExecString(pszLua, ret));
		ASSERT0(ret.IsReferd());

		int nN = 0;
		ret.Get("n", nN);
		ASSERT0(nN == 3);

		int nR1 = 0;
		ret.Get(1, nR1);
		ASSERT0(nR1 == 1);

		std::string strR2;
		ret.Get(2, strR2);
		ASSERT0(strR2 == "Hello");

		CScriptTable R3;
		ret.Get(3, R3);
		ASSERT0(R3.IsReferd());

		int r3_a = 0;
		R3.Get("a", r3_a);
		ASSERT0(r3_a == 1);

		std::string r3_b;
		R3.Get("b", r3_b);
		ASSERT0(r3_b == "Hello");
	}
END_TEST_CASE()

namespace bin
{
	class CClassTest
	{
		DECLARE_SCRIPT_CLASS()
	public:
		CClassTest()
		{
		}

		CClassTest(const std::string& field)
			: m_strField(field)
		{

		}

		~CClassTest()
		{
		}

		void ReadTable(CScriptTable& tbl)
		{
			ASSERT0(tbl.IsReferd());

			m_strField;

			ASSERT0(tbl.Get("msg", m_strField));
		}

		void OperOnUserData()
		{
			CScriptUserData ud;
			ASSERT0(GetScriptUserData(ud));
			ASSERT0(ud.IsReferd());

			std::string scrStr;
			ASSERT0(ud.Get("scrStr", scrStr));
			ASSERT0(scrStr == "this is script string");

			CScriptTable scrTbl;
			ASSERT0(ud.Get("scrTbl", scrTbl));
			ASSERT0(scrTbl.IsReferd());

			int scrInt = 0;
			ASSERT0(ud.Get("scrInt", scrInt));
			ASSERT0(scrInt == 10);
		}

		const std::string& GetField()
		{
			return m_strField;
		}

	public:
		std::string		m_strField;
	};

	BEGIN_SCRIPT_CLASS(exportTestClass, CClassTest)
		DEFINE_CLASS_FUNCTION(readTable, std::string, (CScriptTable& tbl))
		{
			std::cout<< "readTable() is called\n";
			obj->ReadTable(tbl);
			r = obj->m_strField;
			return 1;
		}
		DEFINE_CLASS_FUNCTION(returnTable, CScriptTable, ())
		{	
			std::cout<< "returnTable() is called\n";

			obj->GetScriptHandle().NewTable(r);

			r.Set("msg", obj->m_strField);
			
			return 1;
		}

		DEFINE_CLASS_FUNCTION(opUserdata, bool, ())
		{	
			std::cout<< "opUserdata() is called\n";

			obj->OperOnUserData();

			r = true;

			return 1;
		}

		DEFINE_CLASS_FUNCTION(retVoid, void, ())
		{	
			std::cout<< "retVoid() is called\n";

			return 1;
		}

		DEFINE_CLASS_FUNCTION(getField, std::string, ())
		{
			r = obj->GetField();
			return 1;
		}

		DEFINE_STATIC_FUNCTION(newInstance, CClassTest*, ())
		{
			r = new CClassTest("Hello");
			r->GetScriptObject().SetDelByScr(true);

			return 1;
		}

	END_SCRIPT_CLASS()

	BEGIN_SCRIPT_MODULE(testClassFactory)
		DEFINE_MODULE_FUNCTION(create, CClassTest*, ())
		{
			std::cout<< "testClassFactory.create() is called\n";
			r = new CClassTest;
			r->GetScriptObject().SetDelByScr(true);

			return 1;
		}
	END_SCRIPT_MODULE()
};

BEGIN_TEST_CASE(TestExportClass)
	using namespace bin;
	
	CScriptUserData obj;
	CClassTest      obj2;

	// Test Static Function
	{
		CScriptHandle   scriptHandle;
		ASSERT0(scriptHandle.Init());

		ASSERT0(ScriptExporterManager().ExportClass("exportTestClass", scriptHandle));

		ASSERT0(scriptHandle.ExecString("obj = bin_types.exportTestClass.newInstance()"));

		CScriptUserData obj;
		scriptHandle.Get("obj", obj);

		std::string field;
		obj.CallMemFunc("getField", field);

		ASSERT0(field == "Hello");
	}

	{
		CScriptHandle   scriptHandle;
		ASSERT0(scriptHandle.Init());

		ASSERT0(ScriptExporterManager().ExportModule("testClassFactory", scriptHandle));
		ASSERT0(ScriptExporterManager().ExportClass("exportTestClass", scriptHandle));

		const char* pszLua = 
			"obj = testClassFactory.create()\n"
			"tbl = {msg = 'this is the arg of readTable()'}"
			"str = obj:readTable(tbl)\n"
			;
		ASSERT0(scriptHandle.ExecString(pszLua));
		
		
		ASSERT0(scriptHandle.Get("obj", obj));
		ASSERT0(obj.IsReferd());
		// Set/read script values  
		{
			int x = 0;
			ASSERT0(obj.Set("x", 100));
			ASSERT0(obj.Get("x", x));
			ASSERT0(x == 100);

			ASSERT0(obj.Set("scrStr", "this is script string"));
			ASSERT0(obj.Set("scrInt", 10));
			CScriptTable scrTbl;
			scriptHandle.NewTable(scrTbl);
			ASSERT0(obj.Set("scrTbl", scrTbl));
			bool bRet = false;
			ASSERT0(obj.CallMemFunc("opUserdata", bRet));
			ASSERT0(bRet);
            ASSERT0(obj.CallMemFunc("retVoid", RET_VOID));
		}
	
		scriptHandle.Set<CClassTest*>("obj2", &obj2);
		ASSERT0(obj2.IsExported());

		ASSERT0(scriptHandle.ExecString("obj2:readTable({ msg = 'obj2_msg'})"));
		ASSERT0(obj2.m_strField == "obj2_msg");

		CScriptTable retTbl;
		ASSERT0(obj.CallMemFunc("returnTable", retTbl));
		ASSERT0(retTbl.IsReferd());
		std::string retMsg;
		ASSERT0(retTbl.Get("msg", retMsg));

		// Check weak table mechanism
		{
			// Script own(Del by script) objects will be collected, but Script use(Not del by script) objects will not be collected
			const int OBJ_NUM = 10;
			CClassTest*	objs[OBJ_NUM] = {NULL};
			CScriptUserData objUds[OBJ_NUM];

			CScriptUserData rdUd;
			int				nRdIdx = rand()%OBJ_NUM;

			for(int i=0; i<10; ++i)
			{
				objs[i] = new CClassTest;
				
				char szName[16] = {0};

				sprintf(szName, "obj%d", i);
				ASSERT0(scriptHandle.Set(szName, objs[i]));

				objs[i]->GetScriptObject().SetDelByScr(true);
			}

			for(int i=0; i<10; ++i)
			{
				ASSERT0(objs[i]->IsExported());
			}

			objs[nRdIdx]->GetScriptObject().SetDelByScr(false);

			for(int i=0; i<10; ++i)
			{
				char szScr[16] = {0};

				sprintf(szScr, "obj%d = nil", i);
				ASSERT0(scriptHandle.ExecString(szScr));
			}

			
			// All the object will be collected
			ASSERT0(scriptHandle.ExecString("collectgarbage()"));
						
			for(int i=0; i<10; ++i)
			{
				if(i == nRdIdx)
				{
					ASSERT0(objs[i]->IsExported());
				}
				else	// All the objects have been deleted
				{
					//ASSERT0(!objs[i]->IsExported());

					//delete objs[i];
					//objs[i] = NULL;
				}
			}

			rdUd.UnRef();
			// Collect the random userdata
			ASSERT0(scriptHandle.ExecString("collectgarbage()"));
			
			//ASSERT0(!objs[nRdIdx]->IsExported());
			ASSERT0(!rdUd.IsReferd());
			//delete objs[nRdIdx];
			//objs[nRdIdx] = NULL;
		}

		// Check c++ object's userdata will be invalid autumatically
		{
			CScriptUserData ud;
			{
				ASSERT0(!ud.IsReferd());

				CClassTest tempObj;

				ASSERT0(!tempObj.IsExported());

				ASSERT0(scriptHandle.Set("tempObj", &tempObj));

				ASSERT0(tempObj.IsExported());
				tempObj.GetScriptUserData(ud);
				ASSERT0(ud.IsReferd());
			}

			ASSERT0(!ud.IsReferd());
		}
	}

	ASSERT0(!obj.IsReferd());
	ASSERT0(!obj2.IsExported());
END_TEST_CASE()

BEGIN_TEST_CASE(TestSetScriptValue)
	using namespace bin;	
	
	CScriptHandle scriptHandle;
	ASSERT0(scriptHandle.Init());

	ASSERT0(ScriptExporterManager().ExportClass("exportTestClass", scriptHandle));

	ASSERT0(scriptHandle.Set("a", 1));
	int a = 0;
	ASSERT0(scriptHandle.Get("a", a));
	ASSERT0(a == 1);

	ASSERT0(scriptHandle.Set("b", "this is cstring"));
	const char* b = 0;
	ASSERT0(scriptHandle.Get("b", b));
	ASSERT0(strcmp(b, "this is cstring") == 0);
	
	std::string c_v = "this is std::string";
	ASSERT0(scriptHandle.Set("c", c_v));
	std::string c;
	ASSERT0(scriptHandle.Get("c", c));
	ASSERT0(c == "this is std::string");

	// Set table to global
	{
		ASSERT0(scriptHandle.ExecString(" tbl_v = {x=0, y=1}"));
		CScriptTable tbl_v;		
		ASSERT0(scriptHandle.Get("tbl_v", tbl_v));
		ASSERT0(tbl_v.IsReferd());
		ASSERT0(scriptHandle.Set("tbl", tbl_v));
		CScriptTable tbl;
		ASSERT0(scriptHandle.Get("tbl", tbl));
		ASSERT0(tbl.IsReferd());

		int y = 0;
		ASSERT0(tbl.Get("y", y));
		ASSERT0(y == 1);
	}

	// Set user data to global
	{
		CClassTest clsTest;
		clsTest.GetScriptObject().SetDelByScr(false);
		clsTest.m_strField = "this is clsTest";
		ASSERT0(scriptHandle.Set("clsTestObj", &clsTest));

		// C++ object
		CClassTest* pClsTest = NULL;
		ASSERT0(scriptHandle.Get("clsTestObj", pClsTest));

		ASSERT0(pClsTest == &clsTest);
		ASSERT0(pClsTest->m_strField == "this is clsTest");

		// Script User Data
		CScriptUserData ud;
		ASSERT0(scriptHandle.Get("clsTestObj", ud));
		ASSERT0(ud.IsReferd());
	}

	// Set table field
	{
		CScriptTable testTbl;
		ASSERT0(scriptHandle.NewTable(testTbl));
		ASSERT0(testTbl.IsReferd());

		{
			ASSERT0(testTbl.Set("a", 1));
			int a = 0;
			ASSERT0(testTbl.Get("a", a));
			ASSERT0(a == 1);

			ASSERT0(testTbl.Set("b", "this is cstring"));
			const char* b = 0;
			ASSERT0(testTbl.Get("b", b));
			ASSERT0(strcmp(b, "this is cstring") == 0);

			std::string c_v = "this is std::string";
			ASSERT0(testTbl.Set("c", c_v));
			std::string c;
			ASSERT0(testTbl.Get("c", c));
			ASSERT0(c == "this is std::string");
		}
		
		CScriptTable testTblRef;
		ASSERT0(scriptHandle.Set("testTblRef", testTbl));
		ASSERT0(scriptHandle.Get("testTblRef", testTblRef));

		{
			int a = 0;
			ASSERT0(testTblRef.Get("a", a));
			ASSERT0(a == 1);

			const char* b = 0;
			ASSERT0(testTblRef.Get("b", b));
			ASSERT0(strcmp(b, "this is cstring") == 0);

			std::string c;
			ASSERT0(testTblRef.Get("c", c));
			ASSERT0(c == "this is std::string");
		}
	}
END_TEST_CASE()

namespace bin
{
	class CSuper
	{
		DECLARE_SCRIPT_CLASS()
	public:
		CSuper()
		{
			m_strSupMsg = "CSuper";
		}

		CSuper(const CSuper& r)
			: m_strSupMsg(r.m_strSupMsg)
		{

		}

		CSuper& operator=(const CSuper& r)
		{
			m_strSupMsg = r.m_strSupMsg;

			return *this;
		}

		virtual ~CSuper()
		{
		}

		const std::string& GetMsg()
		{
			return m_strSupMsg;
		}

	public:
		std::string		m_strSupMsg;
	};

	class CSub : public CSuper
	{
		DECLARE_SCRIPT_SUB_CLASS(CSuper)
	public:
		CSub()
		{
			m_strSubMsg = "CSub";
		}

		virtual ~CSub()
		{

		}

		const std::string& GetMsg()
		{
			return m_strSubMsg;
		}
	public:
		std::string			m_strSubMsg;
	};

	class CSuper0
	{
	public:
		virtual ~CSuper0()
		{

		}

		int a;
	};

	// CSub must be the first super class, so &CSubSub == &CSub
	//class CSubSub : public CSuper0, public CSub  
	class CSubSub : public CSub, public CSuper0   
	{
		DECLARE_SCRIPT_SUB_CLASS(CSub)
	public:
		CSubSub()
		{
			m_strSubSubMsg = "CSubSub";
		}

		virtual ~CSubSub()
		{

		}

		const std::string& GetMsg()
		{
			return m_strSubSubMsg;
		}
	public:
		std::string m_strSubSubMsg;
	};

	BEGIN_SCRIPT_CLASS(super, CSuper)
		DEFINE_CLASS_FUNCTION(getMsg, std::string, ())
		{
			r = obj->m_strSupMsg;

			return 1;
		}

		DEFINE_CLASS_FUNCTION(superFunc, int, ())
		{
			r = 1;

			std::cout<< "super::superFunc\n";

			return 1;
		}
	END_SCRIPT_CLASS()

	BEGIN_SCRIPT_CLASS(sub, CSub)
		SUPER_CLASS(super, CSuper)
		DEFINE_CLASS_FUNCTION(getMsg, std::string, ())
		{
			r = obj->m_strSubMsg;
			std::cout<< "sub::getMsg\n";

			return 1;
		}
		
		DEFINE_CLASS_FUNCTION(subFunc, int, ())
		{
			r = 2;

			std::cout<< "sub::subFunc\n";

			return 1;
		}
	END_SCRIPT_CLASS()

	BEGIN_SCRIPT_CLASS(subsub, CSubSub)
		SUPER_CLASS(sub, CSub)
		DEFINE_CLASS_FUNCTION(getMsg, std::string, ())
		{
			r = obj->m_strSubSubMsg;
			std::cout<< "subsub::getMsg\n";

			return 1;
		}

		DEFINE_CLASS_FUNCTION(subsubFunc, int, ())
		{
			r = 100;

			std::cout<< "subsub::subsubFunc\n";

			return 1;
		}
	END_SCRIPT_CLASS()

    BEGIN_SCRIPT_MODULE(core)
        DEFINE_MODULE_FUNCTION(newSuper, CSuper*, ())
        {
            CSuper* pObj = new CSuper;

            pObj->GetScriptObject().SetDelByScr(true);

            r = pObj;

            return 1;
        }

        DEFINE_MODULE_FUNCTION(newSub, CSub*, ())
        {
            CSub* pObj = new CSub;

            pObj->GetScriptObject().SetDelByScr(true);

            r = pObj;

            return 1;
        }

        DEFINE_MODULE_FUNCTION(newSubSub, CSubSub*, ())
        {
            CSubSub* pObj = new CSubSub;

            pObj->GetScriptObject().SetDelByScr(true);

            r = pObj;

            return 1;
        }
    END_SCRIPT_MODULE()
};

BEGIN_TEST_CASE(TestInheritance)
	using namespace bin;	

	CScriptHandle scriptHandle;
	ASSERT0(scriptHandle.Init());

	ASSERT0(ScriptExporterManager().ExportClass("super", scriptHandle));
	ASSERT0(ScriptExporterManager().ExportClass("sub", scriptHandle));
	ASSERT0(ScriptExporterManager().ExportClass("subsub", scriptHandle));


	CSub sub;
	{
		CSuper& super = sub;

		ASSERT0(scriptHandle.Set("obj0", &super));

		ASSERT0(scriptHandle.ExecString("msg = obj0:getMsg()"));

		std::string strMsg;
		ASSERT0(scriptHandle.Get("msg", strMsg));
		ASSERT0(strMsg == "CSub");

		ASSERT0(scriptHandle.ExecString("obj0:superFunc()"));
	}

	CSubSub subsub;
	{
		CSuper& super = subsub;

		ASSERT0(scriptHandle.Set("obj1", &super));

		ASSERT0(scriptHandle.ExecString("msg = obj1:getMsg()"));

		std::string strMsg;
		ASSERT0(scriptHandle.Get("msg", strMsg));
		ASSERT0(strMsg == "CSubSub");

		ASSERT0(scriptHandle.ExecString("obj1:subFunc()"));
	}

	// Test Copy constructor and Opertor = 
	{
		CSuper src;
		src.m_strSupMsg = "src";

		CSuper dst;
		dst.m_strSupMsg = "dst";

		ASSERT0(scriptHandle.Set("dstObj", &dst));

		CScriptTable ret;
		scriptHandle.ExecString("return dstObj:getMsg()", ret);
		std::string msg;
		ret.Get(1, msg);
		ASSERT0(msg == "dst");

		dst = src;

		ret.UnRef();
		scriptHandle.ExecString("return dstObj:getMsg()", ret);
		ret.Get(1, msg);
		ASSERT0(msg == "src");

		dst.m_strSupMsg = "dst";
		CSuper cpy(dst);

		ASSERT0(scriptHandle.Set("cpyObj", &cpy));

		ret.UnRef();
		scriptHandle.ExecString("return cpyObj:getMsg()", ret);
		ret.Get(1, msg);
		ASSERT0(msg == "dst");
	}

    // Test Script Inheritance
    {
        ASSERT0(ScriptExporterManager().ExportModule("core", scriptHandle));
        ASSERT0(scriptHandle.Exec("script/scriptInheritance.lua"));

        {
            ASSERT0(scriptHandle.ExecString("obj = core.newScriptSub()"));
            CScriptUserData obj;
            ASSERT0(scriptHandle.Get("obj", obj));

            std::string strMsg;
            ASSERT0(obj.CallMemFunc("getScriptMessage", strMsg));
            ASSERT0(strMsg == "type_scriptSub");

            ASSERT0(obj.CallMemFunc("getMsg", strMsg));
            ASSERT0(strMsg == "CSuper");         
        }

        {
            ASSERT0(scriptHandle.ExecString("obj = core.newScriptSubSub()"));

            CScriptUserData obj;
            ASSERT0(scriptHandle.Get("obj", obj));

            std::string strMsg;
            ASSERT0(obj.CallMemFunc("getScriptSubMessage", strMsg));
            ASSERT0(strMsg == "type_scriptSub");

            ASSERT0(obj.CallMemFunc("getScriptMessage", strMsg));
            ASSERT0(strMsg == "type_scriptSubSub");

            ASSERT0(obj.CallMemFunc("getMsg", strMsg));
            ASSERT0(strMsg == "CSuper");
        }

        {
            ASSERT0(scriptHandle.ExecString("obj = core.newScriptSubSub_subsub()"));

            CScriptUserData obj;
            ASSERT0(scriptHandle.Get("obj", obj));

            std::string strMsg;

            ASSERT0(obj.CallMemFunc<std::string>("getMsg", strMsg));
            ASSERT0(strMsg == "type_scriptSubSub_subsub");

            int nRet = 0;
            ASSERT0(obj.CallMemFunc("subsubFunc", nRet));
            ASSERT0(nRet == 100);

            ASSERT0(obj.CallMemFunc("subFunc", nRet));
            ASSERT0(nRet == 2);
        }
    }

END_TEST_CASE()

namespace bin
{
	class CGuiDialog
	{
		DECLARE_SCRIPT_CLASS()
	public:
		CGuiDialog()
			: m_nID(0)
		{

		}

		virtual ~CGuiDialog()
		{

		}

		int LoadUI(const char* pszUI)
		{
			std::cout<< "Create Dialog [ "<< pszUI<< " ]\n";
			m_nID = rand()%100;

			return OnCreate();
		}

		virtual int OnCreate()
		{
			CScriptUserData ud;
			GetScriptUserData(ud);

			bool bRet = false;
			ASSERT0(ud.CallMemFunc("onCreate", bRet));

			return bRet ? 1 : 0;
		}

		int GetID() const
		{
			return m_nID;
		}
	private:
		int				m_nID;
	};

	BEGIN_SCRIPT_CLASS(guiDialog, CGuiDialog)
		DEFINE_CLASS_FUNCTION(getID, int, ())
		{
			r = obj->GetID();

			return 1;
		}

		DEFINE_CLASS_FUNCTION(onCreate, void, ())
		{
			std::cout<< "guiDialog::onCreate\n";

			return 1;
		}
	END_SCRIPT_CLASS()

	BEGIN_SCRIPT_MODULE(gui)
		DEFINE_MODULE_FUNCTION(newDialog, CGuiDialog*, ())
		{
			CGuiDialog* pDlg = new CGuiDialog;
			
			r = pDlg;

			return 1;
		}

		DEFINE_MODULE_FUNCTION(loadDialog, int, (CGuiDialog* pDlg, const char* pszUI))
		{
			r = 0;

			if(!pDlg)
			{
				return 0;
			}

			r = pDlg->LoadUI(pszUI);
			if(!r)
			{
				delete pDlg;
				pDlg   = NULL;
			}

			return 1;
		}

		DEFINE_MODULE_FUNCTION(moduleToTable, void, ())
		{
			std::cout<< "moduleToTable\n";
			return 1;
		}

		DEFINE_MODULE_FUNCTION(returnTable, CScriptTable, ())
		{
			lua.NewTable(r);

			r.Set("msg", "returnTable");
			
			return 1;
		}

	END_SCRIPT_MODULE()

	BEGIN_SCRIPT_MODULE(logger)
		DEFINE_MODULE_FUNCTION(__bin_log, void, (const char* pszMsg))
		{
			LOG_MESSAGE(pszMsg);

			return 1;
		}
	END_SCRIPT_MODULE()
}

BEGIN_TEST_CASE(TestApplication)
	using namespace bin;

	CScriptHandle scriptHandle;
	ASSERT0(scriptHandle.Init());

	ASSERT0(scriptHandle.Exec("script/initialize.lua"));

END_TEST_CASE()

namespace bin
{
	class CGCTest
	{
		DECLARE_SCRIPT_CLASS()
	public:
		CGCTest()
		{

		}

		~CGCTest()
		{

		}

		void TestScriptHandle()
		{
			std::string msg;
			GetScriptHandle().Get("msg", msg);
		}
	};

	BEGIN_SCRIPT_CLASS(gcTest, CGCTest)

	END_SCRIPT_CLASS()
};

BEGIN_TEST_CASE(TestGC)
	using namespace bin;

	CScriptHandle scriptHandle;
	ASSERT0(scriptHandle.Init());

	ASSERT0(ScriptExporterManager().ExportClass("gcTest", scriptHandle));


	CGCTest obj;
	{
		scriptHandle.Set("obj", &obj);
		scriptHandle.Set("obj", 1);
		// Now obj can be gc

		obj.TestScriptHandle();
	}

END_TEST_CASE()

int main(int argc, char** argv)
{
	TestExportModule();
	TestScriptHandle();
	TestExportClass();
	TestSetScriptValue();
	TestInheritance();
	TestApplication();
	TestGC();

	return 0;
}