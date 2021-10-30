#ifndef ACTORS_LIST_H
#define ACTORS_LIST_H

#ifdef __cplusplus
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>

// Forward declaration
namespace eternal_lands
{
	class LockedActorsList;
}
typedef eternal_lands::LockedActorsList *locked_list_ptr;

#endif // __cplusplus

#include "actors.h"

#ifdef __cplusplus

#ifdef ACTORS_LIST_MUTEX_DEBUG
namespace
{

template <typename Mutex>
void did_we_deadlock(Mutex& mutex)
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

/*!
 * \brief A lock guard class for the actors list
 *
 * Class LockedActorsList provides a lock guard object to safely manipulate the actors list. Access
 * to the actors list can only be obtained by creating a LockedActorsList object, which will lock
 * the mutex associated with the list. When the LockedActorsList object falls out of scope, its
 * destructor automatically releases the mutex.
 */
class LockedActorsList
{
public:
	// The type of mutex used depends on the compile options:
	// - When ACTORS_LIST_MUTEX_DEBUG is set, a timed mutex is used, to detect deadlocks. If
	//   the mutex cannot be obtained for more than 1 second, did_we_deadlock() is called, which
	//   prints an error message, then tries to lock again. This is used for debugging, so that
	//   we can set a breakpoint on did_we_deadlock() and see how the deadlock occurs. Note that
	//   the lock still proceeds, so if a deadlock occurs, the client will become unresponsive.
	// - By default a recursive mutex is used, which allows the same thread to lock the mutex
	//   multiple times. This is in fact necessary at the moment, as in at least in deeply nested
	//   call site in the sound code, the client can try to lock the list while it is already
	//   locked. Set ACTORS_LIST_NO_RECURSIVE_MUTEX to use a regular mutex and debug these
	//   situations.
	// The default is to use a recursive mutex without timeout.
#ifdef ACTORS_LIST_MUTEX_DEBUG
 #ifdef ACTORS_LIST_NO_RECURSIVE_MUTEX
	//! A regular mutex with a timeout
	typedef std::timed_mutex Mutex;
 #else // ACTORS_LIST_NO_RECURSIVE_MUTEX
	//! A recursive mutex with a timeout
	typedef std::recursive_timed_mutex Mutex;
 #endif // ACTORS_LIST_NO_RECURSIVE_MUTEX
#else // ACTORS_LIST_MUTEX_DEBUG
 #ifdef ACTORS_LIST_NO_RECURSIVE_MUTEX
	//! A regular mutex, without timeout
	typedef std::mutex Mutex;
 #else // ACTORS_LIST_NO_RECURSIVE_MUTEX
	//! A recursive mutex, without timeout.
	typedef std::recursive_mutex Mutex;
 #endif // ACTORS_LIST_NO_RECURSIVE_MUTEX
#endif // ACTORS_LIST_MUTEX_DEBUG
	typedef std::unordered_map<int, actor*> Storage;

	/*!
	 * \brief Constructor
	 *
	 * Create a new lock guard for the actors list. This locks the associated mutex.
	 */
	LockedActorsList() { LOCK(_mutex); }
	/*!
	 * \brief Destructor
	 *
	 * Destroys the lock guard for the actors list. This unlocks the associated mutex.
	 */
	~LockedActorsList() { _mutex.unlock(); }

	/*!
	 * \brief Execute a function for each actor in the actors list
	 *
	 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
	 * data \a data. The function must be callable as
	 * \code{.cpp}
	 * fun(actor *act, void* data, locked_list_ptr list);
	 * \endcode
	 * where the first argument is a pointer to the actor, the second the \a data pointer
	 * passed to this function, and the final argument is a pointer to this proxy object.
	 * \param fun  The function to execute
	 * \param data Additional data to pass to \a fun
	 */
	template <typename T>
	void for_each_actor(T fun, void* data)
	{
		for (auto& id_act: _list)
			fun(id_act.second, data, this);
	}
	/*!
	 * \brief Execute a function for each actor and attached actor in the actors list
	 *
	 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
	 * data \a data. The function must be callable as
	 * \code{.cpp}
	 * fun(actor *act, actor *attached, void* data, locked_list_ptr list);
	 * \endcode
	 * where the first argument is a pointer to the actor, the secon a pointer to its attached
	 * actor (or \a nullptr if there is no attachment), the third the \a data pointer
	 * passed to this function, and the final argument is a pointer to this proxy object.
	 * \param fun  The function to execute
	 * \param data Additional data to pass to \a fun
	 */
	template <typename T>
	void for_each_actor_and_attached(T fun, void* data)
	{
		for (auto& id_act: _list)
		{
			actor* act = id_act.second;
			actor *attached = has_attachment(act)
				? get_actor_from_id(act->attached_actor_id)
				: nullptr;
			fun(act, attached, data, this);
		}
	}

