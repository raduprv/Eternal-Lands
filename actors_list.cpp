#include <algorithm>
#include <cstring>
#include "actor_init.h"
#include "actors_list.h"
#include "actor_scripts.h"
#include "asc.h"
#include "buffs.h"
#include "cal.h"
#include "cluster.h"
#include "elloggingwrapper.h"
#include "eye_candy_wrapper.h"
#include "hash.h"
#include "misc.h"
#include "special_effects.h"
#include "textures.h"
#include "translate.h"

#ifdef ACTORS_LIST_MUTEX_DEBUG
namespace
{

void did_we_deadlock(eternal_lands::ActorsList::Mutex& mutex)
{
	fprintf(stderr, "Failed to acquire lock, did we deadlock?\n");
	mutex.lock();
}

} // namespace

#define LOCK(m) \
do {\
	if (!m.try_lock_for(std::chrono::milliseconds(1000))) \
	{ \
		/* If we fail to take a lock for a full second, it's most likely dead-locked */ \
		did_we_deadlock(m); \
	} \
} while(false)
#define GUARD(g, m) \
	LOCK(m); \
	std::lock_guard<Mutex> g(m, std::adopt_lock_t())
#else // ACTORS_LIST_MUTEX_DEBUG
#define LOCK(m) m.lock()
#define GUARD(g, m) std::lock_guard<Mutex> g(m)
#endif // ACTORS_LIST_MUTEX_DEBUG

namespace eternal_lands
{

actor* ActorsList::lock_and_get_self()
{
	LOCK(_mutex);
	if (_self)
		return _self;
	_mutex.unlock();
	return nullptr;
}

actor* ActorsList::lock_and_get_actor_from_id(int actor_id)
{
	LOCK(_mutex);
	size_t idx = find_index_for_id(actor_id);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return nullptr;
	}
	return _list[idx];
}

std::pair<actor*, actor*> ActorsList::lock_and_get_self_and_actor_from_id(int actor_id)
{
	LOCK(_mutex);
	if (!_self)
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	size_t idx = find_index_for_id(actor_id);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	return { _self, _list[idx] };
}

std::pair<actor*, actor*> ActorsList::lock_and_get_actor_and_attached_from_id(int actor_id)
{
	LOCK(_mutex);
	size_t idx = find_index_for_id(actor_id);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	actor *act = _list[idx];
	int att_idx = act->attached_actor;
	actor *attached = (att_idx >= 0 && att_idx < _list.size()) ? _list[att_idx] : nullptr;

	return { act, attached };
}

std::pair<actor*, actor*> ActorsList::lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2)
{
	LOCK(_mutex);
	size_t idx = find_index_for_id(actor_id1);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}
	actor* act1 = _list[idx];

	idx = find_index_for_id(actor_id2);
	actor* act2 = idx < _list.size() ? _list[idx] : nullptr;

	return { act1, act2 };
}

actor* ActorsList::lock_and_get_actor_from_name(const char* name)
{
	size_t name_len = std::strlen(name);
	if (name_len >= sizeof_field(actor, actor_name))
		return nullptr;

	LOCK(_mutex);
	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[name, name_len](actor* act) {
			return !strncasecmp(act->actor_name, name, name_len)
				&& (act->actor_name[name_len] == ' ' || act->actor_name[name_len] == '\0');
		}
	);
	if (iter != _list.end())
		return *iter;
	_mutex.unlock();
	return nullptr;
}

actor* ActorsList::lock_and_get_actor_at_index(int idx)
{
	if (idx < 0)
		return nullptr;

	LOCK(_mutex);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return nullptr;
	}

	return _list[idx];
}

std::pair<actor*, actor*> ActorsList::lock_and_get_self_and_actor_at_index(int idx)
{
	if (idx < 0)
		return { nullptr, nullptr };

	LOCK(_mutex);
	if (!_self || idx >= _list.size())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	return { _self, _list[idx] };
}

