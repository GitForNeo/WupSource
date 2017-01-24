#include "jce2java.h"
#include "interface_analysis.h"
#include "jce_filter.h"
#include "util/tc_md5.h"
#include "util/tc_file.h"
#include "util/tc_common.h"
#include <string>

#define TAB     g_parse->getTab()
#define INC_TAB g_parse->incTab()
#define DEL_TAB g_parse->delTab()

static string firstCharUpper(const string& castStr) {
	string resultStr = castStr;
	if (castStr.size() > 0) {
		resultStr[0] = toupper(castStr[0]);
	}
	return resultStr;
}

//////////////////////////////////////////////////////////////////////////////////
//
Jce2Java::Jce2Java()
{
	_bWithServant = false;
	_bWithWsp = false;
	_bWithAsync = false;
	_bForceArray = false;
	_bHolder = false;
	_bCheckDefault = false;
	_bWithJbr = false;
	_bWithCompact = false;

	_bWithPrx = true;
	_bWithStubAndroid = false;
	_bWithGenerateInterfaceDependencies = false;
	_bWithFilterRomJce = false;

	s_JCE_PACKAGE 		= std::string("com.qq") + JCE_PACKAGE;
	s_PROXY_PACKAGE		= std::string("com.qq") + PROXY_PACKAGE;
	s_HOLDER_PACKAGE	= std::string("com.qq") + HOLDER_PACKAGE;
	s_SERVER_PACKAGE	= std::string("com.qq") + SERVER_PACKAGE;
	s_WUP_PACKAGE		= std::string("com.qq") + WUP_PACKAGE;
	s_WSP_PACKAGE		= std::string("com.qq") + WSP_PACKAGE;
}

string Jce2Java::writeTo(const TypeIdPtr &pPtr) const
{
	ostringstream s;
	if (EnumPtr::dynamicCast(pPtr->getTypePtr()))
	{
		s << TAB << "_os.write(" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
	}
	else
	{
		s << TAB << "_os.write(" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
	}

	return s.str();
}

string Jce2Java::readFrom(const TypeIdPtr &pPtr) const
{
	ostringstream s;
	if (EnumPtr::dynamicCast(pPtr->getTypePtr()))
	{
		s << TAB << "_is.read((taf::Int32&)" << pPtr->getId();
	}
	else
	{
		s << TAB << "_is.read(" << pPtr->getId();
	}

	s << ", " << pPtr->getTag() << ", " << (pPtr->isRequire()?"true":"false") << ");" << endl;

	return s.str();
}

string Jce2Java::display(const TypeIdPtr &pPtr) const
{
	ostringstream s;
	if (EnumPtr::dynamicCast(pPtr->getTypePtr()))
	{
		s << TAB << "_ds.display((taf::Int32)" << pPtr->getId() << ",\"" << pPtr->getId() << "\");" << endl;;
	}
	else
	{
		s << TAB << "_ds.display(" << pPtr->getId() << ",\"" << pPtr->getId() << "\");" << endl;;
	}

	return s.str();
}

string Jce2Java::toTypeInit(const TypePtr &pPtr) const
{
	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(pPtr);
	if (bPtr)
	{
		switch (bPtr->kind())
		{
		case Builtin::KindBool:
			return "false;";
		case Builtin::KindByte:
			return "0;";
		case Builtin::KindShort:
			return "0;";
		case Builtin::KindInt:
			return "0;";
		case Builtin::KindLong:
			return "0L;";
		case Builtin::KindFloat:
			return "0.0f;";
		case Builtin::KindDouble:
			return "0.0;";
		case Builtin::KindString:
			return "\"\";";
			//return "null;";
		default:
			return "";
		}
	}

	VectorPtr vPtr = VectorPtr::dynamicCast(pPtr);
	//if(vPtr) return "new " + tostrVector(vPtr) + "();";
	//if(vPtr) return "(" + toObjStr(vPtr->getTypePtr()) + "[]) new Object[1];";
	if (vPtr)
	{
		/*
		//�������⴦��
		string sType;
		size_t iPosBegin, iPosEnd;
		sType = tostr(vPtr->getTypePtr());
		//map<>������ȥ�� '<...>'
		if((iPosBegin = sType.find("<")) != string::npos && (iPosEnd = sType.rfind(">")) != string::npos)
		{
			sType = sType.substr(0, iPosBegin) +  sType.substr(iPosEnd+1);
		}
		//[] (����)�������Ϊ[1]
		sType = taf::TC_Common::replace(sType, "[]" , "[1]");
		return "(" + tostr(vPtr->getTypePtr()) + "[]) new " + sType + "[1];";;
		*/

		BuiltinPtr bPtr = BuiltinPtr::dynamicCast(vPtr->getTypePtr());
		if (bPtr && bPtr->kind() == Builtin::KindByte)
		{
			return "(" + tostr(vPtr->getTypePtr()) + "[]) new " + tostr(vPtr->getTypePtr()) + "[1];";
		}

		if (_bForceArray)
		{
			string sType;
			size_t iPosBegin, iPosEnd;
			sType = tostr(vPtr->getTypePtr());
			//map<>������ȥ�� '<...>'
			if ((iPosBegin = sType.find("<")) != string::npos && (iPosEnd = sType.rfind(">")) != string::npos)
			{
				sType = sType.substr(0, iPosBegin) +  sType.substr(iPosEnd+1);
			}
			//[] (����)�������Ϊ[1]
			sType = taf::TC_Common::replace(sType, "[]" , "[1]");
			return "(" + tostr(vPtr->getTypePtr()) + "[]) new " + sType + "[1];";;
			//return "(" + tostr(vPtr->getTypePtr()) + "[]) new " + tostr(vPtr->getTypePtr()) + "[1];";;
		}

		return "new " + tostrVector(vPtr)+"();";
	}

	MapPtr mPtr = MapPtr::dynamicCast(pPtr);
	if (mPtr) return "new " + tostrMap(mPtr, true) + "();";

	StructPtr sPtr = StructPtr::dynamicCast(pPtr);
	if (sPtr) return "new " + tostrStruct(sPtr) + "();";

	EnumPtr ePtr = EnumPtr::dynamicCast(pPtr);
	if (ePtr) return "0;";

	return "";
}


string Jce2Java::toObjStr(const TypePtr &pPtr) const
{
	string sType = tostr(pPtr);

	if (sType == "boolean")	return "Boolean";
	if (sType == "byte")	return "Byte";
	if (sType == "short" )	return "Short";
	if (sType == "int" )	return "Integer";
	if (sType == "long" )	return "Long";
	if (sType == "float" )	return "Float";
	if (sType == "double" )	return "Double";

	return sType;
}

bool Jce2Java::isObjType(const TypePtr &pPtr) const
{
	EnumPtr ePtr = EnumPtr::dynamicCast(pPtr);
	if (ePtr)
	{
		return false;
	}

	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(pPtr);
	if (bPtr)
	{
		switch (bPtr->kind())
		{
		case Builtin::KindBool:
			return false;
		case Builtin::KindByte:
			return false;
		case Builtin::KindShort:
			return false;
		case Builtin::KindInt:
			return false;
		case Builtin::KindLong:
			return false;
		case Builtin::KindFloat:
			return false;
		case Builtin::KindDouble:
			return false;
		default:
			return true;
		}
	}

	return true;
}

string Jce2Java::tostr(const TypePtr &pPtr) const
{
	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(pPtr);
	if (bPtr) return tostrBuiltin(bPtr);

	VectorPtr vPtr = VectorPtr::dynamicCast(pPtr);
	if (vPtr) return tostrVector(vPtr);

	MapPtr mPtr = MapPtr::dynamicCast(pPtr);
	if (mPtr) return tostrMap(mPtr);

	StructPtr sPtr = StructPtr::dynamicCast(pPtr);
	if (sPtr) return tostrStruct(sPtr);

	EnumPtr ePtr = EnumPtr::dynamicCast(pPtr);
	if (ePtr) return tostrEnum(ePtr);

	if (!pPtr) return "void";

	assert(false);
	return "";
}

/*******************************BuiltinPtr********************************/
string Jce2Java::tostrBuiltin(const BuiltinPtr &pPtr) const
{
	string s;

	switch (pPtr->kind())
	{
	case Builtin::KindBool:
		s = "boolean";
		break;
	case Builtin::KindByte:
		s = "byte";
		break;
	case Builtin::KindShort:
		s = "short";
		break;
	case Builtin::KindInt:
		s = "int";
		break;
	case Builtin::KindLong:
		s = "long";
		break;
	case Builtin::KindFloat:
		s = "float";
		break;
	case Builtin::KindDouble:
		s = "double";
		break;
	case Builtin::KindString:
		s = "String";
		break;
	case Builtin::KindVector:
		s = "java.util.ArrayList";
		break;
	case Builtin::KindMap:
		s = "java.util.HashMap";
		break;
	default:
		assert(false);
		break;
	}

	return s;
}
/*******************************VectorPtr********************************/
string Jce2Java::tostrVector(const VectorPtr &pPtr) const
{
	//string s = "java.util.ArrayList" + string("<") + toObjStr(pPtr->getTypePtr()) + ">";
	string s;
	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(pPtr->getTypePtr());
	if (bPtr && bPtr->kind() == Builtin::KindByte )
	{
		s ="byte []";
	}
	else if (_bForceArray)
	{
		s = tostr(pPtr->getTypePtr()) + "[]";
	}
	else
	{
		s = "java.util.ArrayList" + string("<") + toObjStr(pPtr->getTypePtr()) + ">";
	}
	return s;

}

/*******************************MapPtr********************************/
string Jce2Java::tostrMap(const MapPtr &pPtr, bool bNew) const
{
	string s;
	if (!bNew)
	{
		s = "java.util.Map";
	}
	else
	{
		s = "java.util.HashMap";
	}
	s += string("<") + toObjStr(pPtr->getLeftTypePtr()) + ", " + toObjStr(pPtr->getRightTypePtr()) + ">";

	return s;
}

string Jce2Java::tostrStruct(const StructPtr &pPtr) const
{
	return _packagePrefix + taf::TC_Common::replace(pPtr->getSid(), "::", ".");
}

string Jce2Java::tostrEnum(const EnumPtr &pPtr) const
{
	return "int";
}

/////////////////////////////////////////////////////////////////////

string Jce2Java::MD5(const StructPtr &pPtr) const
{
	string s;
	vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();
	for (size_t j = 0; j < member.size(); j++)
	{
		s += "_" + tostr(member[j]->getTypePtr());
	}

	return "\"" + taf::TC_MD5::md5str(s) + "\"";
}


string Jce2Java::generateJava(const ParamDeclPtr &pPtr) const
{
	ostringstream s;

	TypePtr typePtr = pPtr->getTypeIdPtr()->getTypePtr();
	if (!pPtr->isOut())
	{
		s << tostr(typePtr);
	}
	else
	{
		s << generateHolder(pPtr);
	}

	s << " " << pPtr->getTypeIdPtr()->getId();

	return s.str();
}

string Jce2Java::generateHolder(const ParamDeclPtr &pPtr) const
{
	ostringstream s;

	TypePtr typePtr = pPtr->getTypeIdPtr()->getTypePtr();

	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(typePtr);
	if (bPtr) s << s_HOLDER_PACKAGE << ".Jce" + taf::TC_Common::upper(tostrBuiltin(bPtr).substr(0,1))
		+ tostrBuiltin(bPtr).substr(1) + "Holder";

	VectorPtr vPtr = VectorPtr::dynamicCast(typePtr);
	if (vPtr)
	{
		BuiltinPtr bPtr = BuiltinPtr::dynamicCast(vPtr->getTypePtr());
		if ((bPtr && bPtr->kind() == Builtin::KindByte) || _bForceArray)
		{
			s << s_HOLDER_PACKAGE << ".JceArrayHolder";
		}
		else
		{
			s << s_HOLDER_PACKAGE << ".JceArrayListHolder";
		}
	}
	MapPtr mPtr = MapPtr::dynamicCast(typePtr);
	if (mPtr) s << s_HOLDER_PACKAGE << ".JceHashMapHolder";

	StructPtr sPtr = StructPtr::dynamicCast(typePtr);
	if (sPtr) s << _packagePrefix + taf::TC_Common::replace(sPtr->getSid(), "::", ".") + "Holder";

	EnumPtr ePtr = EnumPtr::dynamicCast(typePtr);
	if (ePtr) s << s_HOLDER_PACKAGE << ".JceIntHolder";

	return s.str();
}

