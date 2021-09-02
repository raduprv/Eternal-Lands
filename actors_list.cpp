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
#else // ACTORS_LIST_MUTEX_DEBUG
#define LOCK(m) m.lock()
#endif // ACTORS_LIST_MUTEX_DEBUG

namespace eternal_lands
{

ActorsList::LockedList ActorsList::lock()
{
	LOCK(_mutex);
	return LockedList(*this, _mutex);
}

ActorsList::LockedList* ActorsList::lock_ptr()
{
	LOCK(_mutex);
	return new LockedList(*this, _mutex);
}



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
	actor *act = get_actor_from_id_locked(actor_id);
	if (!act)
		_mutex.unlock();
	return act;
}

std::pair<actor*, actor*> ActorsList::lock_and_get_self_and_actor_from_id(int actor_id)
{
	LOCK(_mutex);
	auto res = get_self_and_actor_from_id_locked(actor_id);
	if (!res.first)
		_mutex.unlock();
	return res;
}

std::pair<actor*, actor*> ActorsList::lock_and_get_actor_and_attached_from_id(int actor_id)
{
	LOCK(_mutex);
	auto res = get_actor_and_attached_from_id_locked(actor_id);
	if (!res.first)
		_mutex.unlock();
	return res;
}

std::pair<actor*, actor*> ActorsList::lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2)
{
	LOCK(_mutex);
	return get_actor_pair_from_id_locked(actor_id1, actor_id2);
}

actor* ActorsList::lock_and_get_actor_from_name(const char* name)
{
	LOCK(_mutex);
	actor *act = get_actor_from_name_locked(name);
	if (!act)
		_mutex.unlock();
	return act;
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
	for (auto id_act: _list)
	{
		actor* act = id_act.second;
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

#ifdef ECDEBUGWIN
actor* ActorsList::lock_and_get_target()
{
	LOCK(_mutex);

	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[this](const std::pair<int, actor*>& other) { return other.second != _self; });
	if (iter == _list.end())
	{
		_mutex.unlock();
		return nullptr;
	}

	return iter->second;

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
		[this](const std::pair<int, actor*>& other) { return other.second != _self; });
	if (iter == _list.end())
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	return { _self, iter->second };
}
#endif // ECDEBUGWIN


std::pair<actor*, actor*> ActorsList::get_self_and_actor_from_id_locked(int actor_id)
{
	if (!_self)
		return { nullptr, nullptr };

	actor *act = get_actor_from_id_locked(actor_id);
	if (!act)
		return { nullptr, nullptr };

	return { _self, act };
}

std::pair<actor*, actor*> ActorsList::get_actor_and_attached_from_id_locked(int actor_id)
{
	actor *act = get_actor_from_id_locked(actor_id);
	if (!act)
		return { nullptr, nullptr };

	actor *attached = has_attachment(act)
		? get_actor_from_id_locked(act->attached_actor_id)
		: nullptr;

	return { act, attached };
}

std::pair<actor*, actor*> ActorsList::get_actor_pair_from_id_locked(int actor_id1, int actor_id2)
{
	actor *act1 = get_actor_from_id_locked(actor_id1);
	if (!act1)
	{
		_mutex.unlock();
		return { nullptr, nullptr };
	}

	actor *act2 = get_actor_from_id_locked(actor_id2);
	return { act1, act2 };
}

actor* ActorsList::get_actor_from_name_locked(const char* name)
{
	size_t name_len = std::strlen(name);
	if (name_len >= sizeof_field(actor, actor_name))
		return nullptr;

	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[name, name_len](const std::pair<int, actor*>& id_act) {
			actor* act = id_act.second;
			return !strncasecmp(act->actor_name, name, name_len)
				&& (act->actor_name[name_len] == ' ' || act->actor_name[name_len] == '\0');
		}
	);
	return (iter != _list.end()) ? iter->second : nullptr;
}

void ActorsList::add_locked(actor* act, actor *attached)
{
	// Find out if there is another actor with this ID.
	// Ideally this shouldn't happen, but just in case
	Storage::const_iterator iter = _list.find(act->actor_id);
	if (iter != _list.end())
	{
		LOG_ERROR(duplicate_actors_str, act->actor_id, iter->second->actor_name,
			act->actor_name);
		remove_and_destroy_locked(iter);
	}

	if (act->is_enhanced_model && act->kind_of_actor == COMPUTER_CONTROLLED_HUMAN)
	{
		Storage::const_iterator iter = std::find_if(_list.begin(), _list.end(),
			[act](const std::pair<int, actor*>& id_act) {
				const actor* other = id_act.second;
				return (other->kind_of_actor == COMPUTER_CONTROLLED_HUMAN
							|| other->kind_of_actor == PKABLE_COMPUTER_CONTROLLED)
					&& strcasecmp(act->actor_name, other->actor_name) == 0;
			});
		if (iter != _list.end())
		{
			LOG_ERROR("%s(%d) = %s => %s\n", duplicate_npc_actor, act->actor_id,
				iter->second->actor_name, act->actor_name);
			remove_and_destroy_locked(iter);
		}
	}

	_list.insert( { act->actor_id, act } );

	if (act->actor_id == yourself)
		_self = act;

	if (attached)
	{
		_list.insert( { attached->actor_id, attached } );
		attached->attached_actor_id = act->actor_id;
		act->attached_actor_id = attached->actor_id;
	}
}