std::pair<actor*, actor*> ActorsList::lock_and_get_actor_and_attached_at_index(int idx)
{
	if (idx < 0)
		return { nullptr, nullptr };

	LOCK(_mutex);
	if (idx >= _list.size())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	actor *act = _list[idx];
	int att_idx = act->attached_actor;
	actor *attached = (att_idx >= 0 && att_idx < _list.size()) ? _list[att_idx] : nullptr;

	return { act, attached };
}

actor* ActorsList::lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance)
{
	const float self_max_dist = 6.0;
	const float self_max_dist_sq = self_max_dist * self_max_dist;

	float x = 0.5 * tile_x;
	float y = 0.5 * tile_y;

	LOCK(_mutex);
	if (_self)
	{
		float dx = _self->x_pos - x;
		float dy = _self->y_pos - y;
		float dist_sq = dx*dx + dy*dy;
		if (dist_sq > self_max_dist_sq)
		{
			_mutex.unlock();
			return nullptr;
		}
	}

	actor* nearest_actor = nullptr;
	float min_dist_sq_found = max_distance * max_distance;
	for (actor* act: _list)
	{
		if (act->dead || act->kind_of_actor == NPC || act->kind_of_actor == HUMAN
			|| act->kind_of_actor == COMPUTER_CONTROLLED_HUMAN)
		{
			continue;
		}

		float dx = act->x_pos - x;
		float dy = act->y_pos - y;
		float dist_sq = dx*dx + dy*dy;
		if (dist_sq < min_dist_sq_found)
		{
			nearest_actor = act;
			min_dist_sq_found = dist_sq;
		}
	}

	if (!nearest_actor)
		_mutex.unlock();
	return nearest_actor;
}

std::pair<ActorsList::Storage&, actor*> ActorsList::lock_and_get_list_and_self()
{
	LOCK(_mutex);
	return { _list, _self };
}

#ifdef ECDEBUGWIN
actor* ActorsList::lock_and_get_target()
{
	LOCK(_mutex);

	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[this](const actor* other) { return other != _self; });
	if (iter == _list.end())
	{
		_mutex.unlock();
		return nullptr;
	}

	return *iter;

}

std::pair<actor*, actor*> ActorsList::lock_and_get_self_and_target()
{
	LOCK(_mutex);

	if (!_self)
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[this](const actor* other) { return other != _self; });
	if (iter == _list.end())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	return { _self, *iter };
}
#endif // ECDEBUGWIN

void ActorsList::add(actor* act, actor *attached)
{
	GUARD(guard, _mutex);

	// Find out if there is another actor with this ID.
	// Ideally this shouldn't happen, but just in case
	size_t idx = find_index_for_id(act->actor_id);
	if (idx < _list.size())
	{
		LOG_ERROR(duplicate_actors_str, act->actor_id, _list[idx]->actor_name, act->actor_name);
		remove_and_destroy_locked(idx);
	}

	if (act->is_enhanced_model && act->kind_of_actor == COMPUTER_CONTROLLED_HUMAN)
	{
		Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
			[act](const actor* other) {
				return (other->kind_of_actor == COMPUTER_CONTROLLED_HUMAN
							|| other->kind_of_actor == PKABLE_COMPUTER_CONTROLLED)
					&& strcasecmp(act->actor_name, other->actor_name) == 0;
			});
		if (iter != _list.end())
		{
			LOG_ERROR("%s(%d) = %s => %s\n", duplicate_npc_actor, act->actor_id,
				(*iter)->actor_name, act->actor_name);
			remove_and_destroy_locked(iter - _list.begin());
		}
	}

	_list.push_back(act);

	if (act->actor_id == yourself)
		_self = act;

	if (attached)
	{
		size_t attached_idx = _list.size();
		size_t parent_idx = attached_idx - 1;
		_list.push_back(attached);

		attached->attached_actor = int(parent_idx);
		act->attached_actor = int(attached_idx);
	}
}