	/*!
	 * \brief Add an actor
	 *
	 * Add actor \a act, with optional attached actor (horse) \a attached, to the actors list.
	 * The attachment \a attached can be \c nullptr, in which case the actor has no attachment.
	 * \param act      The actor to add to the list
	 * \param attached If not \c nullptr, the horse attached to \a act
	 */
	void add(actor* act, actor *attached);
	/*!
	 * \brief Add an attached actor
	 *
	 * Add an attached actor \a attached (presumably a horse) to the actor identified by actor
	 * ID \a actor_id.
	 * \param actor_id The ID of the actor to receive an attachment, as sent by the server
	 * \param attached The actor to attach
	 * \return \c true if the attachment succeeded, \c false if it failed (probably because the
	 * 	parent actor could not be found).
	 */
	bool add_attachment(int actor_id, actor *attached);
	/*!
	 * \brief Remove an actor
	 *
	 * Remove the actor with ID \a actor_id from the actors list and free its resources. If the
	 * actor has an attached actor, it will also be removed and destroyed.
	 * \param actor_id The ID of the actor to remove, as set by the server
	 */
	void remove_and_destroy(int actor_id);
	/*!
	 * \brief Remove an attachment
	 *
	 * Remove the attached actor (horse) for the actor with ID \a actor_id, and free its
	 * resources. The parent actor is left in the list.
	 * \param actor_id The ID of the actor for which the destroy the attachment.
	 */
	void remove_and_destroy_attachment(int actor_id);
	/*!
	 * \brief Clear the actors list
	 *
	 * Clear the actors list, removing and destroying all actors contained within.
	 */
	void clear();

	/*!
	 * \brief Find yourself
	 *
	 * Return the player's own actor, if it exists. If the player's actor cannot be found,
	 * found, \c nullptr is returned.
	 * \return Pointer to the player's own actor
	 */
	actor* self() { return _self; }
	/*!
	 * \brief Set the pointer to the player's own actor
	 *
	 * Set the pointer to the player's own actor, based on the global variable \a yourself,
	 * which holds the server ID of this character.
	 */
	void set_self()
	{
		_self = get_actor_from_id(yourself);
	}

	/*!
	 * \brief Set the actor currently under the mouse
	 *
	 * Set the actor currently under the mouse cursor to that with ID \a actor_id.
	 * \param actor_id The server ID of the actor under the cursor
	 */
	void set_actor_under_mouse(int actor_id)
	{
		_actor_under_mouse = get_actor_from_id(actor_id);
	}
	//! Clear the actor currently under the mouse
	void clear_actor_under_mouse()
	{
		_actor_under_mouse = nullptr;
	}
	//! Return whether the actor under the mouse cursor is alive
	bool actor_under_mouse_alive()
	{
		return _actor_under_mouse && !_actor_under_mouse->dead;
	}

	/*!
		* \brief Check if a tile is occupied
		*
		* Check if the tile at tile coordinates (\a x, \a y) is occupied by one of the actors in
		* this list.
		*/
	bool actor_occupies_tile(int x, int y)
	{
		return std::find_if(_list.begin(), _list.end(),
			[x, y](const std::pair<int, actor*>& id_act) {
				actor* act = id_act.second;
				return act->x_tile_pos == x && act->y_tile_pos == y;
			}
		) != _list.end();
	}

