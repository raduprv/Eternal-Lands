#ifndef ACTORS_LIST_H
#define ACTORS_LIST_H

#ifdef __cplusplus
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>
#endif // __cplusplus

#include "actors.h"

#ifdef __cplusplus

namespace eternal_lands
{

class ActorsList
{
public:
#ifdef ACTORS_LIST_MUTEX_DEBUG
 #ifdef ACTORS_LIST_NO_RECURSIVE_MUTEX
	typedef std::timed_mutex Mutex;
 #else // ACTORS_LIST_NO_RECURSIVE_MUTEX
	typedef std::recursive_timed_mutex Mutex;
 #endif // ACTORS_LIST_NO_RECURSIVE_MUTEX
#else // ACTORS_LIST_MUTEX_DEBUG
 #ifdef ACTORS_LIST_NO_RECURSIVE_MUTEX
	typedef std::mutex Mutex;
 #else // ACTORS_LIST_NO_RECURSIVE_MUTEX
	typedef std::recursive_mutex Mutex;
 #endif // ACTORS_LIST_NO_RECURSIVE_MUTEX
#endif // ACTORS_LIST_MUTEX_DEBUG
	typedef std::vector<actor*> Storage;

	template <typename T>
	class Locked
	{
	public:
		Locked(T& obj, Mutex& mutex): _mutex(mutex), _obj(obj)
		{
			_mutex.lock();
		}
		~Locked()
		{
			_mutex.unlock();
		}

		const T& operator*() const { return _obj; }
		T& operator*() { return _obj; }
		const T* operator->() const { return &_obj; }
		T* operator->() { return &_obj; }

	private:
		Mutex& _mutex;
		T& _obj;
	};

	typedef Locked<Storage> LockedList;
	typedef Locked<actor*> LockedActorPtr;

	static ActorsList& get_instance()
	{
		static ActorsList actors_list;
		return actors_list;
	}

	LockedList get() { return LockedList(_list, _mutex); }
	LockedActorPtr get_self() { return LockedActorPtr(_self, _mutex); }

	Storage& get_locked()
	{
		_mutex.lock();
		return _list;
	}
	void release()
	{
		_mutex.unlock();
	}
	actor* lock_and_get_self();
	actor* lock_and_get_actor_from_id(int actor_id);
	std::pair<actor*, actor*> lock_and_get_self_and_actor_from_id(int actor_id);
	std::pair<actor*, actor*> lock_and_get_actor_and_attached_from_id(int actor_id);
	std::pair<actor*, actor*> lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2);
	actor* lock_and_get_actor_from_name(const char* name);
	actor* lock_and_get_actor_at_index(int idx);
	std::pair<actor*, actor*> lock_and_get_self_and_actor_at_index(int idx);
	std::pair<actor*, actor*> lock_and_get_actor_and_attached_at_index(int idx);
	actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance);
	std::pair<Storage&, actor*> lock_and_get_list_and_self();
#ifdef ECDEBUGWIN
	actor* lock_and_get_target();
	std::pair<actor*, actor*> lock_and_get_self_and_target();
#endif

	void add(actor *act, actor *attached);
	bool add_attachment(int actor_id, actor *attached);
	actor* remove(int actor_id);
	actor* remove_attachment(int actor_id);
	void clear();

	bool have_self();
	void set_self();

	int set_actor_under_mouse(int idx);
	void clear_actor_under_mouse();
	bool actor_under_mouse_alive();

	bool actor_occupies_tile(int x, int y);

private:
	Storage _list;
	Mutex _mutex;
	actor* _self;
	actor* _actor_under_mouse;

	ActorsList(): _list(), _mutex(), _self(nullptr),  _actor_under_mouse(nullptr) {}

	size_t find_index_for_id(int actor_id);

	actor* remove_locked(size_t idx);
	void remove_and_destroy_locked(size_t idx);
};

} // namespace eternal_lands

extern "C"
{
#endif // __cplusplus

actor** lock_and_get_actors_list(size_t* len);
void release_actors_list(void);
actor* lock_and_get_self();
actor* lock_and_get_actor_from_id(int id);
actor* lock_and_get_self_and_actor_from_id(int actor_id, actor **act);
actor* lock_and_get_actor_and_attached_from_id(int actor_id, actor **horse);
actor* lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2, actor **act2);
actor* lock_and_get_actor_from_name(const char* name);
actor* lock_and_get_actor_at_index(int idx);
actor* lock_and_get_self_and_actor_at_index(int idx, actor **act);
actor* lock_and_get_actor_and_attached_at_index(int idx, actor **horse);
actor* lock_and_get_self_and_actor_at_index(int idx, actor **act);
actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance);
actor** lock_and_get_list_and_self(size_t *len, actor **self);
#ifdef ECDEBUGWIN
actor* lock_and_get_target(void);
actor* lock_and_get_self_and_target(actor **target);
#endif
void add_actor_to_list(actor *act, actor *attached);
actor* remove_actor_from_list(int actor_id);
int add_attachment_to_list(int actor_id, actor *attached);
actor* remove_attachment_from_list(int actor_id);
void remove_and_destroy_all_actors(void);
int have_self(void);
void set_self(void);
float self_scale(void);
void clear_actor_under_mouse(void);
int actor_under_mouse_alive(void);
int actor_occupies_tile(int x, int y);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



#endif // ACTORS_LIST_H
