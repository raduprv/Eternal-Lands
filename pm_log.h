/*!
 * \file
 * \ingroup actor_utils
 * \brief handling of PMs and AFK messages.
 */
#ifndef __PM_LOG_H__
#define __PM_LOG_H__

#include "chat.h"

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

struct pm_struct pm_log;

extern int afk; /*!< flag indicating whether a player is currently AFK or not */
extern int last_action_time; /*!< timestamp of the last action for this player */
extern int afk_time; /*!< number of minutes after which the client will go AFK automatically. This can be set via the el.ini file. */
extern char afk_message[MAX_TEXT_MESSAGE_LENGTH]; /*!< buffer for the afk message used to inform other players that this player is currently AFK */

/*!
 * \ingroup other
 * \brief frees up the memory used by the pm log.
 *
 *      Frees up the memory used by the PM log.
 *
 */
void free_pm_log(void);

/*!
 * \ingroup actor_utils
 * \brief sets the players status to AFK.
 *
 *      Sets the players status to AFK.
 *
 * \callgraph
 */
void go_afk(void);

/*!
 * \ingroup actor_utils
 * \brief returns the players status from AFK to normal.
 *
 *      Returns the players status from AFK to normal.
 *
 * \callgraph
 */
void go_ifk(void);

/*!
 * \ingroup actor_utils
 * \brief adds the given message up to the speciifed length to the PM log.
 *
 *      Adds the given message msg, up to the specified length len to the PM log.
 *
 * \param msg   a string containing the message to add to the PM log.
 * \param len   the length of msg.
 *
 * \callgraph
 */
void add_message_to_pm_log(char * msg, int len);

/*!
 * \ingroup network_actors
 * \brief sends an AFK message to the server
 *
 *      Sends an AFK message to the server to inform it about a player going into AFK state.
 *
 * \param server_msg    a handle for the message to sent.
 * \param type          the type of message to sent.
 *
 * \callgraph
 */
void send_afk_message(Uint8 * server_msg, int type);

/*!
 * \ingroup actor_utils
 * \brief prints the specified message.
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
int is_talking_about_me(Uint8 * server_msg, int len);

#endif