	/*!
	 * \brief Find an actor by ID
	 *
	 * Return the actor identified by the server with ID \a actor_id. If no actor with ID
	 * \a actor_id can be found, \c nullptr is returned.
	 * \param actor_id The ID of the actor to look for
	 * \return Pointer to the actor
	 */
	actor* get_actor_from_id(int actor_id)
	{
		Storage::iterator iter = _list.find(actor_id);
		return (iter != _list.end()) ? iter->second : nullptr;
	}
	/*!
	 * \brief Find an actor and its attached actor
	 *
	 * Find the actor with ID \a actor_id and its attached actor in the actors list. If the
	 * actor has no attachment, the second pointer returned will be a \c nullptr. If no actor
	 * with ID \a actor_id can be found, the first returned pointer will be \a nullptr.
	 * \param actor_id The ID of the actor to look for
	 * \return Pointers to the actors
	 */
	std::pair<actor*, actor*> get_actor_and_attached_from_id(int actor_id);
	/*!
	 * \brief Find an actor by name
	 *
	 * Find the actor with name \a name (disregarding letter case). If no actor with name
	 * \a name can be found, \c nullptr is returned.
	 * \param name The name of the actor to look for
	 * \return Pointer to the requested actor
	 */
	actor* get_actor_from_name(const char* name);
	/*!
	 * \brief Find actor nearest to a position
	 *
	 * Find the actor nearest to tile position (\a tile_x, \a tile_y) that can be attacked,
	 * if it is within a distance \a max_distance, and return it. If no attackable actor can
	 * be found within the distance, a \c nullptr is returned.
	 * \param x_tile       x-coordinate of the tile to search around
	 * \param y_tile       y-coordinate of the tile to search around
	 * \param max_distance Maximum search distance
	 * \return Pointer to the nearest attackable actor
	 */
	actor* get_nearest_actor(int tile_x, int tile_y, float max_distance);
#ifdef ECDEBUGWIN
	/*!
	 * \brief Get a target for an eye candy effect
	 *
	 * Find an actor that is not the player's own, and return it as a target for an eye candy
	 * effect. Used in the eye candy debug window.
	 */
	actor* get_target();
#endif

private:
	//! The list of all actors
	static Storage _list;
	//! Mutex serializing access to the actors list
	static Mutex _mutex;
	//! A pointer to the player's own actor
	static actor* _self;
	//! A pointer to the actor currently under the mouse cursor
	static actor* _actor_under_mouse;

	/*!
	 * \brief Remove and destroy an actor and its attachment
	 *
	 * Remove the actor at position \a iter in the actors list, and free its resources. If the
	 * actor has an attachment, it is also removed.
	 * \param iter Iterator pointing to the position of the actor to be removed
	 */
	void remove_and_destroy(Storage::const_iterator iter);
	/*!
	 * \brief Remove and destroy a single actor
	 *
	 * Remove the actor at position \a iter in the actors list, and free its resources. No other
	 * actors are affected.
	 * \param iter Iterator pointing to the position of the actor to be removed
	 */
	void remove_and_destroy_single(Storage::const_iterator iter);
};

} // namespace eternal_lands

