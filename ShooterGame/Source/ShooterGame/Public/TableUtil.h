#pragma once
#include "lua_tinker.h"
#include <typeinfo>
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TableUtil.generated.h"
DECLARE_LOG_CATEGORY_EXTERN(LuaLog, Log, All);
#define LuaDebug 1
using namespace std;
using luafunc = int( struct lua_State* );

struct EnumItem
{
	const char* key;
	const int32 value;
};

struct lua_class {
	void *p;
	const char* name;
};

class FLuaGcObj : FGCObject
{
public:
	TSet<UObject*> objs;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AllowEliminatingReferences(false);
		for (auto Object : objs)
		{
			Collector.AddReferencedObject(Object);
		}
		Collector.AllowEliminatingReferences(true);
	}

};


UCLASS(MinimalAPI)
class UTableUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	static lua_State* L;

public:
	 UTableUtil();
	static void addmodule(const char* classname);

	UFUNCTION(BlueprintCallable, Category = "TableUtil", meta = (DisplayName = "Initlua"))
	static void init();
	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void beginplay();
	static void openmodule(const char* classname);
	static void closemodule();
	static void addfunc(const char* classname, luafunc f);
	static void initmeta();
	static void setmeta(const char* classname, int index);

	static void* tousertype(const char* classname, int i);
	static int toint(int i);
	static void loadlib(const luaL_Reg funclist[], const char* classname);
	static void loadEnum(const EnumItem list[], const char* enumname);
	static void addutil(const luaL_Reg funclist[], const char* tablename);
	static void stopgcref();
	static void startgcref();
	static bool bIsGcRef;
	static bool bIsBeginPlay;

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void log(FString content);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void shutdown();
	static void clearStack();
	static bool existdata(void * p);
	
	static void executeFunc(FString funcName, int n = 0, int nargs = 0);
	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void Call_void(FString funcName, int count_param = -1);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void Push_obj(UObject *p);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void Push_float(float value);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void Push_str(FString value);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void Push_bool(bool isTrue);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static UObject* Call_obj(FString funcName, int count_param);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static float Call_float(FString funcName, int count_param);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static FString Call_str(FString funcName, int count_param);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static bool Call_bool(FString funcName, int count_param);

	UFUNCTION(BlueprintCallable, Category = "TableUtil")
	static void CtorCpp(UObject* p, FString classpath);

	static void addgcref(UObject* p);
	static void rmgcref(UObject* p);


	static FLuaGcObj gcobjs;
// 	UPROPERTY()
// 	static TArray<UObject*> s;
	template<typename T>
	static int push(T value);
	template<typename T>
	static int push(T* value);

	static void pushclass(const char* classname, void* p, bool bgcrecord = false);
	template<> static int push(lua_class value);
	template<> static int push(int value);
	template<> static int push(float value);
	template<> static int push(double value);
	template<> static int push(bool value);
	template<> static int push(FString value);
	template<> static int push(const char* value);

	template<class T> 
	static int push(TArray<T> value);


	template<typename T>
	static T pop(int index);

	template<> static void pop<void>(int index) {};

	template<typename T>
	static TArray<T> poparr(int index);

	template<> static int pop<int>(int index) {
		auto result = (int)lua_tointeger(L, index); 
		lua_pop(L, 1);
		return result;
	};
	template<> static bool pop<bool>(int index) {
		auto result = !!(lua_toboolean(L, index)); 
		lua_pop(L, 1);
		return result;
	};
	template<> static FName pop<FName>(int index) { 
		auto result = FName(luaL_checkstring(L, index));
		lua_pop(L, 1);
		return result;
	};
	template<> static FString pop<FString>(int index) {
		auto result = ANSI_TO_TCHAR(luaL_checkstring(L, index));
		lua_pop(L, 1);
		return result;
	};
	template<> static float pop<float>(int index) {
		auto result = (float)lua_tonumber(L, index); 
		lua_pop(L, 1);
		return result;
	};
	template<> static double pop<double>(int index) {
		auto result = (double)lua_tonumber(L, index);
		lua_pop(L, 1);
		return result;
	};
	// template<> static FString pop<double>(int index) { return (double)lua_tonumber(L, index); };



	template<class T1, class... T>
	static int push(T1 value, T... args)
	{
		int i = push(value); 
		return i + push(args...);
	}

	template<class returnType, class... T>
	static returnType callr(FString funcname, T... args)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 1, push(args...));
		return pop<returnType>(-1);
	}
	
	template<class returnType>
	static returnType callr(FString funcname)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 1, 0);
		return pop<returnType>(-1);
	}

	template<class innertype, class... T>
	static TArray<innertype> callarr(FString funcname, T... args)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 1, push(args...));
		return poparr<innertype>(-1);
	}

	template<class innertype>
	static TArray<innertype> callarr(FString funcname)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 1, 0);
		return poparr<innertype>(-1);
	}

	template<class... T>
	static void call(FString funcname, T... args)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 0, push(args...));
	}

	static void call(FString funcname)
	{
		if (L == nullptr)
			init();
		executeFunc(funcname, 0, 0);
	}
	
	static TMap<FString, TMap<FString, UProperty*>> propertyMap;
	static void InitClassMap();
	static UProperty* GetPropertyByName(FString classname, FString propertyname);
};

template<>
int UTableUtil::push(lua_class value)
{
	pushclass(value.name, value.p, true);
	return 1;
}
template<>
int UTableUtil::push(int value)
{
	lua_pushinteger(L, value);
	return 1;
}
template<>
int UTableUtil::push(float value)
{
	lua_pushnumber(L, value);
	return 1;
}
template<>
int UTableUtil::push(double value)
{
	lua_pushnumber(L, value);
	return 1;
}
template<>
int UTableUtil::push(bool value)
{
	lua_pushboolean(L, value);
	return 1;
}

template<>
int UTableUtil::push(FString value)
{
	lua_pushstring(L, TCHAR_TO_ANSI(*value));
	return 1;
}

template<>
int UTableUtil::push(const char* value)
{
	lua_pushstring(L, value);
	return 1;
}

template<typename T>
int UTableUtil::push(T* value)
{
// 	class T;
	UClass* Class = T::StaticClass();
	FString namecpp = FString::Printf(TEXT("%s%s"), Class->GetPrefixCPP(), *Class->GetName());
	pushclass(TCHAR_TO_ANSI(*namecpp), (void*)value, true);
	return 1;
}

template<typename T>
int UTableUtil::push(T value)
{
	// 	class T;
	FString name = typeid(T).name();
	int FirstSpaceIndex = name.Find(TEXT(" "));
	name = name.Mid(FirstSpaceIndex + 1);
	pushclass(TCHAR_TO_ANSI(*name), (void*)(new T(value)));
	return 1;
}


template<typename T>
T UTableUtil::pop(int index)
{
	return (T)tousertype("", index);
}

template<typename T>
TArray<T> UTableUtil::poparr(int index)
{
	TArray<T> result;
#ifdef LuaDebug
	if (!lua_istable(L, -1))
	{
		log("not table poparr");
		return result;
	}
#endif
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		result.Add(pop<T>(-1));
	}
	lua_pop(L, 1);
	return result;
}

template<class T> 
int UTableUtil::push(TArray<T> value)
{
	lua_newtable(L);
	for (int i = 0; i < value.Num(); i++)
	{
		push(i+1);
		push(value[i]);
		lua_rawset(L, -3);
	}
	return 1;
}