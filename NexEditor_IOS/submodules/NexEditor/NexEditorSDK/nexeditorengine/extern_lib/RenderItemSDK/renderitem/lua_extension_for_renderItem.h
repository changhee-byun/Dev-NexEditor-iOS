#pragma once

#define NO_SDL_GLEXT
#include "NexMath.h"
#include "lua.hpp"
#include "types.h"

void setDefaultGLDefineToLUA(lua_State* L);
void registerLuaExtensionFunc(lua_State* L);
void broadcastGlobalVariable(lua_State* lua_state, const char* szkey, const char* val);
void doListenerWork(const char* key, const char* val);

