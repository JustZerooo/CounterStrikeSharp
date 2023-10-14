#include "core/globals.h"
#include "core/managers/player_manager.h"
#include "iserver.h"
#include "managers/event_manager.h"
#include "scripting/callback_manager.h"
#include "scripting/dotnet_host.h"
#include "timer_system.h"

#include <ISmmPlugin.h>
#include <sourcehook/sourcehook.h>
#include <sourcehook/sourcehook_impl.h>

#include "log.h"
#include "utils/virtual.h"
#include "core/managers/con_command_manager.h"
#include "memory_module.h"
#include "interfaces/cs2_interfaces.h"
#include <public/game/server/iplayerinfo.h>
#include <public/entity2/entitysystem.h>

namespace counterstrikesharp {

namespace globals {
IVEngineServer *engine = nullptr;
IGameEventManager2 *gameEventManager = nullptr;
IGameEventSystem *gameEventSystem = nullptr;
IPlayerInfoManager *playerinfoManager = nullptr;
IBotManager *botManager = nullptr;
IServerPluginHelpers *helpers = nullptr;
IUniformRandomStream *randomStream = nullptr;
IEngineTrace *engineTrace = nullptr;
IEngineSound *engineSound = nullptr;
INetworkStringTableContainer *netStringTables = nullptr;
CGlobalVars *globalVars = nullptr;
IFileSystem *fileSystem = nullptr;
IServerGameDLL *serverGameDll = nullptr;
IServerGameClients *serverGameClients = nullptr;
INetworkServerService *networkServerService = nullptr;
IServerTools *serverTools = nullptr;
IPhysics *physics = nullptr;
IPhysicsCollision *physicsCollision = nullptr;
IPhysicsSurfaceProps *physicsSurfaceProps = nullptr;
IMDLCache *modelCache = nullptr;
IVoiceServer *voiceServer = nullptr;
CDotNetManager dotnetManager;
ICvar *cvars = nullptr;
ISource2Server *server = nullptr;
CGlobalEntityList *globalEntityList = nullptr;
CounterStrikeSharpMMPlugin *mmPlugin = nullptr;
SourceHook::Impl::CSourceHookImpl source_hook_impl;
SourceHook::ISourceHook *source_hook = &source_hook_impl;
ISmmAPI *ismm = nullptr;
CEntitySystem* entitySystem = nullptr;

// Custom Managers
CallbackManager callbackManager;
EventManager eventManager;
PlayerManager playerManager;
TimerSystem timerSystem;
ConCommandManager conCommandManager;

void Initialize() {
    addresses::Initialize();
    interfaces::Initialize();

    globals::entitySystem = interfaces::pGameResourceServiceServer->GetGameEntitySystem();

    gameEventManager = (IGameEventManager2 *)(CALL_VIRTUAL(uintptr_t, 91, server) - 8);

    CSSHARP_CORE_TRACE("[GLOBALS] globals::gameEventManager: {0}", (void *)gameEventManager);
}

int source_hook_pluginid = 0;
CGlobalVars *getGlobalVars() {
    INetworkGameServer *server = networkServerService->GetIGameServer();

    if (!server) return nullptr;

    return networkServerService->GetIGameServer()->GetGlobals();
}

}  // namespace globals
}  // namespace counterstrikesharp
