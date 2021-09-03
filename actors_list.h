#ifndef ACTORS_LIST_H
#define ACTORS_LIST_H

#ifdef __cplusplus
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
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
 * \brief Lock guard proxy
 *
 * This class provides a guard proxy for an object protected by a mutex. Objects of type
 * Locked<T, Mutex> can be dereferenced to a T object. When a Locked<T, Mutex> object falls out
 * of scope, the mutex protecting its contents is unlocked.
 */
template <typename T, typename Mutex>
class Locked
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new lock guard proxy for object \a obj, protected by mutex \a mutex. Calling this
	 * constructor locks the mutex, so it should be unlocked in the calling thread upon entry.
	 * \param obj   Reference to the object to protect
	 * \param mutex The mutex locking the object
	 */
	Locked(T& obj, Mutex& mutex): _mutex(mutex), _obj(obj) { LOCK(_mutex); }
	/*!
	 * \brief Constructor
	 *
	 * Create a new lock guard proxy for object \a obj, protected by mutex \a mutex. The mutex
	 * should already be locked by the calling thread when this constructor is called.
	 * \param obj   Reference to the object to protect
	 * \param mutex The mutex locking the object
	 */
	Locked(T& obj, Mutex& mutex, std::adopt_lock_t t): _mutex(mutex), _obj(obj) {}
	//! Destructor, unlocks the mutex
	~Locked() { _mutex.unlock(); }

	/*!
	 * \brief Access the guarded object
	 *
	 * Dereference the guard object, and return a constant reference to the protected object.
	 */
	const T& operator*() const { return _obj; }
	/*!
	 * \brief Access the guarded object
	 *
	 * Dereference the guard object, and return a mutable reference to the protected object.
	 */
	T& operator*() { return _obj; }
	/*!
	 * \brief Access member of the guarded object
	 *
	 * Dereference the guard object, and return a constant pointer to the protected object.
	 * This is used to access a member function or data field of the object.
	 */
	const T* operator->() const { return &_obj; }
	/*!
	 * \brief Access member of the guarded object
	 *
	 * Dereference the guard object, and return a mutable pointer to the protected object.
	 * This is used to access a member function or data field of the object.
	 */
	T* operator->() { return &_obj; }

private:
	//! Reference to the mutex locking the object
	Mutex& _mutex;
	//! Reference to the protected object
	T& _obj;
};


/*!
 * \brief A class for holding all actors
 *
 * Class ActorsList holds (pointers to) all actors in the client. It provides interfaces to
 * safely manipulate the list and the actors it holds, in a multi-threaded environment. Retrieving
 * an actor from the list by a thread is only possible when the thread holds the the associated
 * mutex. To gain this lock, there are two ways:
 * - The lock_and_get_*() family of function lock the actors list, and return the requested
 *   object. The caller is responsible for unlocking the list by calling release().
 * - The get() and get_self() methods return a guard proxy object that automatically unlocks the
 *   mutex when it goes out of scope.
 */