bool ActorsList::add_attachment(int actor_id, actor *attached)
{
	GUARD(guard, _mutex);

	size_t parent_idx = find_index_for_id(actor_id);
	if (parent_idx < _list.size())
	{
		LOG_ERROR("unable to add an attached actor: actor with id %d doesn't exist!", actor_id);
		return false;
	}
	actor *parent = _list[parent_idx];

	size_t attached_idx = _list.size();
	_list.push_back(attached);

	attached->attached_actor = int(parent_idx);
	parent->attached_actor = int(attached_idx);

	return true;
}

actor *ActorsList::remove(int actor_id)
{
	GUARD(guard, _mutex);
	size_t idx = find_index_for_id(actor_id);
	return idx < _list.size() ? remove_locked(idx) : nullptr;
}

actor* ActorsList::remove_attachment(int actor_id)
{
	GUARD(guard, _mutex);

	size_t parent_idx = find_index_for_id(actor_id);
	if (parent_idx < _list.size())
	{
		actor *parent = _list[parent_idx];
		int attached_idx = parent->attached_actor;
		if (attached_idx >= 0 && attached_idx < _list.size())
		{
			actor *attached = remove_locked(attached_idx);
			parent->attachment_shift[0] = parent->attachment_shift[1] = parent->attachment_shift[2] = 0.0f;
			return attached;
		}
	}
	return nullptr;
}

void ActorsList::clear()
{
	GUARD(guard, _mutex);

	_self = nullptr;
	_actor_under_mouse = nullptr;
	for (actor* act: _list)
	{
		::destroy_actor(act);
	}
	_list.clear();
}

bool ActorsList::have_self()
{
	GUARD(guard, _mutex);
	return _self != nullptr;
}

void ActorsList::set_self()
{
	GUARD(guard, _mutex);
	size_t idx = find_index_for_id(yourself);
	if (idx < _list.size())
		_self = _list[idx];
}

int ActorsList::set_actor_under_mouse(int idx)
{
	if (idx < 0)
		return -1;

	GUARD(guard, _mutex);
	if (idx >= _list.size())
		return -1;

	_actor_under_mouse = _list[idx];
	return _actor_under_mouse->actor_id;
}

bool ActorsList::actor_under_mouse_alive()
{
	GUARD(guard, _mutex);
	return _actor_under_mouse && !_actor_under_mouse->dead;
}

void ActorsList::clear_actor_under_mouse()
{
	GUARD(guard, _mutex);
	_actor_under_mouse = nullptr;
}

bool ActorsList::actor_occupies_tile(int x, int y)
{
	GUARD(guard, _mutex);
	return std::find_if(_list.begin(), _list.end(),
		[x, y](const actor* act) { return act->x_tile_pos == x && act->y_tile_pos == y; }
	) != _list.end();
}

size_t ActorsList::find_index_for_id(int actor_id)
{
	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[actor_id](actor* act) { return act->actor_id == actor_id; });
	return iter - _list.begin();
}

actor* ActorsList::remove_locked(size_t idx)
{
	if (idx >= _list.size())
		return nullptr;

	actor* res = _list[idx];
	if (res->attached_actor >= 0 && res->attached_actor < _list.size())
		_list[res->attached_actor]->attached_actor = -1;
	actor* last = _list.back();
	if (last->attached_actor >= 0 && last->attached_actor < _list.size())
		_list[last->attached_actor]->attached_actor = int(idx);
	_list[idx] = last;
	_list.pop_back();

	if (res == _self)
		_self = nullptr;
	if (res == _actor_under_mouse)
		_actor_under_mouse = nullptr;

	return res;
}

void ActorsList::remove_and_destroy_locked(size_t idx)
{
	actor* act = remove_locked(idx);
	if (act)
	{
		if (act->attached_actor >= 0 && act->attached_actor <= _list.size())
		{
			actor *attached = remove_locked(act->attached_actor);
			::destroy_actor(attached);
		}
		::destroy_actor(act);
	}
}

} // namespace eternal_lands

using namespace eternal_lands;

extern "C" actor** lock_and_get_actors_list(std::size_t *len)
{
	ActorsList::Storage& list = ActorsList::get_instance().get_locked();
	if (len)
		*len = list.size();
	return list.data();
}

