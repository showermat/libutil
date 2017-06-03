#ifndef UTIL_LUA_H
#define UTIL_LUA_H
#include <stdexcept>
#include <string>
#include <vector>
#include <lua.hpp>

namespace lua
{
	class iter;
	class exec
	{
	private:
		lua_State *state;
		void err();
		template <typename T> void push(T t);
		template <typename T> T to(int i = -1);
		template <typename H> void push_args(H arg) { push<H>(arg); }
		template <typename H, typename... T> void push_args(H arg, T... args) { push<H>(arg); push_args<T...>(args...); }
		template <typename T> T get_pop(int n = 1)
		{
			T ret = to<T>();
			lua_pop(state, 1);
			return ret;
		}
		friend class iter;
	public:
		exec();
		void loadstr(const std::string &prog);
		void load(const std::string &path);
		template <typename Ret, typename... Args> Ret call(const std::string &func, Args... args)
		{
			lua_getglobal(state, func.c_str());
			push_args(args...);
			if (lua_pcall(state, sizeof...(Args), 1, 0) != LUA_OK) err();
			return get_pop<Ret>();
		}
		bool exists(const std::string &name);
		template <typename T> T getvar(const std::string &name)
		{
			lua_getglobal(state, name.c_str());
			return get_pop<T>();
		}
		template <typename T> T getfield(const std::string &table, const std::string &field)
		{
			lua_getglobal(state, table.c_str());
			lua_getfield(state, -1, field.c_str());
			return get_pop<T>(2);
		}
		iter table_iter(const std::string &name);
		virtual ~exec();
	};

	class iter
	{
	private:
		exec &o;
		int offset;
	public:
		iter(exec &o, const std::string &name);
		bool next();
		template <typename K, typename V> std::pair<K, V> get()
		{
			V val = o.get_pop<V>();
			K key = o.to<K>();
			return std::make_pair(key, val);
		}
		void close();
		virtual ~iter() { close(); }
	};
}

#endif