class ActorsList
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

	struct LockedList: public Locked<ActorsList, Mutex>
	{
		/*!
		 * Constructor
		 *
		 * Create a new lock guard for actors list \a list, protected by mutex \a mutex.
		 * \param list  The actors list to guard
		 * \param mutex The mutex serializing access to the actors list
		 */
		LockedList(ActorsList& list, Mutex& mutex): Locked<ActorsList, Mutex>(list, mutex) {}

		/*!
		 * \brief Execute a function for each actor in the actors list
		 *
		 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
		 * data \a data. The funtion must be callable as
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
			for (auto& id_act: (*this)->_list)
				fun(id_act.second, data, this);
		}
		/*!
		 * \brief Execute a function for each actor and attached actor in the actors list
		 *
		 * Execute function \a fun for each actor in the actors list, with a pointer to addtional
		 * data \a data. The funtion must be callable as
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
			for (auto& id_act: (*this)->_list)
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
		void add(actor* act, actor *attached) { (*this)->add_locked(act, attached); }
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
		bool add_attachment(int actor_id, actor *attached)
		{
			return (*this)->add_attachment_locked(actor_id, attached);
		}
		/*!
		 * \brief Remove an actor
		 *
		 * Remove the actor with ID \a actor_id from the actors list and free its resources. If the
		 * actor has an attached actor, it will also be removed and destroyed.
		 * \param actor_id The ID of the actor to remove, as set by the server
		 */
		void remove_and_destroy(int actor_id)
		{
			(*this)->remove_and_destroy_locked(actor_id);
		}
		/*!
		 * \brief Remove an attachment
		 *
		 * Remove the attached actor (horse) for the actor with ID \a actor_id, and free its
		 * resources. The parent actor is left in the list.
		 * \param actor_id The ID of the actor for which the destroy the attachment.
		 */
		void remove_and_destroy_attachment(int actor_id)
		{
			(*this)->remove_and_destroy_attachment_locked(actor_id);
		}
		/*!
		 * \brief Clear the actors list
		 *
		 * Clear the actors list, removing and destroying all actors contained within.
		 */
		void clear() { (*this)->clear_locked(); }

		/*!
		 * \brief Find yourself
		 *
		 * Return the player's own actor, if it exists. If the player's actor cannot be found,
		 * found, \c nullptr is returned.
		 * \return Pointer to the player's own actor
		 */
		actor* self() { return (*this)->_self; }
		/*!
		 * \brief Set the pointer to the player's own actor
		 *
		 * Set the pointer to the player's own actor, based on the global variable \a yourself,
		 * which holds the server ID of this character.
		 */
		void set_self()
		{
			(*this)->_self = get_actor_from_id(yourself);
		}

		/*!
		 * \brief Set the actor currently under the mouse
		 *
		 * Set the actor currently under the mouse cursor to that with ID \a actor_id.
		 * \param actor_id The server ID of the actor under the cursor
		 */
		void set_actor_under_mouse(int actor_id)
		{
			(*this)->_actor_under_mouse = get_actor_from_id(actor_id);
		}
		//! Clear the actor currently under the mouse
		void clear_actor_under_mouse()
		{
			(*this)->_actor_under_mouse = nullptr;
		}
		//! Return whether the actor under the mouse cursor is alive
		bool actor_under_mouse_alive()
		{
			return (*this)->_actor_under_mouse && !(*this)->_actor_under_mouse->dead;
		}

		/*!
		 * \brief Check if a tile is occupied
		 *
		 * Check if the tile at tile coordinates (\a x, \a y) is occupied by one of the actors in
		 * this list.
		 */
		bool actor_occupies_tile(int x, int y)
		{
			return std::find_if((*this)->_list.begin(), (*this)->_list.end(),
				[x, y](const std::pair<int, actor*>& id_act) {
					actor* act = id_act.second;
					return act->x_tile_pos == x && act->y_tile_pos == y;
				}
			) != (*this)->_list.end();
		}

		/*!
		 * \brief Find an actor by ID
		 *
		 * Lock the actors list, and return the actor identified by the server with ID
		 * \a actor_id. If no actor with ID \a actor_id can be found, \c nullptr is returned.
		 * \param actor_id The ID of the actor to look for
		 * \return Pointer to the actor
		 */
		actor* get_actor_from_id(int actor_id)
		{
			return (*this)->get_actor_from_id_locked(actor_id);
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
		std::pair<actor*, actor*> get_actor_and_attached_from_id(int actor_id)
		{
			return (*this)->get_actor_and_attached_from_id_locked(actor_id);
		}
		/*!
		 * \brief Find an actor by name
		 *
		 * Find the actor with name \a name (disregarding letter case). If no actor with name
		 * \a name can be found, \c nullptr is returned.
		 * \param name The name of the actor to look for
		 * \return Pointer to the requested actor
		 */
		actor* get_actor_from_name(const char* name)
		{
			return (*this)->get_actor_from_name_locked(name);
		}
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
		actor* get_nearest_actor(int tile_x, int tile_y, float max_distance)
		{
			return (*this)->get_nearest_actor_locked(tile_x, tile_y, max_distance);
		}
#ifdef ECDEBUGWIN
		/*!
		 * \brief Get a target for an eye candy effect
		 *
		 * Find an actor that is not the player's own, and return it as a target for an eye candy
		 * effect. Used in the eye candy debug window.
		 */
		actor* get_target()
		{
			return (*this)->get_target_locked();
		}
#endif
	};

	//! Return the singleton instance of the actors list
	static ActorsList& get_instance()
	{
		static ActorsList actors_list;
		return actors_list;
	}
	//! Return the a locked guard object for the actors list singleton instance
	static LockedList get_locked_instance()
	{
		return get_instance().lock();
	}
	/*!
	 * \brief Return the a locked guard object pointer for the actors list singleton instance
	 * \note This pointer must be \c delete'd after used to release the lock
	 */
	static LockedList* get_locked_instance_ptr()
	{
		return get_instance().lock_ptr();
	}

	//! Get a guarded proxy to this actors list
	LockedList lock() { return LockedList(*this, _mutex); }
	//! Get a guarded proxy to this actors list
	LockedList* lock_ptr() { return new LockedList(*this, _mutex); }

private:
	//! The list of all actors
	Storage _list;
	//! Mutex serializing access to the actors list
	Mutex _mutex;
	//! A pointer to the player's own actor
	actor* _self;
	//! A pointer to the actor currently under the mouse cursor
	actor* _actor_under_mouse;

	/*!
	 * \brief Constructor
	 *
	 * Create a new and empty actors list.
	 */
	ActorsList(): _list(), _mutex(), _self(nullptr),  _actor_under_mouse(nullptr) {}

	/*!
	 * \brief Find an actor by ID
	 *
	 * Return the actor identified by the server with ID \a actor_id. If no actor with ID
	 * \a actor_id can be found, \c nullptr is returned.
	 * \param actor_id The ID of the actor to look for
	 * \return Pointer to the actor
	 */
	actor *get_actor_from_id_locked(int actor_id)
	{
		Storage::iterator iter = _list.find(actor_id);
		return (iter != _list.end()) ? iter->second : nullptr;
	}
	/*!
	 * \brief Find an actor and its attached actor
	 *
	 * Lock the actors list, and find the actor with ID \a actor_id and its attached actor in the
	 * actors list. If the actor has no attachment, the second pointer returned will be a \c nullptr.
	 * If no actor with ID \a actor_id can be found, the first returned pointer will be \a nullptr.
	 * \param actor_id The ID of the actor to look for
	 * \return Pointers to the actors
	 */
	std::pair<actor*, actor*> get_actor_and_attached_from_id_locked(int actor_id);
	/*!
	 * \brief Find an actor by name
	 *
	 * Lock the actors list, and return the actor with name \a name (disregarding letter case). If
	 * no actor with name \a name can be found, \c nullptr is returned.
	 * \param name The name of the actor to look for
	 * \return Pointer to the actor
	 */
	actor* get_actor_from_name_locked(const char* name);
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
	actor* get_nearest_actor_locked(int tile_x, int tile_y, float max_distance);
#ifdef ECDEBUGWIN
	/*!
	 * \brief Get a target for an eye candy effect
	 *
	 * Find an actor that is not the player's own, and return it as a target for an eye candy
	 * effect. Used in the eye candy debug window.
	 */
	actor* get_target_locked();
#endif

	/*!
	 * \brief Add an actor
	 *
	 * Add actor \a act, with optional attached actor (horse) \a attached, to the actors list. The
	 * attachment \a attached can be \c nullptr, in which case the actor has no attachment.
	 * \param act      The actor to add to the list
	 * \param attached If not \c nullptr, the horse attached to \a act
	 */
	void add_locked(actor *act, actor *attached);
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
	bool add_attachment_locked(int actor_id, actor *attached);
	/*!
	 * \brief Remove and destroy a single actor
	 *
	 * Remove the actor at position \a iter in the actors list, and free its resources. No other
	 * actors are affected.
	 * \param iter Iterator pointing to the position of the actor to be removed
	 */
	void remove_and_destroy_single_locked(Storage::const_iterator iter);
	/*!
	 * \brief Remove and destroy an actor and its attachment
	 *
	 * Remove the actor at position \a iter in the actors list, and free its resources. If the
	 * actor has an attachment, it is also removed.
	 * \param iter Iterator pointing to the position of the actor to be removed
	 */
	void remove_and_destroy_locked(Storage::const_iterator iter);
	/*!
	 * \brief Remove and destroy an actor and its attachment
	 *
	 * Remove the actor with ID \a ator_id, and free its resources. If the actor has an attachment,
	 * it is also removed.
	 * \param actor_id The ID of the actor to be removed
	 */
	void remove_and_destroy_locked(int actor_id);
	/*!
	 * \brief Remove an attachment
	 *
	 * Remove the attached actor (horse) for the actor with ID \a actor_id, and free its resources.
	 * The parent actor is left in the list.
	 * \param actor_id The ID of the actor for which the destroy the attachment.
	 */
	void remove_and_destroy_attachment_locked(int actor_id);
	/*!
	 * \brief Clear the actors list
	 *
	 * Clear the actors list, removing and destroying all actors contained within.
	 */
	void clear_locked();
};

} // namespace eternal_lands