struct SortOperation
{
	bool operator()(const OperationPtr &o1, const OperationPtr &o2)
	{
		return o1->getId() < o2->getId();
	}
};

string Jce2Java::generateJava(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;

	if (_bWithPrx) {
		//��ɴ���ӿ�
		generatePrx(pPtr, nPtr);

		//��ɴ��������
		generatePrxHelper(pPtr, nPtr);

		//��ɴ���ص���
		generatePrxCallback(pPtr, nPtr);
	}

	//��ɷ���˿��
	if (_bWithServant)
	{
		generateServant(pPtr, nPtr);
	}

	if (_bWithStubAndroid) {
		generateAndroidStub(pPtr, nPtr);
	}

	return s.str();
}

string Jce2Java::generatePrx(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public interface " << pPtr->getId() << "Prx" << endl;
	s << TAB << "{" << endl;
	INC_TAB;

	string sParams;
	vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();
	std::sort(vOperation.begin(), vOperation.end(), SortOperation());

	//��ɿͻ��˽ӿڵ�����
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();

		//1.��contextͬ������
		if (_bWithAsync) s << TAB << "///@async" << endl;
		s << TAB << "public " << tostr(vOperation[i]->getReturnPtr()->getTypePtr()) << " "
		<< vOperation[i]->getId() << "(" ;
		sParams = "";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			sParams += generateJava(vParamDecl[j]) + ", ";
		}
		s << taf::TC_Common::trimright(sParams, ", ", false) << ");" << endl;
		s << endl;

		//2.��contextͬ������
		if (_bWithAsync) s << TAB << "///@async" << endl;
		s << TAB << "public " << tostr(vOperation[i]->getReturnPtr()->getTypePtr()) << " "
		<< vOperation[i]->getId() << "(" ;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << generateJava(vParamDecl[j]) << ", ";
		}
		s << "java.util.Map __ctx);" << endl;
		s << endl;

		//3.��context�첽����
		s << TAB << "public void async_" << vOperation[i]->getId() << "(" ;
		s << _packagePrefix << nPtr->getId() << "." << pPtr->getId() << "PrxCallback callback";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;
			s << ", " << generateJava(vParamDecl[j]);
		}
		s << ");" << endl;
		s << endl;

		//4.��context�첽����
		s << TAB << "public void async_" << vOperation[i]->getId() << "(" ;
		s << _packagePrefix << nPtr->getId() << "." << pPtr->getId() << "PrxCallback callback";
		s << ", ";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;
			s << generateJava(vParamDecl[j]) << ", ";
		}
		s << "java.util.Map __ctx);" << endl;
		s << endl << endl;
	}

	DEL_TAB;

	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + "Prx.java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

string Jce2Java::generatePrxHelper(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public final class " << pPtr->getId() << "PrxHelper extends "
	<< s_PROXY_PACKAGE << ".ServantProxy implements "
	<< pPtr->getId() << "Prx"<< endl;
	s << TAB << "{" << endl;
	INC_TAB;

	s << endl;

	//hash
	s << TAB << "public " << pPtr->getId() << "PrxHelper taf_hash(int hashCode)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "super.taf_hash(hashCode);" << endl;
	s << TAB << "return this;" << endl;
	DEL_TAB;
	s << TAB << "}"<< endl;
	s << endl;

	//__defaultContext
	s << TAB << "public java.util.Map __defaultContext()" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "java.util.HashMap _ctx = new java.util.HashMap();" << endl;
	s << TAB << "return _ctx;" << endl;
	DEL_TAB;
	s << TAB << "}" <<endl;
	s << endl;

	//setServerEncoding
	s << TAB << "protected String sServerEncoding = \"" << _sCharset << "\";" << endl;
	s << TAB << "public int setServerEncoding(String se){" << endl;
	INC_TAB;
	s << TAB << "sServerEncoding = se;" << endl;
	s << TAB << "return 0;" << endl;
	DEL_TAB;
	s << TAB << "}" <<endl;
	s << endl;

	string sParams;
	vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();
	std::sort(vOperation.begin(), vOperation.end(), SortOperation());

	//��ɿͻ��˽ӿڵ�ʵ��
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();
		TypePtr retTypePtr = vOperation[i]->getReturnPtr()->getTypePtr();

		//1.��contextͬ������
		s << TAB << "public " << tostr(retTypePtr) << " "
		<< vOperation[i]->getId() << "(" ;
		sParams = "";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			sParams += generateJava(vParamDecl[j]) + ", ";
		}
		s << taf::TC_Common::trimright(sParams, ", ", false) << ")" << endl;

		s << TAB << "{" << endl;
		INC_TAB;
		if (_bWithAsync) s << TAB << "///@async" << endl;
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : "return ")
		<< vOperation[i]->getId() << "(";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << vParamDecl[j]->getTypeIdPtr()->getId() << ", ";
		}
		s << "__defaultContext());" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;


		//2.��contextͬ������
		s << TAB << "@SuppressWarnings(\"unchecked\")" << endl;
		string routekey = "";
		s << TAB << "public " << tostr(retTypePtr) << " " << vOperation[i]->getId() << "(" ;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << generateJava(vParamDecl[j]) << ", ";

			if (routekey.empty() && vParamDecl[j]->isRouteKey())
			{
				routekey = vParamDecl[j]->getTypeIdPtr()->getId();
			}
		}
		s << "java.util.Map __ctx)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << s_JCE_PACKAGE << ".JceOutputStream _os = new " << s_JCE_PACKAGE << ".JceOutputStream(0);" << endl;
		s << TAB << "_os.setServerEncoding(sServerEncoding);" << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!(vParamDecl[j]->isOut()))
			{
				s << TAB << "_os.write(" << vParamDecl[j]->getTypeIdPtr()->getId() << ", "
				<< taf::TC_Common::tostr(j+1) << ");" << endl;
			}
			else
			{
				bool isObj = isObjType(vParamDecl[j]->getTypeIdPtr()->getTypePtr());
				if (isObj)
				{
					s << TAB << "if(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value != null) " << endl;
					s << TAB << "{" << endl;
					INC_TAB;
				}
				s << TAB << "_os.write(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
				<< taf::TC_Common::tostr(j+1) << ");" << endl;
				if(isObj) {
					DEL_TAB;
					s << TAB << "}" << endl;
				}
			}
		}
		s << TAB << "byte[] _sBuffer = " << s_JCE_PACKAGE << ".JceUtil.getJceBufArray(_os.getByteBuffer());" << endl;
		s << endl;

		s << TAB << "java.util.HashMap<String, String> _mStatus = new java.util.HashMap<String, String>();" << endl;

		if (!routekey.empty())
		{
			s << TAB << "_mStatus.put(com.qq.taf.cnst.Const.STATUS_GRID_KEY, " << routekey << ");" << endl;
		}

		s << endl;
		if (_bWithAsync) s << TAB << "///@async" << endl;
		s << TAB << "byte[] _returnSBuffer = taf_invoke(\"" <<vOperation[i]->getId()
		<< "\", _sBuffer, __ctx, _mStatus);" << endl;
		s << endl;

		s << TAB << s_JCE_PACKAGE << ".JceInputStream _is = new " << s_JCE_PACKAGE << ".JceInputStream(_returnSBuffer);" << endl;
		s << TAB << "_is.setServerEncoding(sServerEncoding);" << endl;

		if (tostr(retTypePtr) != "void")
		{
			s << TAB << tostr(retTypePtr) << " _ret = " << toTypeInit(retTypePtr) << endl;
			s << generateDefautElem(retTypePtr, "_ret");
			s << TAB << "_ret = ("<< tostr(retTypePtr)
			<<") _is.read(_ret, 0, true);" << endl;
		}
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;

			TypePtr outTypePtr = vParamDecl[j]->getTypeIdPtr()->getTypePtr();
			string sOutValue = vParamDecl[j]->getTypeIdPtr()->getId() + ".value";
			s << TAB << sOutValue << " = " << toTypeInit(outTypePtr) << endl;
			s << generateDefautElem(outTypePtr, sOutValue);
			s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << ".value = ("
			<< tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _is.read("
			<< vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
			<< taf::TC_Common::tostr(j+1) << ", true);" << endl;
		}

		s << TAB <<((tostr(retTypePtr) != "void") ? "return _ret;" : "")<< endl;

		DEL_TAB;
		s << TAB << "}" << endl << endl;

		//3.��context�첽����
		s << TAB << "public void async_" << vOperation[i]->getId() << "(" ;
		s << _packagePrefix << nPtr->getId() << "." << pPtr->getId() << "PrxCallback callback";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;
			s << ", " << generateJava(vParamDecl[j]);
		}
		s << ")" << endl;

		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "async_" << vOperation[i]->getId() << "(";
		s << "callback";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;
			s << ", " << vParamDecl[j]->getTypeIdPtr()->getId();
		}
		s << ", __defaultContext());" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;

		//4.��context�첽����
		s << TAB << "public void async_" << vOperation[i]->getId() << "(" ;
		s << _packagePrefix << nPtr->getId() << "." << pPtr->getId() << "PrxCallback callback";
		s << ", ";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;

			if (routekey.empty() && vParamDecl[j]->isRouteKey())
			{
				routekey = vParamDecl[j]->getTypeIdPtr()->getId();
			}

			s << generateJava(vParamDecl[j]) << ", ";
		}
		s << "java.util.Map __ctx)" << endl;

		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << s_JCE_PACKAGE << ".JceOutputStream _os = new " << s_JCE_PACKAGE << ".JceOutputStream(0);" << endl;
		s << TAB << "_os.setServerEncoding(sServerEncoding);" << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (vParamDecl[j]->isOut())	continue;

			s << TAB << "_os.write(" << vParamDecl[j]->getTypeIdPtr()->getId() << ", "
			<< taf::TC_Common::tostr(j+1) << ");" << endl;
		}
		s << TAB << "byte[] _sBuffer = " << s_JCE_PACKAGE << ".JceUtil.getJceBufArray(_os.getByteBuffer());" << endl;
		s << endl;

		s << TAB << "java.util.HashMap<String, String> _mStatus = new java.util.HashMap<String, String>();" << endl;

		if (!routekey.empty())
		{
			s << TAB << "_mStatus.put(com.qq.taf.cnst.Const.STATUS_GRID_KEY, " << routekey << ");" << endl;
		}
		s << endl;
		s << TAB << "taf_invokeAsync(callback, \"" <<vOperation[i]->getId()
		<< "\", _sBuffer, __ctx, _mStatus);" << endl;
		s << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;
	}

	DEL_TAB;

	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + "PrxHelper.java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

