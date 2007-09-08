/**
 * @file
 * @ingroup io
 * @brief xml callbacks for support of zlib files for libxml
 */

#ifndef	_XMLCALLBACKS_H_
#define	_XMLCALLBACKS_H_

#ifdef NEW_FILE_IO

/**
 * @brief Registers callback functions.
 *
 * Registers callback functions so libxml use el file functions.
 */
void xml_register_el_input_callbacks();

#endif //NEW_FILE_IO

#endif	// _XMLCALLBACKS_H_
