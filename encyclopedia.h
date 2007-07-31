/*!
 * \file
 * \ingroup encyclopedia
 * \brief handling of the encyclopedia window and files.
 */
#ifndef __ENCYCLOPEDIA_H__
#define __ENCYCLOPEDIA_H__

#include <SDL_types.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ENCYC_OFFSET	40

/*!
 * defines the categories used in the encyclopedia
 */
typedef struct
{
	char *Name; /*!< name of the category */
}_Category;

/*!
 * defines a linked list for the \<Text\> xml element used in encyclopedia xml files.
 */
typedef struct _Texts
{
	int x,y,size; /*!< position and size of the text */
	float r,g,b; /*!< color attributes of the text element */
	char *text,*ref; /*!< pointer to the associated text and references */
	struct _Texts *Next; /*! pointer to the next text in the list */
}_Text;

/*!
 * defines a linked list for the \<Image\> xml element used in encyclopedia xml files.
 */
typedef struct _Images
{
	int id,x,y,xend,yend; /*!< id, start and end position of the image */
	Uint8 mouseover; /*!< flag, determining whether the mouse is currently over the image or not */
	float u,v,uend,vend; /*!< start and end points of texture coordinates */
	struct _Images *Next; /*!< pointer to the next image in the list */
}_Image;

/*!
 * defines the data that is valid in a \<Page\> xml element used in encyclopedia xml files.
 */
typedef struct
{
	char *Name; /*!< name of the page */
	_Text T; /*!< a list of text blocks */
	_Image I; /*!< a list of images */
	int max_y; /*!< the full page length */
}_Page;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int encyclopedia_win; /*!< encyclopedia windows handler */
/*! @} */

extern int encyclopedia_menu_x;
extern int encyclopedia_menu_y;

#define	MAX_ENC_PAGES	1000
extern _Page Page[MAX_ENC_PAGES]; /*!< fixed array of pages for the encyclopedia */
extern int numpage;

/*!
 * \ingroup encyclopedia_window
 * \brief Sets the window handler functions for the encyclopedia window
 *
 *      Sets the window handler functions for the encyclopedia window
 *
 * \return None
 */
void fill_encyclopedia_win ();

/*!
 * \ingroup xml_utils
 * \brief   reads xml from the given \a filename
 *
 *      Reads xml from the given \a filename
 *
 * \param filename  filename of the xml file to read.
 *
 * \callgraph
 */
void ReadXML(const char *filename);

/*!
 * \ingroup xml_utils
 * \brief   frees any memory used by the encyclopedia xml handling.
 *
 *      Frees any memory used by the encyclopedia xml handling.
 *
 * \sa start_rendering
 */
void FreeXML();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