string Jce2Java::generatePrxCallback(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public abstract class " << pPtr->getId()
	<< "PrxCallback extends " << s_PROXY_PACKAGE << ".ServantProxyCallback" << endl;
	s << TAB << "{" << endl;
	INC_TAB;

	string sParams;
	vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();
	std::sort(vOperation.begin(), vOperation.end(), SortOperation());

	//function string
	string sFuncArray =  "__" + nPtr->getId() + "__" + pPtr->getId() + "_all";
	s << TAB << "protected String " << sFuncArray << "[] = " << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		s << TAB << "\"" << vOperation[i]->getId() << "\""
		<<((i < vOperation.size()-1) ? "," : "" )<< endl;
	}
	DEL_TAB;
	s << TAB << "};" << endl;
	s << endl;

	//setServerEncoding

	s << TAB << "protected String sServerEncoding = \"" << _sCharset << "\";" << endl;

	s << TAB << "public int setServerEncoding(String se){" << endl;
	INC_TAB;
	s << TAB << "sServerEncoding = se;" << endl;
	s << TAB << "return 0;" << endl;
	DEL_TAB;
	s << TAB << "}" <<endl;
	s << endl;


	//��ɿͻ��˽ӿڵ�����
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();
		TypePtr retTypePtr = vOperation[i]->getReturnPtr()->getTypePtr();

		//callback_FUNCTION
		s << TAB << "public abstract void callback_" << vOperation[i]->getId() << "(";

		sParams = ((tostr(retTypePtr) == "void") ? "" : (tostr(retTypePtr) + " _ret, "));
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			sParams += tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + " "
					   + vParamDecl[j]->getTypeIdPtr()->getId() + ", ";
		}
		s << taf::TC_Common::trimright(sParams, ", ", false) << ");" << endl;
		s << endl;

		//callback_FUNCTION_exception
		s << TAB << "public abstract void callback_" << vOperation[i]->getId()
		<< "_exception(int _iRet);" << endl;

		s << endl;

	}

	//onDispatch
	s << TAB << "final public int _onDispatch(String sFuncName, com.qq.taf.ResponsePacket response)" << endl;

	s << TAB << "{" << endl;
	INC_TAB;

	s << endl;

	s << TAB << "int iPos = java.util.Arrays.binarySearch(" << sFuncArray << ", sFuncName);" << endl;
	s << TAB << "if(iPos < 0 || iPos >= " << vOperation.size() << ") return -1; //or throw execption?" << endl;

	s << TAB << "switch(iPos)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		s << TAB << "case " << taf::TC_Common::tostr(i) << ":" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << "if ( response.iRet != com.qq.taf.cnst.JCESERVERSUCCESS.value )" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "callback_" << vOperation[i]->getId() << "_exception(response.iRet);" << endl;
		s << TAB << "return response.iRet;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;

		s << TAB << s_JCE_PACKAGE << ".JceInputStream _is = new " << s_JCE_PACKAGE << ".JceInputStream(response.sBuffer);" << endl;
		s << TAB << "_is.setServerEncoding(sServerEncoding);" << endl;

		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();
		TypePtr retTypePtr = vOperation[i]->getReturnPtr()->getTypePtr();

		if (tostr(retTypePtr) != "void")
		{
			s << TAB << tostr(retTypePtr) << " _ret = " << toTypeInit(retTypePtr) << endl;
			s << generateDefautElem(retTypePtr, "_ret");
			s << TAB << "_ret = ("<< tostr(retTypePtr)
			<<") _is.read(_ret, 0, true);" << endl;
			s << endl;
		}

		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;

			s << TAB << tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) << " "
			<< vParamDecl[j]->getTypeIdPtr()->getId() << " = "
			<< toTypeInit(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) << endl;

			s << generateDefautElem(vParamDecl[j]->getTypeIdPtr()->getTypePtr(), vParamDecl[j]->getTypeIdPtr()->getId());

			s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << " = ("
			<< tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _is.read("
			<< vParamDecl[j]->getTypeIdPtr()->getId() << ", "
			<< taf::TC_Common::tostr(j+1) << ", true);" << endl;
			s << endl;
		}
		s << TAB << "callback_" << vOperation[i]->getId() << "(";
		sParams = ((tostr(retTypePtr) == "void") ? "" : "_ret, ");
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			sParams += vParamDecl[j]->getTypeIdPtr()->getId() + ", ";
		}
		s << taf::TC_Common::trimright(sParams, ", ", false) << ");" << endl;
		s << TAB << "break;" << endl;

		DEL_TAB;
		s << TAB << "}" << endl;
	}
	DEL_TAB;
	s << TAB << "}" << endl;
	s << TAB << "return 0;" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + "PrxCallback.java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

string Jce2Java::generateServant(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public abstract class " << pPtr->getId() << " extends " << s_SERVER_PACKAGE << ".Servant" << endl;
	s << TAB << "{" << endl;
	INC_TAB;

	s << TAB << "private  java.util.Map<String, " << s_SERVER_PACKAGE << ".ResponseHandler> responseHandlerMap=new java.util.HashMap<String, "
	<< s_SERVER_PACKAGE << ".ResponseHandler>();" << endl;
	s << endl;

	string sParams;
	vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();
	std::sort(vOperation.begin(), vOperation.end(), SortOperation());

	//���캯��
	s << TAB << "public " << pPtr->getId() << "()" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		s << TAB << "responseHandlerMap.put(\"" << vOperation[i]->getId() << "\", new " << s_SERVER_PACKAGE <<".ResponseHandler()" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "public " << s_JCE_PACKAGE << ".JceOutputStream handle(" << s_JCE_PACKAGE << ".JceInputStream _is, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "return response_" << vOperation[i]->getId() << "(_is, _jc);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;

		s << TAB << "public " << s_WUP_PACKAGE << ".UniAttribute handle_wup(" << s_WUP_PACKAGE << ".UniAttribute _unaIn, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "return response_" << vOperation[i]->getId() <<"_wup"<< "(_unaIn, _jc);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;

		DEL_TAB;
		s << TAB << "});" << endl;

	}

	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;

	//��Ӧ�ӿ�
	for (size_t i = 0; i < vOperation.size(); i++)
	{
		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();

		//ҵ����ĳ���ӿ�
		s << TAB << "public abstract " << tostr(vOperation[i]->getReturnPtr()->getTypePtr()) << " "
		<< vOperation[i]->getId() << "(" ;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << generateJava(vParamDecl[j]) << ", ";
		}
		s << s_SERVER_PACKAGE << ".JceCurrent _jc);" << endl;
		s << endl;


		//�����Ľӿ�
		s << TAB << "private " << s_JCE_PACKAGE << ".JceOutputStream response_" <<vOperation[i]->getId()
		<< "(" << s_JCE_PACKAGE << ".JceInputStream _is, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << s_JCE_PACKAGE << ".JceOutputStream _os = null;" << endl;
		s << TAB << "_is.setServerEncoding(getCharacterSet());" <<endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!(vParamDecl[j]->isOut()))
			{
				//  s << TAB << "is.read(" << vParamDecl[j]->getTypeIdPtr()->getId() << ", "
				//     << taf::TC_Common::tostr(j+1) << ", true);" << endl;
				s << TAB << generateJava(vParamDecl[j]) << " = " << toTypeInit(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) << endl;

				TypePtr outTypePtr = vParamDecl[j]->getTypeIdPtr()->getTypePtr();
				string sOutValue = vParamDecl[j]->getTypeIdPtr()->getId();
				//s << TAB << sOutValue << " = " << toTypeInit(outTypePtr) << endl;
				s << generateDefautElem(outTypePtr, sOutValue);

				s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << " = ("
				<< tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _is.read("
				<< vParamDecl[j]->getTypeIdPtr()->getId() << ", "
				<< taf::TC_Common::tostr(j+1) << ", true);" << endl;
			}
			else
			{
				//    s << TAB << "is.read(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
				//       << taf::TC_Common::tostr(j+1) << ", false);" << endl;

				s << TAB << generateJava(vParamDecl[j]) << " = new " << generateHolder(vParamDecl[j]) << "();" << endl;

				TypePtr outTypePtr = vParamDecl[j]->getTypeIdPtr()->getTypePtr();
				string sOutValue = vParamDecl[j]->getTypeIdPtr()->getId() + ".value";
				s << TAB << sOutValue << " = " << toTypeInit(outTypePtr) << endl;
				s << generateDefautElem(outTypePtr, sOutValue);

				s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << ".value = ("
				<< tostr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _is.read("
				<< vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
				<< taf::TC_Common::tostr(j+1) << ", false);" << endl;
			}

		}

		TypePtr retTypePtr = vOperation[i]->getReturnPtr()->getTypePtr();
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : (tostr(retTypePtr) + " _ret = "));
		s << vOperation[i]->getId() << "(";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << vParamDecl[j]->getTypeIdPtr()->getId() << ", ";
		}
		s << "_jc);" << endl;
		s << TAB << "if(_jc.isResponse())" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "_os = new " << s_JCE_PACKAGE << ".JceOutputStream(0);" << endl;
		s << TAB << "_os.setServerEncoding(getCharacterSet());" <<endl;
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : "_os.write(_ret, 0);") << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << TAB << "_os.write(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
			<< taf::TC_Common::tostr(j+1) << ");" << endl;
		}

		DEL_TAB;
		s << TAB << "}" << endl;

		s << TAB << "return _os;" << endl;

		DEL_TAB;
		s << TAB << "}" << endl;

		s << endl;
		//wup�����Ľӿ�
		s << TAB << "private " << s_WUP_PACKAGE << ".UniAttribute response_" <<vOperation[i]->getId()<<"_wup"
		<< "(" << s_WUP_PACKAGE << ".UniAttribute _unaIn, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << s_WUP_PACKAGE << ".UniAttribute _unaOut = null;" << endl;
		s << TAB << "_unaIn.setEncodeName(getCharacterSet());" << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!(vParamDecl[j]->isOut()))
			{
				//  s << TAB << "is.read(" << vParamDecl[j]->getTypeIdPtr()->getId() << ", "
				//     << taf::TC_Common::tostr(j+1) << ", true);" << endl;
				s << TAB << generateJava(vParamDecl[j]) << " = " << toTypeInit(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) << endl;

				TypePtr outTypePtr = vParamDecl[j]->getTypeIdPtr()->getTypePtr();
				string sOutValue = vParamDecl[j]->getTypeIdPtr()->getId();
				//s << TAB << sOutValue << " = " << toTypeInit(outTypePtr) << endl;
				s << generateDefautElem(outTypePtr, sOutValue);

				s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << " = ("
				<< toObjStr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _unaIn.getByClass(\""
				<< vParamDecl[j]->getTypeIdPtr()->getId() + "\", " + vParamDecl[j]->getTypeIdPtr()->getId() + ");" << endl;
			}
			else
			{
				//    s << TAB << "is.read(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
				//       << taf::TC_Common::tostr(j+1) << ", false);" << endl;

				s << TAB << generateJava(vParamDecl[j]) << " = new " << generateHolder(vParamDecl[j]) << "();" << endl;

				TypePtr outTypePtr = vParamDecl[j]->getTypeIdPtr()->getTypePtr();
				string sOutValue = vParamDecl[j]->getTypeIdPtr()->getId() + ".value";
				s << TAB << sOutValue << " = " << toTypeInit(outTypePtr) << endl;
				s << generateDefautElem(outTypePtr, sOutValue);

				s << TAB << vParamDecl[j]->getTypeIdPtr()->getId() << ".value = ("
				<< toObjStr(vParamDecl[j]->getTypeIdPtr()->getTypePtr()) + ")" << " _unaIn.getByClass(\""
				<< vParamDecl[j]->getTypeIdPtr()->getId() << "\", " << sOutValue << ", " << sOutValue << ");" << endl;
			}

		}

		//TypePtr retTypePtr = vOperation[i]->getReturnPtr()->getTypePtr();
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : (tostr(retTypePtr) + " _ret = "));
		s << vOperation[i]->getId() << "(";
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			s << vParamDecl[j]->getTypeIdPtr()->getId() << ", ";
		}
		s << "_jc);" << endl;
		s << TAB << "if(_jc.isResponse())" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "_unaOut = new " << s_WUP_PACKAGE << ".UniAttribute();" << endl;
		s << TAB << "_unaOut.setEncodeName(getCharacterSet());" << endl;
		s << TAB << "if(isWup3(_jc)) _unaOut.useVersion3();" << endl;
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : "_unaOut.put(\"\",_ret);") << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << TAB << "_unaOut.put(\""+vParamDecl[j]->getTypeIdPtr()->getId()+"\"," << vParamDecl[j]->getTypeIdPtr()->getId() << ".value);" << endl;
		}

		DEL_TAB;
		s << TAB << "}" << endl;

		s << TAB << "return _unaOut;" << endl;

		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;

		//�첽���صĽӿ�
		s << TAB << "protected void async_response_" <<vOperation[i]->getId() << "(" << s_SERVER_PACKAGE << ".JceCurrent _jc";
		s << ((tostr(retTypePtr) == "void") ? "" : (", " + tostr(retTypePtr) + " _ret"));
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << ", " << generateJava(vParamDecl[j]);
		}
		s << ")" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << "if(isWupRequest(_jc)){" << endl;
		INC_TAB;
		s << TAB << "async_response_"<<vOperation[i]->getId()<<"_wup(_jc";
		s << ((tostr(retTypePtr) == "void") ? "" : (", _ret"));
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << ", " << vParamDecl[j]->getTypeIdPtr()->getId();
		}
		s<<");"<< endl;
		DEL_TAB;
		s << TAB << "}else{" << endl;
		INC_TAB;
		s << TAB << s_JCE_PACKAGE << ".JceOutputStream _os = new " << s_JCE_PACKAGE << ".JceOutputStream(0);" << endl;
		s << TAB << "_os.setServerEncoding(getCharacterSet());" << endl;
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : "_os.write(_ret, 0);") << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << TAB << "_os.write(" << vParamDecl[j]->getTypeIdPtr()->getId() << ".value, "
			<< taf::TC_Common::tostr(j+1) << ");" << endl;
		}
		s << TAB << "sendResponseMessage(_jc, _os);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;

		DEL_TAB;
		s << TAB << "}" << endl;

		s << endl;


		//�첽���صĽӿ� wup
		s << TAB << "protected void async_response_" <<vOperation[i]->getId() << "_wup(" << s_SERVER_PACKAGE << ".JceCurrent _jc";
		s << ((tostr(retTypePtr) == "void") ? "" : (", " + tostr(retTypePtr) + " _ret"));
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << ", " << generateJava(vParamDecl[j]);
		}
		s << ")" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << s_WUP_PACKAGE << ".UniAttribute _unaOut = new " << s_WUP_PACKAGE << ".UniAttribute();" << endl;
		s << TAB << "_unaOut.setEncodeName(getCharacterSet());"<< endl;
		s << TAB << "if(isWup3(_jc)) _unaOut.useVersion3();" << endl;
		s << TAB << ((tostr(retTypePtr) == "void") ? "" : "_unaOut.put(\"\",_ret);") << endl;
		for (size_t j = 0; j < vParamDecl.size(); j++)
		{
			if (!vParamDecl[j]->isOut()) continue;
			s << TAB << "_unaOut.put(\""+vParamDecl[j]->getTypeIdPtr()->getId()+"\"," << vParamDecl[j]->getTypeIdPtr()->getId() << ".value);" << endl;
		}
		s << TAB << "sendResponseMessage_wup(_jc, _unaOut);" << endl;

		DEL_TAB;
		s << TAB << "}" << endl;

		s << endl;
	}

