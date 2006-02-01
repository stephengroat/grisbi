/* ce fichier contient les paramètres de l'affichage de la liste d'opé */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "include.h"


/*START_INCLUDE*/
#include "affichage_liste.h"
#include "gsb_transactions_list.h"
#include "gsb_data_account.h"
#include "gsb_form.h"
#include "gtk_combofix.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "affichage.h"
#include "utils_str.h"
#include "structures.h"
#include "include.h"
#include "gsb_data_form.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *cree_menu_quatres_lignes ( void );
static gboolean gsb_transactions_list_display_change_max_items ( GtkWidget *entry,
							  gpointer null );
static gboolean gsb_transactions_list_display_update_combofix ( void );
static gboolean transactions_list_display_modes_menu_changed  ( GtkWidget * menu_shell,
							 gint origine );
/*END_STATIC*/



gchar *labels_boutons [] = {
    N_("Date"),
    N_("Value date"),
    N_("Payee"),
    N_("Budgetary line"),
    N_("Debit"),
    N_("Credit"),
    N_("Balance"),
    N_("Amount"),
    N_("Method of payment"),
    N_("Reconciliation ref."),
    N_("Financial year"),
    N_("Category"),
    N_("C/R"),
    N_("Voucher"),
    N_("Notes"),
    N_("Bank references"),
    N_("Transaction number"),
    NULL };


/* utilisée pour éviter l'emballement de la connection allocation */

gint ancienne_allocation_liste;
gint affichage_realise;


gint col_depart_drag;
gint ligne_depart_drag;
gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
GtkWidget *boutons_affichage_liste[17];
GtkWidget *clist_affichage_liste;
GtkWidget *bouton_choix_perso_colonnes;
GtkWidget *bouton_caracteristiques_lignes_par_compte;
GtkWidget *bouton_affichage_lignes_une_ligne;
GtkWidget *bouton_affichage_lignes_deux_lignes_1;
GtkWidget *bouton_affichage_lignes_deux_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_1;
GtkWidget *bouton_affichage_lignes_trois_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_3;
/* contient le no de ligne à afficher lorsqu'on n'affiche qu'une ligne */
gint ligne_affichage_une_ligne;
/* contient les no de lignes à afficher lorsqu'on affiche deux lignes */
GSList *lignes_affichage_deux_lignes;
/* contient les no de lignes à afficher lorsqu'on affiche trois lignes */
GSList *lignes_affichage_trois_lignes;


/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GSList *liste_labels_titres_colonnes_liste_ope ;
extern gint max;
extern gchar *tips_col_liste_operations[TRANSACTION_LIST_COL_NB];
extern gchar *titres_colonnes_liste_operations[TRANSACTION_LIST_COL_NB];
/*END_EXTERN*/



/** TODO: document this */
gboolean transactions_list_display_modes_menu_changed  ( GtkWidget * menu_shell,
							 gint origine )
{
    ligne_affichage_une_ligne = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_une_ligne ) -> menu_item ),
								      "no_ligne" ));

    lignes_affichage_deux_lignes = NULL;
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_1 ) -> menu_item ),
									"no_ligne" ));
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_2 ) -> menu_item ),
									"no_ligne" ));

    lignes_affichage_trois_lignes = NULL;
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ) -> menu_item ),
									 "no_ligne" ));
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ) -> menu_item ),
									 "no_ligne" ));
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ) -> menu_item ),
									 "no_ligne" ));


    demande_mise_a_jour_tous_comptes ();
    modification_fichier ( TRUE );

    return ( FALSE );
}