extern "C" void release_actors_list()
{
	ActorsList::get_instance().release();
}

extern "C" actor* lock_and_get_self()
{
	return ActorsList::get_instance().lock_and_get_self();
}

extern "C" actor* lock_and_get_actor_from_id(int id)
{
	return ActorsList::get_instance().lock_and_get_actor_from_id(id);
}

extern "C" actor* lock_and_get_self_and_actor_from_id(int actor_id, actor **act)
{
	actor *self;
	std::tie(self, *act) = ActorsList::get_instance().lock_and_get_self_and_actor_from_id(actor_id);
	return self;
}

extern "C" actor* lock_and_get_actor_and_attached_from_id(int actor_id, actor **horse)
{
	actor *act;
	std::tie(act, *horse) = ActorsList::get_instance()
		.lock_and_get_actor_and_attached_from_id(actor_id);
	return act;
}

extern "C" actor* lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2, actor **act2)
{
	actor *act1;
	std::tie(act1, *act2) = ActorsList::get_instance()
		.lock_and_get_actor_pair_from_id(actor_id1, actor_id2);
	return act1;
}

extern "C" actor* lock_and_get_actor_from_name(const char* name)
{
	return ActorsList::get_instance().lock_and_get_actor_from_name(name);
}

extern "C" actor* lock_and_get_actor_at_index(int idx)
{
	return ActorsList::get_instance().lock_and_get_actor_at_index(idx);
}

extern "C" actor* lock_and_get_self_and_actor_at_index(int idx, actor **act)
{
	actor *self;
	std::tie(self, *act) = ActorsList::get_instance().lock_and_get_self_and_actor_at_index(idx);
	return self;
}

extern "C" actor* lock_and_get_actor_and_attached_at_index(int idx, actor **horse)
{
	actor *act;
	std::tie(act, *horse) = ActorsList::get_instance().lock_and_get_actor_and_attached_at_index(idx);
	return act;
}

extern "C" actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance)
{
	return ActorsList::get_instance().lock_and_get_nearest_actor(tile_x, tile_y, max_distance);
}

extern "C" actor** lock_and_get_list_and_self(size_t *len, actor **self)
{
	std::pair<ActorsList::Storage& , actor*> tmp = ActorsList::get_instance()
		.lock_and_get_list_and_self();
	*len = tmp.first.size();
	*self = tmp.second;
	return tmp.first.data();
}

#ifdef ECDEBUGWIN
extern "C" actor* lock_and_get_target()
{
	return ActorsList::get_instance().lock_and_get_target();
}

extern "C" actor* lock_and_get_self_and_target(actor **target)
{
	actor *self;
	std::tie(self, *target) = ActorsList::get_instance().lock_and_get_self_and_target();
	return self;
}
#endif

extern "C" void add_actor_to_list(actor* act, actor *attached)
{
	ActorsList::get_instance().add(act, attached);
}

extern "C" actor* remove_actor_from_list(int actor_id)
{
	return ActorsList::get_instance().remove(actor_id);
}

extern "C" int add_attachment_to_list(int actor_id, actor *attached)
{
	return ActorsList::get_instance().add_attachment(actor_id, attached);
}

extern "C" actor* remove_attachment_from_list(int actor_id)
{
	return ActorsList::get_instance().remove_attachment(actor_id);
}

extern "C" void remove_and_destroy_all_actors()
{
	return ActorsList::get_instance().clear();
}

extern "C" int have_self()
{
	return ActorsList::get_instance().have_self();
}

extern "C" void set_self()
{
	ActorsList::get_instance().set_self();
}

extern "C" void clear_actor_under_mouse()
{
	ActorsList::get_instance().clear_actor_under_mouse();
}

extern "C" int actor_under_mouse_alive()
{
	return ActorsList::get_instance().actor_under_mouse_alive();
}

extern "C" int actor_occupies_tile(int x, int y)
{
	return ActorsList::get_instance().actor_occupies_tile(x, y);
}
