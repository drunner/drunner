#ifndef __LUA_UTILS_H
#define __LUA_UTILS_H

#include "lua.hpp"
#include "dassert.h"

//----------------------------------------------------------------

namespace luautils
{

   template<typename T>
   class staticLuaStorage {
   public:
      staticLuaStorage() {}
      void set(lua_State * L, T * tptr)
      {
         drunner_assert(mLookupTable.count(L) == 0, "Multiple registrations of one class.");
         mLookupTable[L] = tptr;
      }
      void clear(lua_State *L)
      {
         drunner_assert(mLookupTable.count(L) == 1, "Trying to clear class that isn't registered.");
         mLookupTable.erase(L);
      }
      T * get(lua_State *L)
      {
         drunner_assert(mLookupTable.count(L) == 1, "Trying to get class that isn't registered.");
         return mLookupTable[L];
      }
   private:
      std::map<lua_State *, T *> mLookupTable;
   };

   template<typename T>
   class staticmonitor {
   public:
      staticmonitor(lua_State * L, T * tptr, staticLuaStorage<T> * sls) : mSLS(sls), mLuaStatePtr(L)
      {
         mSLS->set(mLuaStatePtr, tptr);
      }
      ~staticmonitor() {
         mSLS->clear(mLuaStatePtr);
      }
   private:
      staticLuaStorage<T> * mSLS;
      lua_State * mLuaStatePtr;
   };


   class dLuaState {
   public:
      dLuaState()
      {
         L = luaL_newstate();
         luaL_openlibs(L);
      }

      ~dLuaState()
      {
         if (L)
            lua_close(L);
      }

      lua_State * get() { return L; }

   private:
      lua_State * L;
   };


   // -------------------------------------------------------------

} // namespace

#endif