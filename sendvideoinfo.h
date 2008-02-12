#ifndef	_CHECKEXTENTIONS_H_
#define	_CHECKEXTENTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif


extern int video_info_sent;

/**
 * @brief Sends the supported OpenGL extentions to the server.
 *
 * Sends the supported OpenGL extentions to the server and some other usefull OpenGL information to
 * the server.
 */
void send_video_info();


#ifdef __cplusplus
} // extern "C"
#endif

#endif	// _CHECKEXTENTIONS_H_