/** FIXME: document this */
GtkWidget *onglet_affichage_operations ( void )
{
    GtkWidget * vbox_pref, *table, *label, *paddingbox;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction list behavior"),
					       "transaction-list.png" );

    /* on permet de choisir quelle ligne seront affichées en fonction des caractéristiques de l'affichage */
    /* pour opé simplifiée */

    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Display modes"));

    table = gtk_table_new ( 3, 6, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("One line mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_une_ligne = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_une_ligne),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_une_ligne,
				1, 2, 0, 1 );

    /* pour 2 lignes */
    label = gtk_label_new ( COLON(_("Two lines mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_deux_lignes_1 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_deux_lignes_1),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_deux_lignes_1,
				1, 2, 1, 2 );

    bouton_affichage_lignes_deux_lignes_2 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_deux_lignes_2),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_deux_lignes_2,
				2, 3, 1, 2 );


    /* pour 3 lignes */
    label = gtk_label_new ( COLON(_("Three lines mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_trois_lignes_1 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_1,
				1, 2, 2, 3 );

    bouton_affichage_lignes_trois_lignes_2 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_2,
				2, 3, 2, 3 );


    bouton_affichage_lignes_trois_lignes_3 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_3,
				3, 4, 2, 3 );


    if ( gsb_data_account_get_accounts_amount () )
    {
	/* on place les lignes à afficher */

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_une_ligne ),
				      ligne_affichage_une_ligne);

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_1 ),
				      GPOINTER_TO_INT (lignes_affichage_deux_lignes->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_2 ),
				      GPOINTER_TO_INT (lignes_affichage_deux_lignes->next->data));

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->next->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->next->next->data));
    }
    else
    {
	gtk_widget_set_sensitive ( table, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_une_ligne, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_deux_lignes_1, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_deux_lignes_2, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_1, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_2, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_3, FALSE );
    }

    /* Connect all menus */
    g_signal_connect ( G_OBJECT ( bouton_affichage_lignes_une_ligne ), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );
    g_signal_connect ( G_OBJECT (bouton_affichage_lignes_deux_lignes_1), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_deux_lignes_2), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_1), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_2), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_3), "changed",
		       G_CALLBACK ( transactions_list_display_modes_menu_changed),
		       NULL );

    /* add the 'loading r into the list at begining' */

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 new_checkbox_with_title (_("Load the R transactions into the list at the begining\n(use if look often the R transactions)"),
						  &etat.fill_r_at_begining,
						  NULL ),
			 FALSE, FALSE, 0 );

    /* Then add the "sort by" buttons */
    paddingbox = new_radiogroup_with_title (vbox_pref,
					    _("Sort transaction list"),
					    _("by value date"),
					    _("by date"),
					    &etat.classement_par_date, NULL);


    /* Account distinction */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Account distinction")));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 new_checkbox_with_title (_("Remember display settings for each account separately"),
						  &etat.retient_affichage_par_compte,
						  NULL ),
			 FALSE, FALSE, 0 );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}



/* ************************************************************************************************************** */
/* renvoie un menu contenant 1ère ligne, 2ème ligne, 3ème ligne, 4ème ligne */
/* ************************************************************************************************************** */