typedef eternal_lands::ActorsList::LockedList locked_list;
typedef locked_list *locked_list_ptr;

extern "C"
{
#endif // __cplusplus

#ifndef __cplusplus
typedef struct locked_list *locked_list_ptr;
#endif // __cplusplus

locked_list_ptr get_locked_actors_list(void);
actor* get_self(locked_list_ptr list);
actor* get_actor_from_id(locked_list_ptr list, int actor_id);
actor* get_actor_and_attached_from_id(locked_list_ptr list, int actor_id, actor **attached);
#ifdef ECDEBUGWIN
actor* get_target(locked_list_ptr list);
#endif // ECDEBUGWIN
void for_each_actor(locked_list_ptr list, void (*fun)(actor*, void*, locked_list_ptr), void* data);
void for_each_actor_and_attached(locked_list_ptr list,
	void (*fun)(actor*, actor*, void*, locked_list_ptr), void* data);
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

void add_actor_to_list(actor *act, actor *attached);
void remove_and_destroy_actor_from_list(int actor_id);
int add_attachment_to_list(int actor_id, actor *attached);
void remove_and_destroy_attachment_from_list(int actor_id);
void remove_and_destroy_all_actors(void);
int have_self(void);
void set_self(void);
float self_scale(void);
int actor_under_mouse_alive(void);
int actor_occupies_tile(int x, int y);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



#endif // ACTORS_LIST_H