//    s << TAB << "@Override" << endl;
	s << TAB << "protected " << s_JCE_PACKAGE << ".JceOutputStream doResponse(String funcName, "
	<< s_JCE_PACKAGE << ".JceInputStream _is, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "return responseHandlerMap.get(funcName).handle(_is, _jc);" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;

	//for wup
//    s << TAB << "@Override" << endl;
	s << TAB << "protected " << s_WUP_PACKAGE << ".UniAttribute doResponse_wup(String funcName, "
	<< s_WUP_PACKAGE << ".UniAttribute _unaIn, " << s_SERVER_PACKAGE << ".JceCurrent _jc)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "return responseHandlerMap.get(funcName).handle_wup(_unaIn, _jc);" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;

	DEL_TAB;
	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + ".java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

string Jce2Java::generateConstructor(const TypePtr& vType) const {
	stringstream s;

	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(vType);
	if (bPtr) {
		switch (bPtr->kind()) {
		case Builtin::KindBool: {
			s << "Boolean.valueOf(false)";
		} break;
		case Builtin::KindByte: {
			s << "Byte.valueOf((byte)0)";
		} break;
		case Builtin::KindShort: {
			s << "Short.valueOf((short)0)";
		} break;
		case Builtin::KindInt: {
			s << "Integer.valueOf(0)";
		} break;
		case Builtin::KindLong: {
			s << "Long.valueOf(0)";
		} break;
		case Builtin::KindFloat : {
			s << "Float.valueOf((float)0)";
		} break;
		case Builtin::KindDouble : {
			s << "Double.valueOf((double)0)";
		} break;
		default:{
			s << "new " << toObjStr(vType) << "()";
		} break;
		}
	} else {
		s << "new " << toObjStr(vType) << "()";
	}

	return s.str();
}

string Jce2Java::generateAndroidJavaParams(
		const vector<ParamDeclPtr>& vParamDecl
		, bool needParamType
		, bool needOutParam) const {
	ostringstream s;

	bool needDot = false;
	for (size_t index = 0; index < vParamDecl.size(); ++index) {
		ParamDeclPtr paramDeclPtr = vParamDecl[index];

		if (!paramDeclPtr->isOut()) {
			if (!needDot) {
				needDot = true;
			} else {
				s << ", ";
			}
			if (needParamType) {
				s << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr()) << " ";
			}
			s << "in" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId());
		} else if (needOutParam){
			if (!needDot) {
				needDot = true;
			} else {
				s << ", ";
			}
			if (needParamType) {
				s << "OutWrapper<" << toObjStr(paramDeclPtr->getTypeIdPtr()->getTypePtr()) << "> ";
			}
			s<< "out" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId());
		}
	}
	return s.str();
}

string Jce2Java::generateAndroidStub(const InterfacePtr &pPtr, const NamespacePtr &nPtr) const {
	ostringstream s;

	s << g_parse->printHeaderRemark();
	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;

	s << endl;
	s << TAB << "import java.util.concurrent.Semaphore;" << endl;
	s << TAB << "import qrom.component.wup.QRomWupConstants.WUP_ERROR_CODE;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.AsyncWupMethod;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.AsyncWupOption;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.OutWrapper;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.WupBaseResult;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.WupException;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.WupHandle;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.WupOption;" << endl;
	s << TAB << "import qrom.component.wup.apiv2.WupOption.WupType;" << endl;
	s << TAB << "import com.qq.jce.wup.UniPacket;" << endl;
	s << endl;


	s << TAB << "public class " << firstCharUpper(pPtr->getId()) << "StubAndroid {" << endl << endl;
	INC_TAB;

	s << TAB << "private String mServantName;" << endl;

	s << TAB << "public " << firstCharUpper(pPtr->getId()) << "StubAndroid(String servantName) {" << endl;
	INC_TAB;
	s << TAB << "this.mServantName = servantName;" << endl;
	DEL_TAB;
	s << TAB << "}" << endl << endl;

	s << TAB << "public String getServantName() {" << endl;
	INC_TAB;
	s << TAB << "return mServantName;" << endl;
	DEL_TAB;
	s << TAB << "}";
	s << endl << endl;

	string sParams;
	vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();
	std::sort(vOperation.begin(), vOperation.end(), SortOperation());

	for (size_t i = 0; i < vOperation.size(); i++)
	{
		vector<ParamDeclPtr>& vParamDecl = vOperation[i]->getAllParamDeclPtr();

		TypeIdPtr& returnType = vOperation[i]->getReturnPtr();
		bool isReturnTypeVoid = false;
		if (!returnType->getTypePtr()) {
			isReturnTypeVoid = true;
		}


		// generate Result
		s << TAB << "public static class " << firstCharUpper(vOperation[i]->getId()) <<  "Result extends WupBaseResult " << "{" << endl;

		INC_TAB;

		s << TAB << "public " << firstCharUpper(vOperation[i]->getId()) <<  "Result() {" << endl;
		s << TAB << "}" << endl << endl;

		s << TAB << "public " << firstCharUpper(vOperation[i]->getId()) <<  "Result(int errorCode, String errorMsg) {" << endl;
		INC_TAB;
		s << TAB << "super(errorCode, errorMsg);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {

			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];

			string inOrOutPrefix = "in";
			if (paramDeclPtr->isOut()) {
				inOrOutPrefix = "out";
			}

			s << TAB << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr()) << " ";
			s << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ";" << endl;

			s << TAB << "public " << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr())
					 <<  " get" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
					 << "() {" << endl;
			INC_TAB;

			s << TAB << "return ";
			s << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ";" << endl;

			DEL_TAB;

			s << TAB << "}" << endl;


			s << TAB << "public " << firstCharUpper(vOperation[i]->getId()) <<  "Result"
					 << " set" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
					 << "(" << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr())
					 << " " << paramDeclPtr->getTypeIdPtr()->getId()
					 << ") {" << endl;
			INC_TAB;
			s << TAB << "this." << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
					 << " = " << paramDeclPtr->getTypeIdPtr()->getId() << ";" << endl;
			s << TAB << "return this;" << endl;
			DEL_TAB;

			s << TAB << "}" << endl << endl;

		}

		if (!isReturnTypeVoid) {
			s << TAB << tostr(returnType->getTypePtr()) << " ret;" << endl;
			s << TAB << "public " << tostr(returnType->getTypePtr())
					 << " getRet() {" << endl;
			INC_TAB;
			s << TAB << " return ret;" << endl;
			DEL_TAB;
			s << TAB << "}" << endl;
			s << TAB << "public " << firstCharUpper(vOperation[i]->getId()) <<  "Result"
					 << " setRet(" <<  tostr(returnType->getTypePtr()) << " ret) {" << endl;
			INC_TAB;
			s << TAB << "this.ret = ret;" << endl;
			s << TAB << "return this;" << endl;
			DEL_TAB;
			s << TAB << "}" << endl << endl;
		}

		DEL_TAB;

		s << TAB << "}" << endl << endl;

		// generate result end;

		// generate Callback interface;

		s << TAB << "public static interface I" << firstCharUpper(vOperation[i]->getId()) << "Callback {" << endl;
		INC_TAB;
		s << TAB << "public void on" << firstCharUpper(vOperation[i]->getId()) << "Callback("
				 <<  firstCharUpper(vOperation[i]->getId()) << "Result result);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		// generate Callback interface end;

		// generate BaseMethod

		s << TAB << "private static abstract class "
				 << firstCharUpper(vOperation[i]->getId()) << "BaseMethod extends AsyncWupMethod {" << endl;
		INC_TAB;

		s << TAB << "public " << firstCharUpper(vOperation[i]->getId())
				 << "BaseMethod(String servantName, AsyncWupOption wupOption) {" << endl;
		INC_TAB;
		s << TAB << "super(servantName, \"" << vOperation[i]->getId() << "\", wupOption);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {
			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];

			string inOrOutPrefix = "in";
			bool generateSet = true;
			if (paramDeclPtr->isOut()) {
				inOrOutPrefix = "out";
				generateSet = false;
			}

			s << TAB << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr()) << " ";
			s << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ";" << endl;

			s << TAB << "public " << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr())
					 <<  " get" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
					 << "() {" << endl;
			INC_TAB;

			s << TAB << "return ";
			s << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ";" << endl;

			DEL_TAB;
			s << TAB << "}" << endl;

			if (generateSet) {

				s << TAB << "public void set" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << "(" << tostr(paramDeclPtr->getTypeIdPtr()->getTypePtr())
						 << " " << paramDeclPtr->getTypeIdPtr()->getId()
						 << ") {" << endl;
				INC_TAB;
				s << TAB << "this." << inOrOutPrefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << " = " << paramDeclPtr->getTypeIdPtr()->getId() << ";" << endl;
				DEL_TAB;
				s << TAB << "}" << endl;
			}

			s << endl;
		}

		if (!isReturnTypeVoid) {
			s << TAB << tostr(returnType->getTypePtr()) << " ret;" << endl;
			s << TAB << "public " << tostr(returnType->getTypePtr())
					 << " getRet() {" << endl;
			INC_TAB;
			s << TAB << " return ret;" << endl;
			DEL_TAB;
			s << TAB << "}" << endl << endl;
		}

		s << TAB << "@Override" << endl;
		s << TAB << "protected void fillReqUniPacket(UniPacket reqPacket) {" << endl;
		INC_TAB;
		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {
			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];

			if (!paramDeclPtr->isOut()) {
				s << TAB << "reqPacket.put(\"" << paramDeclPtr->getTypeIdPtr()->getId()
						 << "\", in" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ");" << endl;
			}
		}
		DEL_TAB;
		s << TAB << "}" << endl <<endl;


		s << TAB << "@Override" << endl;
		s << TAB << "protected void fillFieldsByUniPacket(UniPacket respPacket) {" << endl;
		INC_TAB;
		s << TAB << "if (mWupOption.isUseUniPacketV3()) {" << endl;
		INC_TAB;

		if (!isReturnTypeVoid) {
			s << TAB << "ret = respPacket.getByClass(\"\", "
					 << generateConstructor(returnType->getTypePtr()) << ");" << endl;
		}

		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {
			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];
			if (paramDeclPtr->isOut()) {
				s << TAB << "out" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << " = respPacket.getByClass(\"" << paramDeclPtr->getTypeIdPtr()->getId()
						 << "\", " << generateConstructor(paramDeclPtr->getTypeIdPtr()->getTypePtr()) << ");" << endl;
			}
		}
		s << TAB << "return ;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;

		if (!isReturnTypeVoid) {
			s << TAB << "ret = (" << toObjStr(returnType->getTypePtr()) << ")respPacket.get(\"\");" << endl;
		}
		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {
			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];
			if (paramDeclPtr->isOut()) {
				s << TAB << "out" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << " = ("
						 << toObjStr(paramDeclPtr->getTypeIdPtr()->getTypePtr())
						 << ")respPacket.get(\"" << paramDeclPtr->getTypeIdPtr()->getId()
						 << "\");" << endl;
			}
		}

		DEL_TAB;