bool ActorsList::add_attachment_locked(int actor_id, actor *attached)
{
	actor *parent = get_actor_from_id_locked(actor_id);
	if (!parent)
	{
		LOG_ERROR("unable to add an attached actor: actor with id %d doesn't exist!", actor_id);
		return false;
	}
	if (has_attachment(parent))
	{
		LOG_ERROR("actor with ID %d already has an attachment", actor_id);
		return false;
	}

	_list.insert( { attached->actor_id, attached } );
	attached->attached_actor_id = parent->actor_id;
	parent->attached_actor_id = attached->actor_id;

	return true;
}

void ActorsList::remove_and_destroy_single_locked(Storage::const_iterator iter)
{
	actor *act = iter->second;
	_list.erase(iter);
	::destroy_actor(act);
}

void ActorsList::remove_and_destroy_locked(Storage::const_iterator iter)
{
	actor *act = iter->second;
	if (has_attachment(act))
	{
		Storage::iterator att_iter = _list.find(act->attached_actor_id);
		if (att_iter != _list.end())
			remove_and_destroy_single_locked(iter);
	}
	remove_and_destroy_single_locked(iter);
}

void ActorsList::remove_and_destroy_locked(int actor_id)
{
	Storage::const_iterator iter = _list.find(actor_id);
	if (iter != _list.end())
		remove_and_destroy_locked(iter);
}

void ActorsList::remove_and_destroy_attachment_locked(int actor_id)
{
	actor *parent = get_actor_from_id_locked(actor_id);
	if (parent && has_attachment(parent))
	{
		Storage::const_iterator iter = _list.find(parent->attached_actor_id);
		if (iter != _list.end())
			remove_and_destroy_single_locked(iter);
		parent->attached_actor_id = -1;
		parent->attachment_shift[0] = parent->attachment_shift[1]
				= parent->attachment_shift[2] = 0.0f;
	}
}

void ActorsList::clear_locked()
{
	_self = nullptr;
	_actor_under_mouse = nullptr;
	for (const std::pair<const int, actor*>& id_act: _list)
	{
		::destroy_actor(id_act.second);
	}
	_list.clear();
}

} // namespace eternal_lands

using namespace eternal_lands;

extern "C" locked_list_ptr get_locked_actors_list()
{
	return ActorsList::get_locked_instance_ptr();
}

extern "C" actor* get_self(locked_list_ptr list)
{
	return list->self();
}

extern "C" actor* get_actor_from_id(locked_list_ptr list, int actor_id)
{
	return list->get_actor_from_id(actor_id);
}

extern "C" void for_each_actor(locked_list_ptr list, void (*fun)(actor*, void*, locked_list_ptr),
	void* data)
{
	list->for_each_actor(fun, data);
}

extern "C" void for_each_actor_and_attached(locked_list_ptr list,
	void (*fun)(actor*, actor*, void*, locked_list_ptr), void* data)
{
	list->for_each_actor_and_attached(fun, data);
}

extern "C" void release_locked_actors_list(locked_list_ptr list)
{
	delete list;
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

extern "C" actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance)
{
	return ActorsList::get_instance().lock_and_get_nearest_actor(tile_x, tile_y, max_distance);
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
	auto list = ActorsList::get_locked_instance();
	list.add(act, attached);
}

extern "C" int add_attachment_to_list(int actor_id, actor *attached)
{
	auto list = ActorsList::get_locked_instance();
	return list.add_attachment(actor_id, attached);
}

extern "C" void remove_and_destroy_actor_from_list(int actor_id)
{
	auto list = ActorsList::get_locked_instance();
	list.remove_and_destroy(actor_id);
}

extern "C" void remove_and_destroy_attachment_from_list(int actor_id)
{
	auto list = ActorsList::get_locked_instance();
	list.remove_and_destroy_attachment(actor_id);
}

extern "C" void remove_and_destroy_all_actors()
{
	auto list = ActorsList::get_locked_instance();
	list.clear();
}

extern "C" int have_self()
{
	auto list = ActorsList::get_locked_instance();
	return list.self() != nullptr;
}

extern "C" void set_self()
{
	auto list = ActorsList::get_locked_instance();
	list.set_self();
}

extern "C" int actor_under_mouse_alive()
{
	auto list = ActorsList::get_locked_instance();
	return list.actor_under_mouse_alive();
}

extern "C" int actor_occupies_tile(int x, int y)
{
	auto list = ActorsList::get_locked_instance();
	return list.actor_occupies_tile(x, y);
}
