#ifndef __PREFS_PAGE_DISPLAY_FONTS_H__
#define __PREFS_PAGE_DISPLAY_FONTS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_DISPLAY_FONTS_TYPE    	(prefs_page_display_fonts_get_type ())
#define PREFS_PAGE_DISPLAY_FONTS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_DISPLAY_FONTS_TYPE, PrefsPageDisplayFonts))
#define PREFS_IS_PAGE_DISPLAY_FONTS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_DISPLAY_FONTS_TYPE))

typedef struct _PrefsPageDisplayFonts          PrefsPageDisplayFonts;
typedef struct _PrefsPageDisplayFontsClass     PrefsPageDisplayFontsClass;


struct _PrefsPageDisplayFonts
{
    GtkBox parent;
};

struct _PrefsPageDisplayFontsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               		prefs_page_display_fonts_get_type	(void) G_GNUC_CONST;

PrefsPageDisplayFonts * 	prefs_page_display_fonts_new		(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_DISPLAY_FONTS_H__ */