extern "C"
{
#endif // __cplusplus

/*!
 * \brief Return a pointer to the locked actors list
 *
 * Lock the actors list, and return a pointer to a proxy guard object. The pointer should be
 * treated as an opaque pointer in C code. After using the pointer, the lock on the list should
 * be released by freeing the guard object using release_locked_actors_list()
 * \return Pointer to the locked actors list
 */
locked_list_ptr get_locked_actors_list(void);
/*!
 * \brief Find yourself
 *
 * Return the player's own actor, if it exists. If the player's actor cannot be found,
 * found, \c nullptr is returned.
 * \param list Pointer to the locked actors list
 * \return Pointer to the player's own actor
 */
actor* get_self(locked_list_ptr list);
/*!
 * \brief Find an actor by ID
 *
 * Return the actor identified by the server with ID \a actor_id from the actors list \a list. If
 * no actor with ID \a actor_id can be found, \c nullptr is returned.
 * \param list     Pointer to the locked actors list
 * \param actor_id The ID of the actor to look for
 * \return Pointer to the actor
 */
actor* get_actor_from_id(locked_list_ptr list, int actor_id);
/*!
 * \brief Find an actor and its attached actor
 *
 * Find the actor with ID \a actor_id and its attached actor in the actors list \a list. If the
 * actor has no attachment, the \a attached will be set to a \c nullptr. If no actor with
 * ID \a actor_id can be found, returned pointer will be \a nullptr.
 * \param list     Pointer to the locked actors list
 * \param actor_id The ID of the actor to look for
 * \param attached Place to stor a pointer to the attached actor
 * \return Pointer to the actor with ID \a actor_id
 */
actor* get_actor_and_attached_from_id(locked_list_ptr list, int actor_id, actor **attached);
#ifdef ECDEBUGWIN
/*!
 * \brief Get a target for an eye candy effect
 *
 * Find an actor that is not the player's own, and return it as a target for an eye candy
 * effect. Used in the eye candy debug window.
 * \param list Pointer to the locked actors list
 */
actor* get_target(locked_list_ptr list);
#endif // ECDEBUGWIN
/*!
 * \brief Add an attached actor to the list
 *
 * Add \a attached as an attachment to the actor with ID \a actor_id to the actors list \a list. If
 * the actor already has an attachment, the addition fails.
 * \param list     Pointer to the locked actors list
 * \param actor_id The ID of the actor to receive the attachment
 * \param attached Pointer to the attached actor
 * \return 1 if the addition succeeded, 0 if it failed
 */
int add_attachment(locked_list_ptr list, int actor_id, actor *attached);
/*!
 * \brief Remove an attached actor from the actors list
 *
 * Remove the attachment from the actor with ID \a actor_id (if any) from the actors list \a list,
 * and free its resources.
 * \note \a actor_id is the (server) ID of the owner of the attachment (i.e. the player), not the
 *       fake ID generated by the client for the attached actor itself.
 * \param actor_id The ID of the actor for which to remove the attachment.
 */
void remove_and_destroy_attachment(locked_list_ptr list, int actor_id);
/*!
 * \brief Execute a function for each actor in the actors list
 *
 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
 * data \a data. The function is called with the pointer to the actor as first argument,
 * \a data as the second argument, and \a list as the third.
 * \param list Pointer to the locked actors list
 * \param fun  The function to execute
 * \param data Additional data to pass to \a fun
 */
void for_each_actor(locked_list_ptr list, void (*fun)(actor*, void*, locked_list_ptr), void* data);
/*!
 * \brief Execute a function for each actor and attached actor in the actors list
 *
 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
 * data \a data. The function is called with a pointer to the actor as the first argument,
 * a pointer to it attachment as the second, \a data as the third, and \a list as the final
 * argument.
 * \param list Pointer to the locked actors list
 * \param fun  The function to execute
 * \param data Additional data to pass to \a fun
 */
void for_each_actor_and_attached(locked_list_ptr list,
	void (*fun)(actor*, actor*, void*, locked_list_ptr), void* data);
/*!
 * \brief Release the actors list mutex
 *
 * Free the list guard object \a list, releasing the actors list mutex. After calling this function,
 * \a list should no longer be used.
 * \param list Pointer to an actors list guard object
 */
void release_locked_actors_list(locked_list_ptr list);


/*!
 * \brief Find yourself
 *
 * Lock the actors list, and return the player's own actor. If the player's actor cannot be
 * found, \c nullptr is returned and the actors list is unlocked.
 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
 *       release_locked_actors_list().
 * \note On an error return (result is \c nullptr), the actors list is already unlocked
 * \param self Place t store the player's own actor
 * \return Pointer to the actors list
 */
locked_list_ptr lock_and_get_self(actor **self);
/*!
 * \brief Find an actor by ID
 *
 * Lock the actors list, and return the actor identified by the server with ID \a actor_id. If
 * no actor with ID \a actor_id can be found, \c nullptr is returned and the list is unlocked.
 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
 *       release_locked_actors_list().
 * \note On an error return (result is \c nullptr), the actors list is already unlocked
 * \param actor_id The ID of the actor to look for
 * \param act      Place to store the pointer to the actor
 * \return Pointer to the actors list
 */
locked_list_ptr lock_and_get_actor_from_id(int actor_id, actor **act);
/*!
 * \brief Find an actor and its attached actor
 *
 * Lock the actors list, and find the actor with ID \a actor_id and its attached actor in the
 * actors list. If the actor has no attachment, the \a attached will be set to \c nullptr.
 * If no actor with ID \a actor_id can be found, the function will return \a nullptr,
 * and the actors list is unlocked.
 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
 *       release_locked_actors_list().
 * \note On an error return (result is \c nullptr), the actors list is already unlocked
 * \param actor_id The ID of the actor to look for
 * \param act      Place to store pointer to the actor itself
 * \param attached Place to store pointer to the actor attached to \a act
 * \return Pointers to the actors list
*/
locked_list_ptr lock_and_get_actor_and_attached_from_id(int actor_id, actor **act,
	actor **attached);
/*!
 * \brief Find an actor by name
 *
 * Lock the actors list, and find the actor with name \a name (disregarding letter case). If
 * successful, this function returnsa pointer to the actors list, and set \a act to the requested
 * actor. Ifno actor with name \a name can be found, \c nullptr is returned and the actors list is
 * unlocked.
 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
 *       release_locked_actors_list().
 * \note On an error return (result is \c nullptr), the actors list is already unlocked
 * \param name The name of the actor to look for
 * \param act  Place to store the pointer to the actor
 * \return Pointer to the actors list
 */
locked_list_ptr lock_and_get_actor_from_name(const char* name, actor **act);
/*!
 * \brief Find actor nearest to a position
 *
 * Lock the actors list, and find the actor nearest to tile position (\a tile_x, \a tile_y)
 * that can be attacked and is within a distance \a max_distance. If successful, \a act is
 * set to the requested actor. If no attackable actor can be found within the distance, a
 * \c nullptr is returned, and the actors list is unlocked.
 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
 *       release_locked_actors_list().
 * \note On an error return (result is \c nullptr), the actors list is already unlocked
 * \param x_tile       x-coordinate of the tile to search around
 * \param y_tile       y-coordinate of the tile to search around
 * \param max_distance Maximum search distance
 * \param act          Place to store the pointer to the actor
 * \return Pointer to the actors list
 */
locked_list_ptr lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance,
	actor **act);