GtkWidget *cree_menu_quatres_lignes ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("first line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("second line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("third line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("fourth line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    gtk_widget_show ( menu );

    return ( menu );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* récupère les noms de colonnes ( en fait les objets de la 1ère ligne ), */
/* et ce qu'on met dans les tips des titres de colonnes */
/* donc, remplit les variables tips_col_liste_operations et titres_colonnes_liste_operations */
/* ************************************************************************************************************** */
void recuperation_noms_colonnes_et_tips ( void )
{
    gint i, j;
    gchar *ligne[TRANSACTION_LIST_COL_NB];

    /* on met les titres et tips à NULL */

    for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
    {
	titres_colonnes_liste_operations[j] = NULL;
	tips_col_liste_operations[j] = NULL;
    }


    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    ligne[j] = g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
					  tab_affichage_ope[i][j] - 1 );

	    /* 	  si on est sur la 1ère ligne, on met les titres ainsi que la 1ere ligne des tips */
	    /* 	    sinon, on rajoute aux tips existant */

	    if ( i )
	    {
		if ( ligne[j] )
		{
		    if ( !titres_colonnes_liste_operations[j] )
			titres_colonnes_liste_operations[j] = ligne[j];

		    if ( tips_col_liste_operations[j] )
			tips_col_liste_operations[j] = g_strconcat ( tips_col_liste_operations[j],
								     ", ",
								     ligne[j],
								     NULL );
		    else
			tips_col_liste_operations[j] = ligne[j];
		}
	    }
	    else
	    {
		if ( ligne[j] )
		{
		    titres_colonnes_liste_operations[j] = ligne[j];
		    tips_col_liste_operations[j] = ligne[j];
		}
	    }
	}
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
/* renvoie le widget contenu dans l'onglet divers du formulaire/liste des paramètres */
/* ************************************************************************************************************** */
GtkWidget *onglet_diverse_form_and_lists ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *paddingbox;
    GtkWidget *radiogroup;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *separator;
    GtkWidget *vbox;


    vbox_pref = new_vbox_with_title_and_icon ( _("Form behavior"),
					       "form.png" );

    /* What to do if RETURN is pressed into transaction form */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("Pressing RETURN in transaction form"),
					    _("selects next field"),
					    _("terminates transaction"),
					    &etat.entree, NULL);

    /* Displayed fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Displayed fields")));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 new_checkbox_with_title (_("'Accept' and 'Cancel' buttons"),
						  &etat.affiche_boutons_valider_annuler,
						  (G_CALLBACK ( update_transaction_form))),
			 FALSE, FALSE, 0 );

    /* How to display financial year */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("By default, use financial year"),
					    _("last selected financial year"),
					    _("according to transaction date"),
					    &etat.affichage_exercice_automatique, 
					    NULL);
    /* Payee search fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Payee completion")));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 new_checkbox_with_title (_("Limit payee completion to current account"),
						  &etat.limit_completion_to_current_account,
						  ((GCallback) NULL )),
			 FALSE, FALSE, 0 );

    /* set the combofix configuration */

    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Completion box configuration")));

    hbox = gtk_hbox_new ( FALSE,
			  5);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 hbox,
			 FALSE, FALSE, 0 );
    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 vbox,
			 FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox),
			 new_checkbox_with_title (_("Mix the categories in list"),
						  &etat.combofix_mixed_sort,
						  (G_CALLBACK ( gsb_transactions_list_display_update_combofix))),
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 new_checkbox_with_title (_("Case sensitive"),
						  &etat.combofix_case_sensitive,
						  (G_CALLBACK ( gsb_transactions_list_display_update_combofix))),
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 new_checkbox_with_title (_("Enter keep the current completion"),
						  &etat.combofix_enter_select_completion,
						  (G_CALLBACK ( gsb_transactions_list_display_update_combofix))),
			 FALSE, FALSE, 0 );

    separator = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX (hbox),
			 separator,
			 FALSE, FALSE, 0 );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 vbox,
			 FALSE, FALSE, 0 );

    /* !! carreful, hbox change here, cannot use later with the function like that */
    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Maximum items showed in list\n(0 no limit)"));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_widget_set_usize ( entry,
			   30, FALSE );
    gtk_entry_set_text ( GTK_ENTRY (entry),
			   utils_str_itoa (etat.combofix_max_item));
    g_signal_connect ( G_OBJECT (entry),
		       "changed",
		       G_CALLBACK (gsb_transactions_list_display_change_max_items),
		       NULL );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 entry,
			 FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox),
			 new_checkbox_with_title (_("Force the item in the payee"),
						  &etat.combofix_force_payee,
						  (G_CALLBACK ( gsb_transactions_list_display_update_combofix))),
			 FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX (vbox),
			 new_checkbox_with_title (_("Force the item in the category/budget"),
						  &etat.combofix_force_category,
						  (G_CALLBACK ( gsb_transactions_list_display_update_combofix))),
			 FALSE, FALSE, 0 );
    
    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/**
 * called when we change a parameter of the combofix configuration
 * update the combofix in the form if they exists
 * as we don't know what was changed, update all the parameter (not a problem
 * because very fast)
 * at this level, the etat.___ variable has already been changed
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_display_update_combofix ( void )
{
    gint account_number;
    GtkWidget *combofix;

    account_number = gsb_form_get_account_number_from_origin (gsb_form_get_origin ());

    combofix = gsb_form_get_element_widget ( TRANSACTION_FORM_PARTY,
					       account_number );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_payee );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
	gtk_combofix_set_enter_function ( GTK_COMBOFIX (combofix),
					  etat.combofix_enter_select_completion );
    }

    combofix = gsb_form_get_element_widget ( TRANSACTION_FORM_CATEGORY,
					       account_number );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_category );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
	gtk_combofix_set_enter_function ( GTK_COMBOFIX (combofix),
					  etat.combofix_enter_select_completion );
	gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (combofix),
				      etat.combofix_mixed_sort );
    }

    combofix = gsb_form_get_element_widget ( TRANSACTION_FORM_BUDGET,
					       account_number );
    if (combofix)
    {
	gtk_combofix_set_force_text ( GTK_COMBOFIX (combofix),
				      etat.combofix_force_category );
	gtk_combofix_set_max_items ( GTK_COMBOFIX (combofix),
				     etat.combofix_max_item );
	gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (combofix),
					  etat.combofix_case_sensitive );
	gtk_combofix_set_enter_function ( GTK_COMBOFIX (combofix),
					  etat.combofix_enter_select_completion );
	gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (combofix),
				      etat.combofix_mixed_sort );
    }
    return FALSE;
}

/**
 * called when change in the max items field
 * change the variable and update the combofix
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_display_change_max_items ( GtkWidget *entry,
							  gpointer null )
{
    etat.combofix_max_item = utils_str_atoi ( gtk_entry_get_text (GTK_ENTRY (entry)));
    gsb_transactions_list_display_update_combofix ();

    return FALSE;
}

							
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
