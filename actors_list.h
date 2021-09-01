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
	typedef std::vector<actor*> Storage;

	/*!
	 * \brief Lock guard proxy
	 *
	 * This class provides a guard proxy for an object protected by a mutex. Objects of type
	 * Locked<T> can be dereferenced to a T object. When a Locked<T> object falls out of scope,
	 * the mutex protecting its contents is unlocked.
	 */
	template <typename T>
	class Locked
	{
	public:
		/*!
		 * \brief Constructor
		 *
		 * Create a new lock guard proxy for object \a obj, protected by mutex \a mutex. The
		 * mutex must be locked when the constructor is called.
		 * \param obj   Reference to the object to protect
		 * \param mutex The mutex locking the object
		 */
		Locked(T& obj, Mutex& mutex): _mutex(mutex), _obj(obj) {}
		//! Destructor, unlocks the mutex
		~Locked()
		{
			_mutex.unlock();
		}

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

	struct LockedList: public Locked<ActorsList>
	{
		LockedList(ActorsList& list, Mutex& mutex): Locked<ActorsList>(list, mutex) {}

		void add(actor* act, actor *attached) { (*this)->add_locked(act, attached); }
		bool add_attachment(int actor_id, actor *attached)
		{
			return (*this)->add_attachment_locked(actor_id, attached);
		}
		void remove_and_destroy(int actor_id) { (*this)->remove_and_destroy_locked(actor_id); }
		void clear() { (*this)->clear_locked(); }

		actor* get_self() { return (*this)->_self; }
		actor* get_actor_from_id(int actor_id) { return (*this)->get_actor_from_id_locked(actor_id); }
		std::pair<actor*, actor*> get_self_and_actor_from_id(int actor_id)
		{
			return (*this)->get_self_and_actor_from_id_locked(actor_id);
		}
		std::pair<actor*, actor*> get_actor_and_attached_from_id(int actor_id)
		{
			return (*this)->get_actor_and_attached_from_id_locked(actor_id);
		}
	};
	//! Type definition for an actor pointer protected by the actors list mutex
	typedef Locked<actor*> LockedActorPtr;

	//! Return the singleton instance of the actors list
	static ActorsList& get_instance()
	{
		static ActorsList actors_list;
		return actors_list;
	}

	//! Get a guarded proxy to this actors list
	LockedList get();
	//! Get a guarded proxy to the player's own actor
	LockedActorPtr get_self();

	Storage& lock_and_get_actors_list();
	/*!
	 * \brief Find yourself
	 *
	 * Lock the actors list, and return the player's own actor. If the player's actor cannot be
	 * found, \c nullptr is returned and the list is unlocked.
	 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
	 *       release().
	 * \note On an error return (result is \c nullptr), the actors list is already unlocked
	 * \return Pointer to the actor
	 */
	actor* lock_and_get_self();
	/*!
	 * \brief Find an actor by ID
	 *
	 * Lock the actors list, and return the actor identified by the server with ID \a actor_id. If
	 * no actor with ID \a actor_id can be found, \c nullptr is returned and the list is unlocked.
	 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
	 *       release().
	 * \note On an error return (result is \c nullptr), the actors list is already unlocked
	 * \param actor_id The ID of the actor to look for
	 * \return Pointer to the actor
	 */
	actor* lock_and_get_actor_from_id(int actor_id);
	/*!
	 * \brief Find self and another actor
	 *
	 * Lock the actors list, and find the actor with ID \a actor_id in the actors list. Return
	 * a pair of pointers (self, act), where the first is a pointer to the player's own actor,
	 * and the second is the requested actor. If either actor cannot be found, a pair of
	 * \c nullptr's is returned and the list is unlocked.
	 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
	 *       release().
	 * \note On an error return (both pointers are \c nullptr), the actors list is already unlocked
	 * \param actor_id The ID of the actor to look for
	 * \return Pointers to the actors
	 */
	std::pair<actor*, actor*> lock_and_get_self_and_actor_from_id(int actor_id);
	/*!
	 * \brief Find an actor and its attached actor
	 *
	 * Lock the actors list, and find the actor with ID \a actor_id and its attached actor in the
	 * actors list. If the actor has no attachment, the second pointer returned will be a \c nullptr.
	 * If no actor with ID \a actor_id can be found, the first returned pointer will be \a nullptr,
	 * and the list is unlocked.
	 * \note On succesful return, the caller is responsible for unlocking the actors list by calling
	 *       release().
	 * \note On an error return (first pointer is \c nullptr), the actors list is already unlocked
	 * \param actor_id The ID of the actor to look for
	 * \return Pointers to the actors
	 */
	std::pair<actor*, actor*> lock_and_get_actor_and_attached_from_id(int actor_id);
	std::pair<actor*, actor*> lock_and_get_actor_pair_from_id(int actor_id1, int actor_id2);
	actor* lock_and_get_actor_from_name(const char* name);
	actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance);
	std::pair<Storage&, actor*> lock_and_get_list_and_self();
