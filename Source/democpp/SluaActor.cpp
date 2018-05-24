// Fill out your copyright notice in the Description page of Project Settings.


#include "SluaActor.h"
#include "SluaComponent.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"


ASluaActor* ASluaActor::instance=nullptr;

// Sets default values
ASluaActor::ASluaActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	instance = this;
}

// Called when the game starts or when spawned
void ASluaActor::BeginPlay()
{
	Super::BeginPlay();
	
	auto slua = FindComponentByClass<USluaComponent>();
	auto state = slua->State();
	state->setLoadFileDelegate([](const char* fn,uint32& len)->uint8* {

		FString path = FPaths::ProjectContentDir();
		path+="/Lua/";
		path+=UTF8_TO_TCHAR(fn);
		path+=".lua";

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
 
		IFileHandle* FileHandle = PlatformFile.OpenRead(*path);
		if(FileHandle)
		{
			len = (uint32) FileHandle->Size();
			uint8* buf = new uint8[len];

			FileHandle->Read(buf, len);
		
			// Close the file again
			delete FileHandle;

			return buf;
		}

		return nullptr;
	});
	slua::LuaVar v = state->doFile("Test");
	if(!v.isNil()) {
		ensure(v.isTuple());
		ensure(v.count()==5);
		ensure(v.getAt(0).asInt()==1024);
		slua::Log::Log("first return value is %d",v.getAt(0).asInt());
	}

	state->call("xx.text");
}

// Called every frame
void ASluaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto slua = FindComponentByClass<USluaComponent>();
	auto state = slua->State();
	slua::LuaVar v = state->call("update",DeltaTime,this);
	if(!v.isNil()) {
		// test copy constructor
		slua::LuaVar v2 = v;
		slua::LuaVar v3 = v2;
		ensure(v.isTuple());
		ensure(v.count()==5);
		ensure(v.getAt(0).asInt()==1024);
	}

	// slua::Log::Log("lua stack top %d",lua_gettop(*state));
}

void ASluaActor::SetFName(FName name) {
	slua::Log::Log("set fname %s", TCHAR_TO_UTF8(*(name.ToString())));	
}