//
		s << TAB << "}" << endl <<endl;

		DEL_TAB;
		s << TAB << "}" << endl << endl;

		// generate BaseMethod end

		// generate AsyncMethod

		s << TAB << "private static class Async" << firstCharUpper(vOperation[i]->getId())
				 << "Method extends " << firstCharUpper(vOperation[i]->getId()) << "BaseMethod {" << endl;
		INC_TAB;
		s << TAB << "private I" <<  firstCharUpper(vOperation[i]->getId())
				 << "Callback m" << firstCharUpper(vOperation[i]->getId()) << "Callback;" << endl;

		s << TAB << "public Async" << firstCharUpper(vOperation[i]->getId()) << "Method(String servantName"
				 << ", AsyncWupOption wupOption"
				 << ", I" << firstCharUpper(vOperation[i]->getId())
				 << "Callback " << vOperation[i]->getId() << "Callback) {" << endl;
		INC_TAB;
		s << TAB << "super(servantName, wupOption);" << endl;
		s << TAB << "this.m" <<  firstCharUpper(vOperation[i]->getId()) << "Callback = "
				 << vOperation[i]->getId() << "Callback;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		s << TAB << "@Override" << endl;
		s << TAB << "protected void onError(int errorCode, String errorMsg) {" << endl;
		INC_TAB;
		s << TAB << firstCharUpper(vOperation[i]->getId())
				 << "Result result = new " <<  firstCharUpper(vOperation[i]->getId()) << "Result(errorCode, errorMsg);" << endl;
		s << TAB << "result.setRequestId(getRequestId());" << endl;
		s << TAB << "m" << firstCharUpper(vOperation[i]->getId())
				 << "Callback.on" << firstCharUpper(vOperation[i]->getId()) << "Callback(result);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		s << TAB << "@Override" << endl;
		s << TAB << "protected void onFinished() {" << endl;
		INC_TAB;
		s << TAB << firstCharUpper(vOperation[i]->getId())
				 << "Result result = new " <<  firstCharUpper(vOperation[i]->getId()) << "Result();" << endl;
		s << TAB << "result.setRequestId(getRequestId());" << endl;
		for (size_t paramIndex = 0; paramIndex < vParamDecl.size(); paramIndex++) {
			ParamDeclPtr paramDeclPtr = vParamDecl[paramIndex];

			s << TAB << "result.set" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
					 << "(get" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << "());" << endl;
		}
		if (!isReturnTypeVoid) {
			s << TAB << "result.setRet(getRet());" << endl;
		}
		s << TAB << "m" << firstCharUpper(vOperation[i]->getId())
				 << "Callback.on" << firstCharUpper(vOperation[i]->getId()) << "Callback(result);" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		DEL_TAB;
		s << TAB << "}" << endl << endl;

		// generate AsyncMethod end

		// generate SyncMethod
		s << TAB << "private static class Sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method extends " << firstCharUpper(vOperation[i]->getId()) << "BaseMethod {" << endl;
		INC_TAB;
		s << TAB << "private Semaphore mSemaphore;" << endl;
		s << TAB << "private WupException mWupException;" << endl;

		s << TAB << "public Sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method(String servantName, AsyncWupOption wupOption) {" << endl;
		INC_TAB;
		s << TAB << "super(servantName, wupOption);" << endl;
		s << TAB << "mSemaphore = new Semaphore(0);" << endl;
		DEL_TAB;
		s << TAB  << "}" << endl << endl;

		s << TAB << "public void waitResponse() throws InterruptedException {" << endl;
		INC_TAB;
		s << TAB << "mSemaphore.acquire();" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		s << TAB << "@Override" << endl;
		s << TAB << "protected void handleFinished() {" << endl;
		INC_TAB;
		s << TAB << "mSemaphore.release();" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		s << TAB << "@Override" << endl;
		s << TAB << "protected void handleError(final int errorCode, final String errorMsg) {" << endl;
		INC_TAB;
		s << TAB << "mWupException = new WupException(errorCode, errorMsg);" << endl;
		s << TAB << "mSemaphore.release();" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		s << TAB << "public WupException getWupException() {" << endl;
		INC_TAB;
		s << TAB << "return mWupException;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		DEL_TAB;
		s << TAB << "}" << endl << endl;

		// generate SyncMethod end

		// async api
		string param_string = generateAndroidJavaParams(vParamDecl, true, false);
		s << TAB << "public WupHandle async"
				 << firstCharUpper(vOperation[i]->getId())
				 << "(" << param_string;
		if (param_string.length() > 0) {
			s << ", ";
		}
		s << "I" << firstCharUpper(vOperation[i]->getId()) << "Callback "
				 << vOperation[i]->getId() << "Callback) throws WupException { " << endl;
		INC_TAB;

		param_string = generateAndroidJavaParams(vParamDecl, false, false);
		s << TAB << "return async"
				 <<  firstCharUpper(vOperation[i]->getId())
				 << "(" << param_string;
		if (param_string.length() > 0) {
			s << ", ";
		}
		s << vOperation[i]->getId() << "Callback, new AsyncWupOption(WupType.WUP_NORMAL_REQUEST));" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		param_string = generateAndroidJavaParams(vParamDecl, true, false);
		s << TAB << "public WupHandle async"
				 << firstCharUpper(vOperation[i]->getId())
				 << "(" <<  param_string;
		if (param_string.length() > 0) {
			s << ", ";
		}
		s << "I" << firstCharUpper(vOperation[i]->getId()) << "Callback "
				 << vOperation[i]->getId() << "Callback, AsyncWupOption option) throws WupException{" << endl;
		INC_TAB;

		s << TAB << "if (" << vOperation[i]->getId() << "Callback == null) {" << endl;
		INC_TAB;
		s << TAB << "throw new IllegalArgumentException(\"" << vOperation[i]->getId()
				 << "Callback should not be null\");" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << TAB << "if (option == null) {" << endl;
		INC_TAB;
		s << TAB << "throw new IllegalArgumentException(\"option should not be null\");" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		for (size_t index = 0; index < vParamDecl.size(); ++index) {
			ParamDeclPtr paramDeclPtr = vParamDecl[index];

			if (!paramDeclPtr->isOut() && isObjType(paramDeclPtr->getTypeIdPtr()->getTypePtr())) {
				s << TAB <<  "if (in" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << " == null) {" << endl;
				INC_TAB;
				s << TAB << "throw new IllegalArgumentException(\"in"
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << " should not be null\");" << endl;
				DEL_TAB;
				s << TAB << "}" << endl << endl;
			}

		}

		s << TAB << "Async" <<  firstCharUpper(vOperation[i]->getId()) << "Method "
				 << vOperation[i]->getId() << "Method = new Async"
				 << firstCharUpper(vOperation[i]->getId())
				 << "Method(getServantName(), option, " << vOperation[i]->getId() << "Callback);" << endl;

		for (size_t index = 0; index < vParamDecl.size(); ++index) {
			ParamDeclPtr paramDeclPtr = vParamDecl[index];

			if (!paramDeclPtr->isOut()) {
				s << TAB << vOperation[i]->getId() << "Method.set"
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << "(in" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << ");" << endl;
			}

		}

		s << TAB << vOperation[i]->getId() << "Method.start();" << endl;
		s << TAB << "return new WupHandle(" <<  vOperation[i]->getId() << "Method);" << endl;

		DEL_TAB;
		s << TAB << "}" << endl << endl;

		// async api end

		// sync api
		s << TAB << "public " << tostr(returnType->getTypePtr())
				 << " " << vOperation[i]->getId() << "("
				 << generateAndroidJavaParams(vParamDecl, true, true)
				 << ") throws WupException {"  << endl;
		INC_TAB;

		param_string = generateAndroidJavaParams(vParamDecl, false, true);
		s << TAB << "return "
				 << vOperation[i]->getId()
				 << "(" << param_string;
		if (param_string.length() > 0) {
			s << ", ";
		}
		s << "new WupOption(WupOption.WupType.WUP_NORMAL_REQUEST));" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		param_string = generateAndroidJavaParams(vParamDecl, true, true);
		s << TAB << "public " << tostr(returnType->getTypePtr())
				 << " " << vOperation[i]->getId() << "("
				 << param_string;
		if (param_string.length() > 0) {
			s << ", ";
		}
		s << "WupOption option"
				 << ") throws WupException {"  << endl;
		INC_TAB;
		s << TAB << "if (option == null) {" << endl;
		INC_TAB;
		s << TAB << "throw new IllegalArgumentException(\"option should not be null\");" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		for (size_t index = 0; index < vParamDecl.size(); ++index) {
			ParamDeclPtr paramDeclPtr = vParamDecl[index];

			string prefix = "in";
			if (paramDeclPtr->isOut()) {
				prefix = "out";
			}
			if (isObjType(paramDeclPtr->getTypeIdPtr()->getTypePtr())) {
				s << TAB <<  "if (" << prefix << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << " == null) {" << endl;
				INC_TAB;
				s << TAB << "throw new IllegalArgumentException(\"" << prefix
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId()) << " should not be null\");" << endl;
				DEL_TAB;
				s << TAB << "}" << endl << endl;
			}

		}

		s << TAB << "Sync" << firstCharUpper(vOperation[i]->getId())
				<< "Method " << "sync" << firstCharUpper(vOperation[i]->getId())
				<< "Method = " << "new Sync"
				<< firstCharUpper(vOperation[i]->getId())
				<< "Method(getServantName(), new AsyncWupOption(option, null));"
				<< endl;

		for (size_t index = 0; index < vParamDecl.size(); ++index) {
			ParamDeclPtr paramDeclPtr = vParamDecl[index];

			if (!paramDeclPtr->isOut()) {
				s << TAB << "sync" << firstCharUpper(vOperation[i]->getId())
						 << "Method.set"
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << "(in"
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << ");" << endl;
			}

		}

		s << TAB << "sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method.start();" << endl << endl;
		s << TAB << "try {" << endl;
		INC_TAB;
		s << TAB << "sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method.waitResponse();" << endl;
		DEL_TAB;
		s << TAB << "} catch (InterruptedException e) {" << endl;
		INC_TAB;
		s << TAB << "throw new WupException(WUP_ERROR_CODE.WUP_INNER_ERROR, e.getMessage());"
				 << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;
		s << TAB << "if (sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method.getWupException() != null) {" << endl;
		INC_TAB;
		s << TAB << "throw sync" << firstCharUpper(vOperation[i]->getId())
				 << "Method.getWupException();" << endl;
		DEL_TAB;
		s << TAB << "}" << endl << endl;

		for (size_t index = 0; index < vParamDecl.size(); ++index) {
			ParamDeclPtr paramDeclPtr = vParamDecl[index];

			if (paramDeclPtr->isOut()) {
				s << TAB << "out" << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						 << ".setOut"
						 << "(sync" << firstCharUpper(vOperation[i]->getId()) << "Method.get"
						 << firstCharUpper(paramDeclPtr->getTypeIdPtr()->getId())
						<< "());" << endl;
			}
		}

		if (!isReturnTypeVoid) {
			s << TAB << "return sync" << firstCharUpper(vOperation[i]->getId())
					 << "Method.getRet();" << endl;
		}

		DEL_TAB;
		s << TAB << "}" << endl << endl;
		// sync api end
	}

	DEL_TAB;
	s << TAB << "}" << endl << endl;


	string fileJava = getFilePath(nPtr->getId()) + pPtr->getId() + "StubAndroid.java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}



