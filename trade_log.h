#ifndef __TRADE_LOG_H
#define __TRADE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "trade.h"

// The trade log mode, set by the options window, as described in the enum
extern int trade_log_mode;
extern int enable_trade_log;	// TODO temporary code delete in a bit - this is a hiden option
enum { TRADE_LOG_NONE = 0, TRADE_LOG_CONSOLE, TRADE_LOG_FILE, TRADE_LOG_BOTH };

/**
 * @ingroup trade_log
 * @brief We are sending the server "Accept" for the second time.
 *
 * @param name	the name of the person we are trading with
 * @param yours	the list of items we are trading
 * @param others	the list of items they are trading with us
 * @param max_items	the maximum size of the item lists
 * @callgraph
 */
void trade_accepted(const char *name, const trade_item *yours, const trade_item *others, int max_items);

/**
 * @ingroup trade_log
 * @brief server has sent the GET_TRADE_EXIT command.
 *
 * @callgraph
 */
void trade_exit(void);

/**
 * @ingroup trade_log
 * @brief server has sent the trade window abort text message
 *
 * @param message	The text of the message
 * @callgraph
 */
void trade_aborted(const char *message);

/**
 * @ingroup trade_log
 * @brief server has sent the STORAGE_ITEMS command.
 *
 * @callgraph
 */
void trade_post_storage(void);

/**
 * @ingroup trade_log
 * @brief server has sent the HERE_YOUR_INVENTORY command.
 *
 * @callgraph
 */
void trade_post_inventory(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
