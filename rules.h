/*!
 * \file
 * \ingroup interface_rules
 * \brief handling and display of the rules
 */
#ifndef __RULES_H__
#define __RULES_H__

/*!
 * holds the data for up to 40 rules.
 */
struct rules_struct {
	int no; /*!< the rule no (0-39) */
    /*!
     * inner struct containing the data for a single rule
     */
	struct rule_struct {
		char * short_desc; /*!< a short description for this rule */
		int short_len; /*!< the length of short_desc */
		char * long_desc; /*!< the complete description for this rule */
		int long_len; /*!< the length of long_desc */
		int type; /*!< the type of the rule */
	} rule[40];
};


/*!
 * a conversion of a \see rule_struct to be displayed in-game and used by, for example, moderators
 */
typedef struct {
	int type; /*!< type of the rule */
	int show_long_desc; /*!< flag whether to show the long description of the rule or not */
	int mouseover; /*!< mouseover */
	int highlight;/*!< flag indicating whether the rule should be highlighted or not (Used for moderators) */
    /*!
     * \name coordinates where the rule should appear
     */
    /* \{ */
	int x_start;
	int y_start;
	int x_end;
	int y_end;
    /* \} */
	char ** short_str; /*!< the short description of the rule */
	char ** long_str; /*!< the long description of the rule */
} rule_string;


extern int have_rules; /*!< *** flag indicating whether the rules.xml file is available of not */
extern int last_display; /*! indicates when the rules were last displayed */
extern int countdown; /*!< number of seconds how long the rules should be displayed */
extern int has_accepted; /*!< flag indicating whether the rules are accepted or rejected */

extern int rules_root_win;

/*!
 * \ingroup other
 * \brief reads in the rules
 *
 *      Reads in the rules from the rules.xml file
 *
 * \retval int  0 on success, else != 0 ***
 * \callgraph
 */
int read_rules(void);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_rules
// * \brief releases the given rule_string
// *
// *      Releases the given rule_string and frees the memory used.
// *
// * \param d pointer to the rule_string to release
// */
//void free_rules(rule_string * d);

/*!
 * \ingroup interface_rules
 * \brief draws the interface to show up the rules
 *
 *      Draws the interface to show up the rules at the start of the game
 *
 * \param len_x The width of the rules window
 * \param len_y The height of the rules window
 *
 * \callgraph
 */
void draw_rules_interface (int len_x, int len_y);

/*!
 * \ingroup rules_win
 * \brief toggles the rules window visibile or invisible
 *
 *      Toggles the state of the rules window from visible to invisible and vice versa.
 *
 * \param toggle    defines to which state to switch the rules window
 *
 * \callgraph
 */
void toggle_rules_window(int toggle);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_rules
// * \brief reads the interface rules into a rule_string structure and returns it.
// *
// *      Reads in the interface rules into a rule_string structure and returns it.
// *
// * \param chars_per_line    number of characters in a line.
// * \retval rule_string*     a pointer to a rule_string structure allocated and filled by the function.
// * \callgraph
// */
//rule_string * get_interface_rules(int chars_per_line);

/*!
 * \ingroup interface_rules
 * \brief initializes the rules interface
 *
 *      Initializes the rules interface
 *
 * \param text_size the size of the text being displayed
 * \param countdown number of seconds to show off the rules.
 * \param len_x the width of the rules window
 * \param len_y the height of the rules window
 *
 * \callgraph
 */
void init_rules_interface(float text_size, int countdown, int len_x, int len_y);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_rules
// * \brief checks the position of the mouse within the rules interface
// *
// *      Checks the position of the mouse within the rules interface
// *
// * \param rules An array containing the rule_string structures to display
// * \param lenx
// * \param leny
// * \param mx
// * \param my
// */
//void check_mouse_rules_interface(rule_string * rules, int lenx, int leny, int mx, int my);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_rules
// * \brief draws all the rules to the rules interface
// *
// *      Draws all rules specified in the parameter rules to the rules interface
// *
// * \param rules An array of rule_string structures to be displayed
// * \param rules_no  Number of rules in the rules parameter
// * \param x
// * \param y
// * \param lenx
// * \param leny
// * \param text_size the size of the text to display
// * \retval int
// * \callgraph
// */
//int draw_rules(rule_string * rules, int rules_no, int x, int y, int lenx, int leny, float text_size);

/*!
 * \ingroup other
 * \brief cleans up and frees the memory used by the rules.
 *
 *      Cleans up and frees the memory used by the rules.
 *
 * \callgraph
 */
void cleanup_rules(void);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_rules
// * \brief Resets the rules to the ones specified in the parameter r.
// *
// *      Resets the rules to the ones specified in the parameter.
// *
// * \param r An array of \see rule_string structures which will be used to replace the current rules.
// */
//void reset_rules(rule_string * r);

/*!
 * \ingroup interface_rules
 * \brief highlights the specified rule with the type specified.
 *
 *      Highlights a particular rule, specified by no with a given highlighing type.
 *
 * \param type  the highlighing type to use
 * \param rules
 * \param no    the rule number to highlight.
 *
 * \callgraph
 */
void highlight_rule(int type, Uint8 * rules, int no);

/*!
 * \ingroup interface_rules
 * \brief creates and initializes the rules root window
 *
 *      Creates and initializes the rules root window
 *
 * \param width the width of the window
 * \param height the height of the window
 * \param next the ID of the window to open when  the rules are accepted
 * \param time the timeout in seconds
 *
 * \callgraph
 */
void create_rules_root_window (int width, int height, int next, int time);

#endif
