#include <algorithm>
#include <cstring>
#include "actors_list.h"
#include "actor_init.h"
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

namespace eternal_lands
{

LockedActorsList::Storage LockedActorsList::_list;
LockedActorsList::Mutex LockedActorsList::_mutex;
actor* LockedActorsList::_self = nullptr;
actor* LockedActorsList::_actor_under_mouse = nullptr;

std::pair<actor*, actor*> LockedActorsList::get_actor_and_attached_from_id(int actor_id)
{
	actor *act = get_actor_from_id(actor_id);
	if (!act)
		return { nullptr, nullptr };

	actor *attached = has_attachment(act)
		? get_actor_from_id(act->attached_actor_id)
		: nullptr;

	return { act, attached };
}

actor* LockedActorsList::get_actor_from_name(const char* name)
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

actor* LockedActorsList::get_nearest_actor(int tile_x, int tile_y, float max_distance)
{
	const float self_max_dist = 6.0;
	const float self_max_dist_sq = self_max_dist * self_max_dist;

	float x = 0.5 * tile_x;
	float y = 0.5 * tile_y;

	if (_self)
	{
		float dx = _self->x_pos - x;
		float dy = _self->y_pos - y;
		float dist_sq = dx*dx + dy*dy;
		if (dist_sq > self_max_dist_sq)
			return nullptr;
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

	return nearest_actor;
}

#ifdef ECDEBUGWIN
actor* LockedActorsList::get_target()
{
	Storage::iterator iter = std::find_if(_list.begin(), _list.end(),
		[this](const std::pair<int, actor*>& other) { return other.second != _self; });
	return (iter != _list.end()) ? iter->second : nullptr;
}
#endif // ECDEBUGWIN



void LockedActorsList::add(actor* act, actor *attached)
{
	// Find out if there is another actor with this ID.
	// Ideally this shouldn't happen, but just in case
	Storage::const_iterator iter = _list.find(act->actor_id);
	if (iter != _list.end())
	{
		LOG_ERROR(duplicate_actors_str, act->actor_id, iter->second->actor_name,
			act->actor_name);
		remove_and_destroy(iter);
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
			remove_and_destroy(iter);
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

bool LockedActorsList::add_attachment(int actor_id, actor *attached)
{
	actor *parent = get_actor_from_id(actor_id);
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

void LockedActorsList::remove_and_destroy_single(Storage::const_iterator iter)
{
	actor *act = iter->second;
	_list.erase(iter);
	::destroy_actor(act);
}

void LockedActorsList::remove_and_destroy(Storage::const_iterator iter)
{
	actor *act = iter->second;
	if (has_attachment(act))
	{
		Storage::iterator att_iter = _list.find(act->attached_actor_id);
		if (att_iter != _list.end())
			remove_and_destroy_single(att_iter);
	}
	remove_and_destroy_single(iter);
}

void LockedActorsList::remove_and_destroy(int actor_id)
{
	Storage::const_iterator iter = _list.find(actor_id);
	if (iter != _list.end())
		remove_and_destroy(iter);
}

void LockedActorsList::remove_and_destroy_attachment(int actor_id)
{
	actor *parent = get_actor_from_id(actor_id);
	if (parent && has_attachment(parent))
	{
		Storage::const_iterator iter = _list.find(parent->attached_actor_id);
		if (iter != _list.end())
			remove_and_destroy_single(iter);
		parent->attached_actor_id = -1;
		parent->attachment_shift[0] = parent->attachment_shift[1]
			= parent->attachment_shift[2] = 0.0f;
	}
}

void LockedActorsList::clear()
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

extern "C" LockedActorsList* get_locked_actors_list()
{
	return new LockedActorsList();
}

extern "C" actor* get_self(LockedActorsList* list)
{
	return list->self();
}

extern "C" actor* get_actor_from_id(LockedActorsList* list, int actor_id)
{
	return list->get_actor_from_id(actor_id);
}

extern "C" actor* get_actor_and_attached_from_id(LockedActorsList* list, int actor_id,
	actor **attached)
{
	actor *act;
	std::tie(act, *attached) = list->get_actor_and_attached_from_id(actor_id);
	return act;
}

#ifdef ECDEBUGWIN
extern "C" actor* get_target(LockedActorsList* list)
{
	return list->get_target();
}
#endif // ECDEBUGWIN

extern "C" int add_attachment(LockedActorsList* list, int actor_id, actor *attached)
{
	return list->add_attachment(actor_id, attached);
}

extern "C" void remove_and_destroy_attachment(LockedActorsList* list, int actor_id)
{
	list->remove_and_destroy_attachment(actor_id);
}

extern "C" void for_each_actor(LockedActorsList* list,
	void (*fun)(actor*, void*, LockedActorsList*), void* data)
{
	list->for_each_actor(fun, data);
}

extern "C" void for_each_actor_and_attached(LockedActorsList* list,
	void (*fun)(actor*, actor*, void*, LockedActorsList*), void* data)
{
	list->for_each_actor_and_attached(fun, data);
}

extern "C" void release_locked_actors_list(LockedActorsList* list)
{
	delete list;
}

extern "C" LockedActorsList* lock_and_get_self(actor **self)
{
	auto list = new LockedActorsList();
	*self = list->self();
	if (!*self)
	{
		delete list;
		return nullptr;
	}
	return list;
}

extern "C" LockedActorsList* lock_and_get_actor_from_id(int actor_id, actor **act)
{
	auto list = new LockedActorsList();
	*act = list->get_actor_from_id(actor_id);
	if (!*act)
	{
		delete list;
		return nullptr;
	}
	return list;
}

extern "C" LockedActorsList* lock_and_get_actor_and_attached_from_id(int actor_id,
	actor **act, actor **attached)
{
	auto list = new LockedActorsList();
	std::tie(*act, *attached) = list->get_actor_and_attached_from_id(actor_id);
	if (!*act)
	{
		delete list;
		return nullptr;
	}
	return list;
}

extern "C" LockedActorsList* lock_and_get_actor_from_name(const char* name, actor **act)
{
	auto list = new LockedActorsList();
	*act = list->get_actor_from_name(name);
	if (!*act)
	{
		delete list;
		return nullptr;
	}
	return list;
}

extern "C" LockedActorsList* lock_and_get_nearest_actor(int tile_x, int tile_y,
	float max_distance, actor **act)
{
	auto list = new LockedActorsList();
	*act = list->get_nearest_actor(tile_x, tile_y, max_distance);
	if (!*act)
	{
		delete list;
		return nullptr;
	}
	return list;
}

#ifdef ECDEBUGWIN
extern "C" LockedActorsList* lock_and_get_target(actor **target)
{
	auto list = new LockedActorsList();
	*target = list->get_target();
	if (!*target)
	{
		delete list;
		return nullptr;
	}
	return list;
}
#endif

extern "C" void add_actor_to_list(actor* act, actor *attached)
{
	LockedActorsList list;
	list.add(act, attached);
}

extern "C" void remove_and_destroy_actor_from_list(int actor_id)
{
	LockedActorsList list;
	list.remove_and_destroy(actor_id);
}

extern "C" void remove_and_destroy_all_actors()
{
	LockedActorsList list;
	list.clear();
}

extern "C" int have_self()
{
	LockedActorsList list;
	return list.self() != nullptr;
}

extern "C" void set_self()
{
	LockedActorsList list;
	list.set_self();
}

extern "C" int actor_under_mouse_alive()
{
	LockedActorsList list;
	return list.actor_under_mouse_alive();
}

extern "C" int actor_occupies_tile(int x, int y)
{
	LockedActorsList list;
	return list.actor_occupies_tile(x, y);
}
