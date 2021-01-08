#ifndef __PASSWORD_MANAGER_H
#define __PASSWORD_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name External interface function for password manager.
 */
/*! @{ */
void passmngr_open_window(void);
void passmngr_destroy_window(void);
void passmngr_save_login(void);
void passmngr_set_login(void);
void passmngr_resize(void);
void passmngr_init(void);
void passmngr_destroy(void);
void passmngr_pending_pw_change(const char * old_and_new_password);
void passmngr_confirm_pw_change(void);
/*! @} */

extern int passmngr_enabled; /*!< if true login details are saved and selectable from login screen */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