string Jce2Java::generateHolder(const StructPtr &pPtr, const NamespacePtr &nPtr) const
{
	if (_bHolder == false) return "";

	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public final class " << pPtr->getId() << "Holder" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "public " << pPtr->getId() << "Holder()" << endl;
	s << TAB << "{" << endl;
	s << TAB << "}" << endl;
	s << TAB << "public " << pPtr->getId() << "Holder(" << pPtr->getId() << " value)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	s << TAB << "this.value = value;" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;
	s << TAB << "public " << pPtr->getId() << " value;" << endl;
	DEL_TAB;
	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + "Holder.java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

string Jce2Java::generateDefautElem(const TypePtr &pPtr, const string & sElemName) const
{
	static int iCount = 0;
	ostringstream s;

	if (sElemName.empty())
	{
		iCount++;
	}

	VectorPtr vPtr = VectorPtr::dynamicCast(pPtr);
	if (vPtr)
	{
		int iId = iCount;
		string sVar = sElemName.empty() ? "__var_" + taf::TC_Common::tostr(iId) : sElemName;
		if (sElemName.empty())
		{
			s << TAB << toObjStr(vPtr) << " " << sVar
			<< " = " << toTypeInit(vPtr) << endl;
		}

		BuiltinPtr bPtr = BuiltinPtr::dynamicCast(vPtr->getTypePtr());
		if (bPtr && bPtr->kind() == Builtin::KindByte )
		{
			iCount++;
			s << TAB << "byte __var_" << taf::TC_Common::tostr(iCount)
			<< " = 0;"  << endl;

			s << TAB << "((" << tostr(vPtr->getTypePtr()) << "[])" << sVar
			<< ")[0] = __var_" << taf::TC_Common::tostr(iId+1) << ";" << endl;
		}
		else if (_bForceArray)
		{
			s << generateDefautElem(vPtr->getTypePtr(), "");
			s << TAB << "((" << tostr(vPtr->getTypePtr()) << "[])" << sVar
			<< ")[0] = __var_" << taf::TC_Common::tostr(iId+1) << ";" << endl;
		}
		else
		{
			s << generateDefautElem(vPtr->getTypePtr(), "");
			s<<  TAB <<"(("<<tostr(pPtr)<<")"<< sVar <<").add(__var_"<< taf::TC_Common::tostr(iId+1)<<");"<<endl;
		}

		return s.str();
	}

	MapPtr mPtr = MapPtr::dynamicCast(pPtr);
	if (mPtr)
	{
		int iId = iCount;
		string sVar = sElemName.empty() ? "__var_" + taf::TC_Common::tostr(iId) : sElemName;

		if (sElemName.empty())
		{
			s << TAB << toObjStr(mPtr) << " " << sVar
			<< " = " << toTypeInit(mPtr) << endl;
		}

		s << generateDefautElem(mPtr->getLeftTypePtr(), "");
		int iIdRight = iCount+1;
		s << generateDefautElem(mPtr->getRightTypePtr(), "");

		s << TAB << sVar << ".put(__var_" << taf::TC_Common::tostr(iId+1) << ", __var_"
		<< taf::TC_Common::tostr(iIdRight) << ");" << endl;

		return s.str();
	}

	//�׸���map��vector���ã��������Ĭ��Ԫ��
	if (!sElemName.empty())
	{
		return s.str();
	}

	BuiltinPtr bPtr = BuiltinPtr::dynamicCast(pPtr);
	if (bPtr)
	{
		s << TAB << toObjStr(bPtr) << " __var_" << taf::TC_Common::tostr(iCount)
		<< " = " << toTypeInit(bPtr) << endl;
		return s.str();
	}

	StructPtr sPtr = StructPtr::dynamicCast(pPtr);
	if (sPtr)
	{
		s << TAB << toObjStr(sPtr) << " __var_" << taf::TC_Common::tostr(iCount)
		<< " = " << toTypeInit(sPtr) << endl;
		return s.str();
	}

	EnumPtr ePtr = EnumPtr::dynamicCast(pPtr);
	if (ePtr)
	{
		s << TAB << toObjStr(ePtr) << " __var_" << taf::TC_Common::tostr(iCount)
		<< " = " << toTypeInit(ePtr) << endl;
		return s.str();
	}

	return s.str();
}


/******************************StructPtr***************************************/
string Jce2Java::generateJava(const StructPtr &pPtr, const NamespacePtr &nPtr) const
{
	//���Holder��
	generateHolder(pPtr, nPtr);

	ostringstream s;
	s << g_parse->printHeaderRemark();

	vector<string> key = pPtr->getKey();
	vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	bool bHasImpPrefix = false;

	if (_bWithWsp)
	{
		s << TAB << "import java.util.HashMap"  << ";" << endl;
		s << endl;

		s << TAB << "import "<< s_WSP_PACKAGE << ";" << endl;
		s << endl;
	}

	s << TAB << "public final class " << pPtr->getId() << " extends " << s_JCE_PACKAGE << ".JceStruct";
	if (_bWithWsp)
	{
		s << " implements WspStruct";
		bHasImpPrefix = true;
	}

	if (key.size() > 0)
	{
		if(bHasImpPrefix)
		{
			s << ", Comparable<" << pPtr->getId() << ">";
		}
		else
		{
			s << " implements Comparable<" << pPtr->getId() << ">";
			bHasImpPrefix = true;
		}
	}

	if(!_bWithCompact)
	{
		if(bHasImpPrefix)
		{
			s << ", java.lang.Cloneable";
		}
		else
		{
			s << " implements java.lang.Cloneable";
		}
	}
	s << endl;
	s << TAB << "{" << endl;
	INC_TAB;

	if(!_bWithCompact)
	{
		//��ɽṹ���
		s << TAB << "public String className()" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "return " << "\"" << nPtr->getId() << "." << pPtr->getId() << "\"" << ";" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;


		// ��ɴ��ǰ׺�Ľṹ��
		s << TAB << "public String fullClassName()" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "return " << "\"" << _packagePrefix << nPtr->getId() << "." << pPtr->getId() << "\"" << ";" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;
	}

	if (_bWithWsp)
	{
		s << TAB <<"private HashMap<String,Object> arr = new HashMap<String,Object>();"<<endl;
	}

	//�����Ա����
	for (size_t i = 0; i < member.size(); i++)
	{
		string sDefalut;
		if (member[i]->hasDefault())
		{
			BuiltinPtr bPtr  = BuiltinPtr::dynamicCast(member[i]->getTypePtr());
			EnumPtr    ePtr  = EnumPtr::dynamicCast(member[i]->getTypePtr());
			//stringת��
			if (bPtr && bPtr->kind() == Builtin::KindString)
			{
				sDefalut = taf::TC_Common::replace(member[i]->def(), "\"", "\\\"");
				//sDefalut = " = (" + tostr(member[i]->getTypePtr()) + ")\"" + sDefalut + "\"";
				sDefalut = " = \"" + sDefalut + "\"";
			}
			else if (ePtr)
			{

				std::string sdef = member[i]->def();
				std::string::size_type pos = sdef.rfind("::");
				if (pos != std::string::npos && pos + 2 < sdef.size())
				{
					sdef = sdef.substr(pos + 2);
				}

				if(_bEnumCompact)
				{
					sDefalut = " = " + _packagePrefix + taf::TC_Common::replace(ePtr->getSid(), "::", ".") + "._" + sdef;
				}
				else
				{
					sDefalut = " = " + _packagePrefix + taf::TC_Common::replace(ePtr->getSid(), "::", ".") + "." + sdef + ".value()";
				}
			}
			else
			{
				//sDefalut = " = (" + tostr(member[i]->getTypePtr()) + ")" + member[i]->def();
				sDefalut = " = " + member[i]->def();
			}
			s << TAB << "public " << tostr(member[i]->getTypePtr()) << " "<< member[i]->getId() << sDefalut << ";" << endl;
		}
		else
		{
			sDefalut = " = " + toTypeInit(member[i]->getTypePtr());
			//s << TAB << "public " << tostr(member[i]->getTypePtr()) << " "<< member[i]->getId() << sDefalut << endl;
			BuiltinPtr bPtr  = BuiltinPtr::dynamicCast(member[i]->getTypePtr());
			EnumPtr ePtr = EnumPtr::dynamicCast(member[i]->getTypePtr());
			if ((!bPtr && !ePtr) || (bPtr && bPtr->kind() == Builtin::KindString))
			{
				s << TAB << "public " << tostr(member[i]->getTypePtr()) << " "<< member[i]->getId() <<" = null;" << endl;
			}
			else
			{
				s << TAB << "public " << tostr(member[i]->getTypePtr()) << " "<< member[i]->getId() << sDefalut << endl;
			}
		}
		s << endl;
	}

	if(!_bWithCompact)
	{
		//��Ա����get/set for java bean
		for (size_t i = 0; i < member.size(); i++)
		{
			string sName = "";
			//֧��javabean�淶,���ҵڶ����ַ��Ǵ�д
			if (_bWithJbr && member[i]->getId()[1] >= 'A' && member[i]->getId()[1] <= 'Z' )
			{
				sName = member[i]->getId();
			}
			else
			{
				sName = taf::TC_Common::upper(member[i]->getId().substr(0, 1)) + member[i]->getId().substr(1);
			}
			s << TAB << "public " << tostr(member[i]->getTypePtr()) << " get" << sName
			 << "()" << endl;
			s << TAB << "{" << endl;
			INC_TAB;
			s << TAB << "return " <<  member[i]->getId() << ";" << endl;
			DEL_TAB;
			s << TAB << "}" << endl;
			s << endl;

			s << TAB << "public void " << " set" << sName << "(" << tostr(member[i]->getTypePtr()) << " " << member[i]->getId() << ")" << endl;
			s << TAB << "{" << endl;
			INC_TAB;
			s << TAB << "this." <<  member[i]->getId() << " = " <<  member[i]->getId() << ";" << endl;
			if (_bWithWsp)
			{
				s << TAB <<"arr.put(\""<<member[i]->getId()<<"\","<< member[i]->getId()<<");"<<endl;
			}
			DEL_TAB;
			s << TAB << "}" << endl;
			s << endl;
		}
	}

	//(constructor)()
	s << TAB << "public " << pPtr->getId() << "()" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	if (_bWithWsp)
	{
		s << TAB << "buildIndex();" << endl;
	}
	else
	{
/*
 * �޲εĹ��캯���õ��ó�Ա���÷���
		for (size_t i = 0; i < member.size(); i++)
		{
			string sName = "";
			//֧��javabean�淶,���ҵڶ����ַ��Ǵ�д
			if (_bWithJbr && member[i]->getId()[1] >= 'A' && member[i]->getId()[1] <= 'Z' )
			{
				sName = member[i]->getId();
			}
			else
			{
				sName = taf::TC_Common::upper(member[i]->getId().substr(0, 1)) + member[i]->getId().substr(1);
			}
			s << TAB <<"set" << sName  << "(" << member[i]->getId() << ");" << endl;
		}
*/
	}

	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;

	//(constructor)(...)
	s << TAB << "public " << pPtr->getId() << "(";
	for (size_t i = 0; i < member.size(); i++)
	{
		s << tostr(member[i]->getTypePtr()) << " " << member[i]->getId()
		<< ( (i < member.size() - 1) ? ", " : "" );
	}
	s << ")" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	for (size_t i = 0; i < member.size(); i++)
	{
		s << TAB <<"this." << member[i]->getId()  << " = " << member[i]->getId() << ";" << endl;
	}
	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;

	//compareTo()
	if (key.size() > 0)
	{
		s << TAB << "public int compareTo(" << pPtr->getId() << " o)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << "int[] r = " << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		for (size_t i = 0; i < key.size(); i++)
		{
			s << TAB << s_JCE_PACKAGE << ".JceUtil.compareTo(" << key[i] << ", o."
			<< key[i] << ")" << ( (i < key.size() - 1) ? ", " : "" ) << endl;
		}
		DEL_TAB;
		s << TAB << "};" << endl;

		s << TAB << "for(int i = 0; i < r.length; ++i)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "if(r[i] != 0) return r[i];" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << TAB << "return 0;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;
	}

	if(!_bWithCompact)
	{
		//equals()
		s << TAB << "public boolean equals(Object o)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;

		s << TAB << "if(o == null)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "return false;" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;

		s << TAB << pPtr->getId() << " t = (" << pPtr->getId() << ") o;" << endl;
		s << TAB << "return (" << endl;

		INC_TAB;

		//������key
		if (key.size() > 0)
		{
			for (size_t i = 0; i < key.size(); i++)
			{
				s << TAB << s_JCE_PACKAGE << ".JceUtil.equals(" << key[i] << ", t." << key[i] << ")"
				<< ( (i < key.size() - 1) ? " && " : " );" ) << endl;
			}
		}
		else
		{
			//ʹ������Ԫ�رȽ�
			for (size_t i = 0; i < member.size(); i++)
			{
				s << TAB << s_JCE_PACKAGE << ".JceUtil.equals(" << member[i]->getId() << ", t." << member[i]->getId() << ")"
				<< ( (i < member.size() - 1) ? " && " : " );" ) << endl;
			}
		}
		DEL_TAB;

		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;
	}

	if(!_bWithCompact)
	{
		//hashCode()
		if (key.size() > 0)
		{
			s << TAB << "public int hashCode()" << endl;
			s << TAB << "{" << endl;
			INC_TAB;

			s << TAB << "int [] hc = { " << endl;
			INC_TAB;
			for (size_t i = 0; i < key.size(); i++)
			{
				s << TAB << s_JCE_PACKAGE << ".JceUtil.hashCode(" << key[i]
				<< ")" << ( (i < key.size() - 1) ? ", " : "" ) << endl;
			}
			DEL_TAB;
			s << TAB << "};" << endl;
			s << TAB << "return java.util.Arrays.hashCode(hc);" << endl;

			DEL_TAB;
			s << TAB << "}" << endl;
			s << endl;
		}
		else//����쳣����
		{
			s << TAB << "public int hashCode()" << endl;
			s << TAB << "{" << endl;
			INC_TAB;
			s << TAB << "try" << endl;
			s << TAB << "{" << endl;
			INC_TAB;
			s << TAB << "throw new Exception(\"Need define key first!\");" <<endl;
			DEL_TAB;
			s << TAB << "}" << endl;
			s << TAB << "catch(Exception ex)" <<endl;
			s << TAB << "{" << endl;
			INC_TAB;
			s << TAB << "ex.printStackTrace();" <<endl;
			DEL_TAB;
			s << TAB << "}" << endl;
			s << TAB << "return 0;" <<endl;
			DEL_TAB;
			s << TAB << "}" << endl;
		}
	}

	if(!_bWithCompact)
	{
		//clone()
		s << TAB << "public java.lang.Object clone()" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "java.lang.Object o = null;" <<endl;
		s << TAB << "try" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "o = super.clone();" <<endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << TAB << "catch(CloneNotSupportedException ex)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << "assert false; // impossible" << endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << TAB << "return o;" <<endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;
	}

	//writeTo()
    s << TAB << "public void writeTo(" << s_JCE_PACKAGE << ".JceOutputStream _os)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    for(size_t i = 0; i < member.size(); i++)
    {
        BuiltinPtr bPtr = BuiltinPtr::dynamicCast(member[i]->getTypePtr());
		EnumPtr ePtr    = EnumPtr::dynamicCast(member[i]->getTypePtr());
        if (!member[i]->isRequire() && ((!bPtr && !ePtr) || (bPtr && bPtr->kind() == Builtin::KindString)))
        {
            s << TAB << "if (null != " << member[i]->getId() << ")" << endl;
            s << TAB << "{" << endl;
            INC_TAB;
            s << TAB << "_os.write(" << member[i]->getId() << ", " << member[i]->getTag() << ");" << endl;
            DEL_TAB;
            s << TAB << "}" << endl;
        }
        else if(EnumPtr::dynamicCast(member[i]->getTypePtr()))
        {
            s << TAB << "_os.write(" << member[i]->getId() << ", " << member[i]->getTag() << ");" << endl;
        }
        else
        {
            //s << TAB << "_os.write(" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
            MapPtr mPtr = MapPtr::dynamicCast(member[i]->getTypePtr());
            VectorPtr vPtr = VectorPtr::dynamicCast(member[i]->getTypePtr());

            if (!_bCheckDefault || member[i]->isRequire() || (!member[i]->hasDefault() && !mPtr && !vPtr))
            {
                s << TAB << "_os.write("<< member[i]->getId() << ", " << member[i]->getTag() << ");" << endl;
            }
            else
            {
                string sDefault = member[i]->def();
                BuiltinPtr bPtr = BuiltinPtr::dynamicCast(member[i]->getTypePtr());

                if (bPtr && bPtr->kind() == Builtin::KindString)
                {
                    sDefault = "\"" + taf::TC_Common::replace(member[i]->def(), "\"", "\\\"") + "\"";
                }

                if (mPtr)
                {
                    s << TAB << "if (" << member[i]->getId() << ".size() > 0)" << endl;
                }
                else if (vPtr)
                {
                    BuiltinPtr newPtr = BuiltinPtr::dynamicCast(vPtr->getTypePtr());
                	if (newPtr && newPtr->kind() == Builtin::KindByte )
                	{
                		s << TAB << "if (" << member[i]->getId() << ".length > 0)" << endl;
                	}
                	else if (_bForceArray)
                	{
                		s << TAB << "if (" << member[i]->getId() << ".length > 0)" << endl;
                	}
                    else
                    {
                        s << TAB << "if (" << member[i]->getId() << ".size() > 0)" << endl;
                    }
                }
                else if(bPtr && bPtr->kind() == Builtin::KindString)
                {
                    s << TAB << "if (!(" << sDefault << ".equals(" << member[i]->getId() << ")))" << endl;
                }
                else
                {
                    s << TAB << "if (" << member[i]->getId() << " != " << sDefault << ")" << endl;
                }
                s << TAB << "{" << endl;
                INC_TAB;
                s << TAB << "_os.write("<< member[i]->getId() << ", " << member[i]->getTag() << ");" << endl;
                DEL_TAB;
                s << TAB << "}" << endl;
            }
        }
    }
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;

    for(size_t i = 0; i < member.size(); i++)
    {
        BuiltinPtr bPtr  = BuiltinPtr::dynamicCast(member[i]->getTypePtr());
        if(!bPtr)
        {
            s << TAB << "static " <<  tostr(member[i]->getTypePtr())<< " cache_" <<member[i]->getId() <<";"<< endl;
        }
    }
    s<<endl;
	//readFrom()
	s << TAB <<"public void readFrom(" << s_JCE_PACKAGE << ".JceInputStream _is)" << endl;
	s << TAB << "{" << endl;
	INC_TAB;
	for (size_t i = 0; i < member.size(); i++)
	{
		string prefix = "";
		BuiltinPtr bPtr  = BuiltinPtr::dynamicCast(member[i]->getTypePtr());
		EnumPtr ePtr = EnumPtr::dynamicCast(member[i]->getTypePtr());
		if (!bPtr && !ePtr)
		{
			prefix = "cache_";
			//����Ĭ��Ԫ������ʶ������
			s << TAB <<"if(null == "<<prefix<<member[i]->getId()<<")"<<endl;
			s << TAB <<"{"<<endl;
			INC_TAB;
			s << TAB <<prefix << member[i]->getId()<<" = " << toTypeInit(member[i]->getTypePtr()) << endl;
			s << generateDefautElem(member[i]->getTypePtr(), prefix+member[i]->getId());
			DEL_TAB;
			s << TAB <<"}"<<endl;
			//
			//s << TAB << member[i]->getId() << " = " << toTypeInit(member[i]->getTypePtr()) << endl;
			//s << generateDefautElem(member[i]->getTypePtr(), member[i]->getId());

		}

		//string���⴦��
		if (bPtr && bPtr->kind() == Builtin::KindString)
		{
			s<< TAB << "this." << member[i]->getId() << " = "
			<< " _is.readString(" << member[i]->getTag() << ", " << (member[i]->isRequire() ? "true" : "false") << ");" << endl;
		}
		else
		{
			s<< TAB << "this." << member[i]->getId()  << " = (" +tostr(member[i]->getTypePtr()) + ")"
			<< " _is.read(" << prefix+member[i]->getId()
			<< ", " << member[i]->getTag() << ", " << (member[i]->isRequire() ? "true" : "false") << ");" << endl;
		}
	}

	DEL_TAB;
	s << TAB << "}" << endl;
	s << endl;

	if(!_bWithCompact)
	{
		//display()
		s << TAB << "public void display(java.lang.StringBuilder _os, int _level)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << s_JCE_PACKAGE << ".JceDisplayer _ds = new " << s_JCE_PACKAGE << ".JceDisplayer(_os, _level);" << endl;
		for (size_t i = 0; i < member.size(); i++)
		{
			s << TAB << "_ds.display(" << member[i]->getId()
			<< ", \"" << member[i]->getId() << "\");" << endl;
		}
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;

		//displaySimple()
		s << TAB << "public void displaySimple(java.lang.StringBuilder _os, int _level)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB << s_JCE_PACKAGE << ".JceDisplayer _ds = new " << s_JCE_PACKAGE << ".JceDisplayer(_os, _level);" << endl;
		for (size_t i = 0; i < member.size(); i++)
		{
			if(i != member.size() - 1)
			{
				s << TAB << "_ds.displaySimple(" << member[i]->getId() << ", true);" << endl;
			}
			else
			{
				s << TAB << "_ds.displaySimple(" << member[i]->getId() << ", false);" << endl;
			}
		}
		DEL_TAB;
		s << TAB << "}" << endl;
		s << endl;
	}


	if (_bWithWsp)
	{
		s << TAB << "public Object getFieldByName(String name)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB <<"return arr.get(name);"<<endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s <<endl;

//        s << TAB <<"@Override"<<endl;
		s << TAB << "public boolean containField(String name)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB <<"return arr.containsKey(name);"<<endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s <<endl;

//        s << TAB <<"@Override"<<endl;
		s << TAB << "public void setFieldByName(String name, Object value)" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		s << TAB <<"throw new RuntimeException(this+\" can not support setFieldByName now\");"<<endl;
		DEL_TAB;
		s << TAB << "}" << endl;
		s <<endl;

//        s << TAB <<"@Override"<<endl;
		s << TAB << "public void buildIndex()" << endl;
		s << TAB << "{" << endl;
		INC_TAB;
		for (size_t i = 0; i < member.size(); i++)
		{
			string sName = "";
			//֧��javabean�淶,���ҵڶ����ַ��Ǵ�д
			if (_bWithJbr && member[i]->getId()[1] >= 'A' && member[i]->getId()[1] <= 'Z' )
			{
				sName = member[i]->getId();
			}
			else
			{
				sName = taf::TC_Common::upper(member[i]->getId().substr(0, 1)) + member[i]->getId().substr(1);
			}
			s << TAB <<"set" << sName << "(" << member[i]->getId() << ");" << endl;
		}
		DEL_TAB;
		s << TAB << "}" << endl;
	}

	DEL_TAB;
	s << TAB << "}" << endl;
	s<<endl;



	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + ".java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return s.str();
}

/******************************EnumPtr***************************************/
string Jce2Java::generateJava(const EnumPtr &pPtr, const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	s << TAB << "package " << _packagePrefix << nPtr->getId() << ";" << endl;
	s << endl;

	s << TAB << "public final class " << pPtr->getId() <<" implements java.io.Serializable"<< endl;
	s << TAB << "{" << endl;
	INC_TAB;

    if(_bEnumCompact)
    {
     	//��Ա����
    	vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();
        bool bFlag = false;
        string sValue;
        for (size_t i = 0; i < member.size(); i++)
        {
            s << TAB << "public static final int _" << member[i]->getId()<< " = ";
            if (member[i]->hasDefault())
            {
                bFlag = true;
                sValue = member[i]->def();
                s << sValue;
            }
            else
            {
                if (bFlag == true)
                {
                    assert(i>0);
                    string stem = taf::TC_Common::tostr(taf::TC_Common::strto<int>(sValue)+1);
                    sValue = stem;
                    s << stem;
                }
                else
                {
                    s << taf::TC_Common::tostr(i);
                }
            }
            s <<";"<<endl;
        }
    }
    else
    {
    	//��Ա����
    	vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();
    	s << TAB << "private static " << pPtr->getId()
    	<<"[] __values = new " << pPtr->getId()
    	<< "[" << taf::TC_Common::tostr(member.size()) << "];" << endl;
    	s << TAB << "private int __value;" << endl;

    	s << TAB << "private String __T = new String();" << endl;
    	s << endl;

    	bool bFlag = false;
    	string sValue;
    	for (size_t i = 0; i < member.size(); i++)
    	{
    		s << TAB << "public static final int _" << member[i]->getId()<< " = ";
    		if (member[i]->hasDefault())
    		{
    			bFlag = true;
    			sValue = member[i]->def();
    			s << sValue;
    		}
    		else
    		{
    			if (bFlag == true)
    			{
    				assert(i>0);
    				string stem = taf::TC_Common::tostr(taf::TC_Common::strto<int>(sValue)+1);
    				sValue = stem;
    				s << stem;
    			}
    			else
    			{
    				s << taf::TC_Common::tostr(i);
    			}
    		}
    		s <<";"<<endl;
    		/*s << TAB << "public static final " << pPtr->getId() << " "
    		<< member[i]->getId() << " = new " << pPtr->getId() << "(_"
    		<< member[i]->getId() << ",\""<<member[i]->getId()<<"\");" << endl;*/
            s << TAB << "public static final " << pPtr->getId() << " "
    		<< member[i]->getId() << " = new " << pPtr->getId() << "("<<i<<",_"
    		<< member[i]->getId() << ",\""<<member[i]->getId()<<"\");" << endl;
    	}
    	s << endl;


    	//convert(int)
    	s << TAB << "public static " << pPtr->getId() << " convert(int val)"<< endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "for(int __i = 0; __i < __values.length; ++__i)" << endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "if(__values[__i].value() == val)" << endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "return __values[__i];" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << TAB << "assert false;" << endl;
    	s << TAB << "return null;" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << endl;

    	//convert(String)
    	s << TAB << "public static " << pPtr->getId() << " convert(String val)"<< endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "for(int __i = 0; __i < __values.length; ++__i)" << endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "if(__values[__i].toString().equals(val))" << endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "return __values[__i];" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << TAB << "assert false;" << endl;
    	s << TAB << "return null;" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << endl;

    	//value()
    	s << TAB << "public int value()"<< endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "return __value;" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << endl;

    	//toString()
    	s << TAB << "public String toString()"<< endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "return __T;" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << endl;

    	//(constructor)(int)
    	s << TAB << "private " << pPtr->getId() << "(int index, int val, String s)" << endl;
    	s << TAB << "{" << endl;
    	INC_TAB;
    	s << TAB << "__T = s;" << endl;
    	s << TAB << "__value = val;" << endl;
    	s << TAB << "__values[index] = this;" << endl;
    	DEL_TAB;
    	s << TAB << "}" << endl;
    	s << endl;
    }
	DEL_TAB;
	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + pPtr->getId() + ".java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId()), 0755);
	taf::TC_File::save2file(fileJava, s.str());


	return s.str();
}

/******************************ConstPtr***************************************/
void Jce2Java::generateJava(const ConstPtr &pPtr,const NamespacePtr &nPtr) const
{
	ostringstream s;
	s << g_parse->printHeaderRemark();

	//const���͵İ���ŵ�һ��constĿ¼����
	s << TAB << "package " << _packagePrefix << nPtr->getId() << ".cnst" << ";" << endl;
	s << endl;

	s << TAB << "public interface " << pPtr->getTypeIdPtr()->getId() << endl;
	s << TAB << "{" << endl;
	INC_TAB;

	//TODO: Stringת�� / long ʶ�� / byte ��Χ
	if (pPtr->getConstTokPtr()->t == ConstTok::STRING)
	{
		string tmp = taf::TC_Common::replace(pPtr->getConstTokPtr()->v, "\"", "\\\"");
		s << TAB << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " value = \""
		<< tmp << "\";"<< endl;
	}
	else if (tostr(pPtr->getTypeIdPtr()->getTypePtr()) == "long")
	{
		s << TAB << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " value = "
		<< pPtr->getConstTokPtr()->v << "L;" << endl;
	}
	else
	{
		s << TAB << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " value = "
		<< pPtr->getConstTokPtr()->v << ";" << endl;
	}

	DEL_TAB;
	s << TAB << "}" << endl;

	string fileJava  = getFilePath(nPtr->getId()) + "/cnst/" + pPtr->getTypeIdPtr()->getId() + ".java";
	taf::TC_File::makeDirRecursive(getFilePath(nPtr->getId() + "/cnst/"), 0755);
	taf::TC_File::save2file(fileJava, s.str());

	return;
}

/******************************NamespacePtr***************************************/

void Jce2Java::generateJava(const NamespacePtr &pPtr) const
{
	vector<InterfacePtr>    &is    = pPtr->getAllInterfacePtr();
	vector<StructPtr>       &ss    = pPtr->getAllStructPtr();
	vector<EnumPtr>         &es    = pPtr->getAllEnumPtr();
	vector<ConstPtr>        &cs    = pPtr->getAllConstPtr();

	InterfaceAnalysis interfaceAnalysis;

	if (_bWithGenerateInterfaceDependencies) {
		interfaceAnalysis.analysis(is);
	}

	for (size_t i = 0; i < is.size(); i++)
	{
		generateJava(is[i], pPtr);
	}

	JceFilter romJceFilter;
	std::map<std::string, StructPtr> needGenerateStructMap;
	if (_bWithGenerateInterfaceDependencies) {
		if (_bWithFilterRomJce) {
			romJceFilter.filterStructs(
					interfaceAnalysis.getAllStructs(), needGenerateStructMap);
		} else {
			needGenerateStructMap = interfaceAnalysis.getAllStructs();
		}
	}

	for (size_t i = 0; i < ss.size(); i++)
	{
		generateJava(ss[i], pPtr);
		needGenerateStructMap.erase(ss[i]->getSid());
	}
	for (std::map<std::string, StructPtr>::const_iterator
			it = needGenerateStructMap.begin(); it != needGenerateStructMap.end(); ++it) {
		generateJava(it->second, pPtr);
	}

	std::map<std::string, EnumPtr> needGenerateEnumMap;
	if (_bWithGenerateInterfaceDependencies) {
		if (_bWithFilterRomJce) {
			romJceFilter.filterEnums(
							interfaceAnalysis.getAllEnums(), needGenerateEnumMap);
		} else {
			needGenerateEnumMap = interfaceAnalysis.getAllEnums();
		}
	}

	for (size_t i = 0; i < es.size(); i++)
	{
		generateJava(es[i], pPtr);
		needGenerateEnumMap.erase(es[i]->getSid());
	}
	for (std::map<std::string, EnumPtr>::const_iterator
				it = needGenerateEnumMap.begin(); it != needGenerateEnumMap.end(); ++it) {
		generateJava(it->second, pPtr);
	}

	for (size_t i = 0; i < cs.size(); i++)
	{
		generateJava(cs[i], pPtr);
	}


	return;
}


/******************************Jce2Java***************************************/

void Jce2Java::generateJava(const ContextPtr &pPtr) const
{
	ostringstream s;

	vector<NamespacePtr> namespaces = pPtr->getNamespaces();

	for (size_t i = 0; i < namespaces.size(); i++)
	{
		generateJava(namespaces[i]);
	}

}

void Jce2Java::createFile(const string &file)
{
	std::vector<ContextPtr> contexts = g_parse->getContexts();
	for (size_t i = 0; i < contexts.size(); i++)
	{
		if (file == contexts[i]->getFileName())
		{
			generateJava(contexts[i]);
		}
	}
}

void Jce2Java::setBaseDir(const string &dir)
{
	_baseDir = dir;
}


void Jce2Java::setBasePackage(const string &prefix)
{
	_packagePrefix = prefix;
	if (_packagePrefix.length() != 0
		&& _packagePrefix.substr(_packagePrefix.length()-1, 1) != ".")
	{
		_packagePrefix += ".";
	}
}


string Jce2Java::getFilePath(const string &ns) const
{
	return _baseDir + "/" + taf::TC_Common::replace(_packagePrefix, ".", "/") + "/" + ns + "/";
}