#ifdef ECDEBUGWIN
	actor* lock_and_get_target();
	std::pair<actor*, actor*> lock_and_get_self_and_target();
#endif

	/*!
	 * \brief Release the mutex
	 *
	 * Release the mutex on the actors list. To be called after locking the mutex using one of
	 * the lock_and_get_*() methods.
	 */
	void release()
	{
		_mutex.unlock();
	}

	/*!
	 * \brief Add an actor
	 *
	 * Add actor \a act, with optional attached actor (horse) \a attached, to the actors list. The
	 * attachment \a attached can be \c nullptr, in which case the actor has no attachment.
	 * \note The actors list mutex should not be locked when calling this method.
	 * \param act      The actor to add to the list
	 * \param attached If not \c nullptr, the horse attached to \a act
	 */
	void add(actor *act, actor *attached);
	/*!
	 * \brief Add an attached actor
	 *
	 * Add an attached actor \a attached (presumably a horse) to the actor identified by actor
	 * ID \a actor_id.
	 * \note The actors list mutex should not be locked when calling this method.
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
	 * \note The actors list mutex should not be locked when calling this method.
	 * \param actor_id The ID of the actor to remove, as set by the server
	 */
	void remove_and_destroy(int actor_id);
	/*!
	 * \brief Remove an attachment
	 *
	 * Remove the attached actor (horse) for the actor with ID \a actor_id, and free its resources.
	 * The parent actor is left in the list.
	 * \note The actors list mutex should not be locked when calling this method.
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
	 * \brief Check if we have ourselves
	 *
	 * Check if the player's own actor exists
	 */
	bool have_self();
	/*!
	 * \brief Set the pointer to the player's own actor
	 *
	 * Set the pointer to the player's own actor, based on the global variable \a yourself,
	 * which holds the server ID of this character.
	 */
	void set_self();

	/*!
	 * \brief Set the actor currently under the mouse
	 *
	 * Set the actor currently under the mouse cursor to that with ID \a actor_id.
	 * \param actor_id The server ID of the actor under the cursor
	 */
	void set_actor_under_mouse(int actor_id);
	//! Clear the actor currently under the mouse
	void clear_actor_under_mouse();
	//! Return whether the actor under the mouse cursor is alive
	bool actor_under_mouse_alive();

	/*!
	 * \brief Check if a tile is occupied
	 *
	 * Check if the tile at tile coordinates (\a x, \a y) is occupied by one of the actors in this
	 * list.
	 */
	bool actor_occupies_tile(int x, int y);

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
	actor *get_actor_from_id_locked(int actor_id);
	/*!
	 * \brief Find self and another actor
	 *
	 * Lock the actors list, and find the actor with ID \a actor_id in the actors list. Return
	 * a pair of pointers (self, act), where the first is a pointer to the player's own actor,
	 * and the second is the requested actor. If either actor cannot be found, a pair of
	 * \c nullptr's is returned.
	 * \param actor_id The ID of the actor to look for
	 * \return Pointers to the actors
	 */
	std::pair<actor*, actor*> get_self_and_actor_from_id_locked(int actor_id);
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

	size_t find_index_for_id(int actor_id);

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
	actor* remove_at_index_locked(size_t idx);
	void remove_and_destroy_locked(int actor_id);
	void remove_and_destroy_at_index_locked(size_t idx);
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
actor* lock_and_get_nearest_actor(int tile_x, int tile_y, float max_distance);
actor** lock_and_get_list_and_self(size_t *len, actor **self);
#ifdef ECDEBUGWIN
actor* lock_and_get_target(void);
actor* lock_and_get_self_and_target(actor **target);
#endif
void add_actor_to_list(actor *act, actor *attached);
void remove_and_destroy_actor_from_list(int actor_id);
int add_attachment_to_list(int actor_id, actor *attached);
void remove_and_destroy_attachment_from_list(int actor_id);
void remove_and_destroy_all_actors(void);
int have_self(void);
void set_self(void);
float self_scale(void);
void clear_actor_under_mouse(void);
int actor_under_mouse_alive(void);
int actor_occupies_tile(int x, int y);

static inline actor* find_actor_ptr(actor **actors_list, size_t max_actors, int actor_id)
{
	for (size_t i = 0; i < max_actors; ++i)
	{
		if (actors_list[i]->actor_id == actor_id)
			return actors_list[i];
	}
	return NULL;
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



#endif // ACTORS_LIST_H
