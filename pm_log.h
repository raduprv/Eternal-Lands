/*!
 * \file
 * \ingroup actor_utils
 * \brief handling of PMs and AFK messages.
 */
#ifndef __PM_LOG_H__
#define __PM_LOG_H__

#include "chat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_AFK_MINUTES 5

/*!
 * the afk_struct contains the messages that a player receives while AFK.
 */
typedef struct
{
        int msgs; /*!< number of messages arrived so far */
        char * name; /*!< name of the player */
        char ** messages; /*!< array containing all messages arrived so far */
} afk_struct;

/*!
 * Contains the number of message and the number of people that sent messages while AFK and a
 * list of all messages that arrived.
 */
struct pm_struct
{
        int msgs; /*!< number of message that arrived while AFK */
        int ppl; /*!< number of people that sent a messages while AFK */
        afk_struct * afk_msgs; /*!< array of \see afk_struct elements */
};

extern struct pm_struct pm_log;
extern int afk; /*!< flag indicating whether a player is currently AFK or not */
extern int last_action_time; /*!< timestamp of the last action for this player */
extern int afk_time; /*!< number of minutes after which the client will go AFK automatically. This can be set via the el.ini file. */
extern int afk_time_conf;
extern char afk_message[MAX_TEXT_MESSAGE_LENGTH]; /*!< buffer for the afk message used to inform other players that this player is currently AFK */
extern int afk_local;

/*!
 * \ingroup other
 * \brief Frees up the memory used by the pm log.
 *
 *      Frees up the memory used by the PM log.
 *
 */
void free_pm_log(void);

/*!
 * \ingroup actor_utils
 * \brief Sets the players status to AFK.
 *
 *      Sets the players status to AFK.
 *
 * \callgraph
 */
void go_afk(void);

/*!
 * \ingroup actor_utils
 * \brief Returns the players status from AFK to normal.
 *
 *      Returns the players status from AFK to normal.
 *
 * \callgraph
 */
void go_ifk(void);

/*!
 * \ingroup actor_utils
 * \brief Check for afk state changes and action change.
 *
 *      Check for afk state changes and action change.
 *
 * \callgraph
 */
void check_afk_state(void);

/*!
 * \ingroup actor_utils
 * \brief Adds the given message up to the speciifed length to the PM log.
 *
 *      Adds the given message msg, up to the specified length len to the PM log.
 *
 * \param msg   a string containing the message to add to the PM log.
 * \param len   the length of msg.
 * \param channel          the channel of message to send.
 *
 * \callgraph
 */
void add_message_to_pm_log (char *msg, int len, Uint8 channel);

/*!
 * \ingroup network_actors
 * \brief Sends an AFK message to a player
 *
 *      Sends an AFK message to a player to inform him about being AFK
 *
 * \param server_msg	string containing the name of the player 
 * \param len		the length of the string
 * \param channel          the channel of message to send.
 *
 * \callgraph
 */
void send_afk_message (const char *server_msg, int len, Uint8 channel);

/*!
 * \ingroup actor_utils
 * \brief Prints the specified message.
 *
 *     Prints the specified message from the users \see afk_struct.
 *
 * \param no    the number of the message to print
 */
void print_message(int no);

/*!
 * \ingroup network_actors
 * \brief is_talking_about_me
 *
 *      is_talking_about_me(Uint8*,int)
 *
 * \param server_msg
 * \param len
 * \retval int
 * \callgraph
 */
int is_talking_about_me (const char * server_msg, int len, char everywhere);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