#ifdef ECDEBUGWIN
/*!
 * \brief Get a target for an eye candy effect
 *
 * Lock the actors list, and find an actor that is not the player's own as a target for an
 * eye candy effect. On success the actor is stored in \a target. If no actor can be found
 * (except possibly the player itself), \c nullptr is returned, and the actors list is unlocked.
 * \param target Place to store the pointer to the target actor
 * \return Pointer to the actors list
 */
locked_list_ptr lock_and_get_target(actor **target);
#endif


/*!
 * \brief Add a new actor to the actors list
 *
 * Add actor \a act to the actors list. If \a attached is not \c NULL, it is also added as an
 * attached actor to \a act.
 * \note If an actor with the same actor ID as \a act, or a player actor with the same name as
 *       \a act, already exists, the old actor is removed first.
 * \param act      Pointer to the actor to add to the list
 * \param attached If not nill, the attachment (horse) of \a act
 */
void add_actor_to_list(actor *act, actor *attached);
/*!
 * \brief Remove an actor from the actors list
 *
 * Remove the actor with ID \a actor_id from the actors list, and free up its resources. If the
 * actor has an attachment, it is also removed an destroyed.
 * \param actor_id The server ID of the actor to remove.
 */
void remove_and_destroy_actor_from_list(int actor_id);
//! Remove all actors from the actors list and free up the resources they use
void remove_and_destroy_all_actors(void);
/*!
 * \brief Check if the player's own actor is set
 *
 * Check if the player's own actor exists in the actors list.
 * \note Use with caution: because the actors list mutex is dropped when leaving the function,
 *       the actor may be removed by another thread at any time, invalidating the result. Use
 *       this function only for a quick return. When you need to be sure that the pointer to the
 *       player's actor remains available, use e.g. lock_and_get_self().
 */
int have_self(void);
/*!
 * \brief Set the pointer to the player's own actor
 *
 * Set the pointer to the player's own actor, based on the global variable \a yourself,
 * which holds the server ID of this character.
 */
void set_self(void);
/*!
 * \brief Check if the actor under the mouse is alive
 *
 * Check if there is currently an actor under the mouse cursor, and if so, whether it is still
 * alive. This is used to determine which cursor to use when hovering over the actor.
 * \note Use with caution: this function drops the lock on the actors list upon exit, after which
 *       it is possible that an actor dies -- resurrection is rarely seen in EL -- before the
 *       result is used. Use only when the result does not strictly need to be correct.
 */
int actor_under_mouse_alive(void);
/*!
 * \brief Check if a tile is occupied
 *
 * Check if the tile at position (\a x, \a y) is occupied by an actor at the time of calling.
 * \note Use with caution: this function drops the lock on the actors list upon exit, after which
 *       it is possible that an actor moves onto or out of the square just checked. Use only when
 *       the result does not strictly need to be correct (e.g. in the pathfinder, where the
 *       requested path is checked by the server).
 * \param x The x coordinate of the tile to check
 * \param y The y coordinate of the tile to check
 * \return Whether the tile was occupied by an actor
 */
int actor_occupies_tile(int x, int y);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



#endif // ACTORS_LIST_H
