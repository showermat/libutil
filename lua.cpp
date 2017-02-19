#include "lua.h"

namespace lua
{
	iter::iter(exec &owner, const std::string &name) : o{owner}, offset{0}
	{
		lua_getglobal(o.state, name.c_str());
		offset = lua_gettop(o.state);
		lua_pushnil(o.state);
	}

	bool iter::next()
	{
		return (lua_next(o.state, offset) != 0);
	}

	void iter::close()
	{
		lua_remove(o.state, offset);
	}

	void exec::err()
	{
		std::string msg{lua_tostring(state, -1)};
		lua_pop(state, 1);
		throw std::runtime_error{msg};
	}

	template <> void exec::push<bool>(bool t) { lua_pushboolean(state, t); }
	template <> void exec::push<int>(int t) { lua_pushinteger(state, t); }
	template <> void exec::push<double>(double t) { lua_pushnumber(state, t); }
	template <> void exec::push<std::string>(std::string t) { lua_pushstring(state, t.c_str()); }
	template <> bool exec::to<bool>(int i) { return lua_toboolean(state, i); }
	template <> int exec::to<int>(int i) { return lua_tointeger(state, i); }
	template <> double exec::to<double>(int i) { return lua_tonumber(state, i); }
	template <> std::string exec::to<std::string>(int i) { return std::string{lua_tostring(state, i)}; }
	
	exec::exec() : state{luaL_newstate()}
	{
		if (! state) throw std::runtime_error{"Couldn't initialize Lua state"};
		luaL_openlibs(state);
	}

	void exec::loadstr(const std::string &prog)
	{
		if (! state) throw std::runtime_error{"Can't run invalid Lua state"};
		if (luaL_dostring(state, prog.c_str()) != LUA_OK) err();
	}

	void exec::load(const std::string &prog)
	{
		if (! state) throw std::runtime_error{"Can't run invalid Lua state"};
		if (luaL_loadfile(state, prog.c_str()) != LUA_OK) err();
		if (lua_pcall(state, 0, 0, 0) != LUA_OK) err();
	}

	bool exec::exists(const std::string &name)
	{
		lua_getglobal(state, name.c_str());
		bool ret = ! lua_isnil(state, -1);
		lua_pop(state, 1);
		return ret;
	}

	iter exec::table_iter(const std::string &name)
	{
		return iter{*this, name};
	}

	exec::~exec()
	{
		if (state) lua_close(state);
	}
}

