/*!
 * \file
 * \ingroup encyclopedia
 * \brief handling of the encyclopedia window and files.
 */
#ifndef __ENCYCLOPEDIA_H__
#define __ENCYCLOPEDIA_H__

#include <libxml/parser.h>
#include <libxml/tree.h>

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
}_Page;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int encyclopedia_win; /*!< encyclopedia windows handler */
/*! @} */

extern int encyclopedia_menu_x;
extern int encyclopedia_menu_y;
extern int encyclopedia_menu_x_len;
extern int encyclopedia_menu_y_len;
extern _Category Category[100]; /*!< array of categories used in the encyclopedia */
extern _Page Page[500]; /*!< fixed array of pages for the encyclopedia */
extern int numpage;

/*!
 * \ingroup display
 * \brief   displays the encyclopedia window
 *
 *      Displays the encyclopedia window
 *
 * \return None
 */
void display_encyclopedia();

/*!
 * \ingroup encyclopedia
 * \brief   mouse over event handler for encyclopedia
 *
 *      Mouse over event handler for encyclopedia
 *
 * \return int
 */
int encyclopedia_mouse_over();

/*!
 * \ingroup xml_utils
 * \brief   reads a category from the xml node \a a_node
 *
 *      Reads a category from the xml node element \a a_node
 *
 * \param a_node    the xml node element that contains the declaration of the category
 * \return None
 */
void ReadCategoryXML(xmlNode * a_node);

/*!
 * \ingroup xml_utils
 * \brief   reads the index.xml file for the encyclopedia
 *
 *      Reads and parses the index.xml file used by the encyclopedia
 *
 * \param a_node    the root xml node element of the index.
 * \return None
 */
void ReadIndexXML(xmlNode * a_node);

/*!
 * \ingroup xml_utils
 * \brief   reads xml from the given \a filename
 *
 *      Reads xml from the given \a filename
 *
 * \param filename  filename of the xml file to read.
 * \return None
 */
void ReadXML(const char *filename);

/*!
 * \ingroup xml_utils
 * \brief   frees any memory used by the encyclopedia xml handling.
 *
 *      Frees any memory used by the encyclopedia xml handling.
 *
 * \return None
 */
void FreeXML();
#endif
