/* Ce fichier s'occupe de la gestion des types d'op�rations */
/* type_operations.c */

/*     Copyright (C)	2000-2003 C�dric Auger (cedric@grisbi.org) */
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
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"


/** Columns for payment methods tree */
enum payment_methods_columns {
  PAYMENT_METHODS_NAME_COLUMN = 0,
  PAYMENT_METHODS_NUMBERING_COLUMN,
  PAYMENT_METHODS_DEFAULT_COLUMN,
  PAYMENT_METHODS_TYPE_COLUMN,
  PAYMENT_METHODS_VISIBLE_COLUMN,
  PAYMENT_METHODS_ACTIVABLE_COLUMN,
  PAYMENT_METHODS_POINTER_COLUMN,
  NUM_PAYMENT_METHODS_COLUMNS,
};

GtkWidget *entree_automatic_numbering; /* FIXME: Move it */


GtkWidget *treeview;
GtkTreeStore *model;

/** Global to handle sensitiveness */
GtkWidget * details_paddingbox;


static void
item_toggled (GtkCellRendererToggle *cell,
	      gchar                 *path_str,
	      gpointer               data)
{
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter, parent, child;
  gboolean toggle_item;
  struct struct_type_ope * type_ope;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 
		      PAYMENT_METHODS_DEFAULT_COLUMN, &toggle_item, 
		      PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
		      -1);

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + type_ope -> no_compte;
  
  if (type_ope -> signe_type == 1) /* D�bit */
    TYPE_DEFAUT_DEBIT = type_ope -> no_type;
  else if  (type_ope -> signe_type == 2) /* Cr�dit */
    TYPE_DEFAUT_CREDIT = type_ope -> no_type;

  /* do something with the value */
  toggle_item ^= 1;

  gtk_tree_model_iter_parent ( model, &parent, &iter );
  
  if ( gtk_tree_model_iter_children (model, &child, &parent) )
    {
      do 
	{
	  gtk_tree_store_set (GTK_TREE_STORE (model), &child, 
			      PAYMENT_METHODS_DEFAULT_COLUMN, FALSE, 
			      -1);
	}
      while ( gtk_tree_model_iter_next (model, &child) );
    } 
  else
    {
      /* Should not happen theorically */
      dialogue ( _("Serious brain damage expected.") );
    }

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
		      PAYMENT_METHODS_DEFAULT_COLUMN, toggle_item, 
		      -1);

  /* clean up */
  gtk_tree_path_free (path);
}



/** FIXME: MOVE IT */
gboolean activate_automatic_numbering ( GtkToggleButton * checkbox, gpointer data )
{
  gtk_widget_set_sensitive ( entree_type_dernier_no,
			     gtk_toggle_button_get_active ( checkbox ));
}



/**
 * Creates the "Payment methods" tab.  It uses a nice GtkTreeView.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_types_operations ( void )
{
  GtkWidget *vbox_pref, *hbox, *scrolled_window, *paddingbox;
  GtkWidget *vbox, *table, *menu, *item, *label, *bouton;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeIter account_iter, debit_iter, credit_iter, child_iter;
  gint i;

  vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					     "reconciliation.png" );

  /* Now we have a model, create view */
  vbox_pref = new_vbox_with_title_and_icon ( _("Payment methods"),
					     "payment-methods.png" );

  /* Known payment methods */
  paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
					  _("Known payment methods"));
  hbox = gtk_hbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       TRUE, TRUE, 0 );

  /* Create tree */
  scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					GTK_SHADOW_IN);
  gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
		       TRUE, TRUE, 0 );

  /* Create tree view */
  model = gtk_tree_store_new (NUM_PAYMENT_METHODS_COLUMNS,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_BOOLEAN,
			      G_TYPE_INT,
			      G_TYPE_BOOLEAN,
			      G_TYPE_BOOLEAN,
			      G_TYPE_POINTER);
  treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (model) );
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
		    "changed", 
		    G_CALLBACK (select_payment_method),
		    model);

  /* Account */
  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ( );
  gtk_tree_view_column_pack_end ( column, cell, TRUE );
  gtk_tree_view_column_set_title ( column, _("Account") );
  gtk_tree_view_column_set_attributes (column, cell,
				       "text", PAYMENT_METHODS_NAME_COLUMN,
				       NULL);
  gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview), column);

  /* Defaults */
  cell = gtk_cell_renderer_toggle_new ();
  g_signal_connect (cell, "toggled", G_CALLBACK (item_toggled), model);
  gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), TRUE );
  g_object_set (cell, "xalign", 0.5, NULL);
  column = gtk_tree_view_column_new ( );
  gtk_tree_view_column_set_alignment ( column, 0.5 );
  gtk_tree_view_column_pack_end ( column, cell, TRUE );
  gtk_tree_view_column_set_title ( column, _("Default") );
  gtk_tree_view_column_set_attributes (column, cell,
				       "active", PAYMENT_METHODS_DEFAULT_COLUMN,
				       "activatable", PAYMENT_METHODS_ACTIVABLE_COLUMN,
				       "visible", PAYMENT_METHODS_VISIBLE_COLUMN,
				       NULL);
  gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview), column);

  /* Numbering */
  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ( );
  gtk_tree_view_column_pack_end ( column, cell, TRUE );
  gtk_tree_view_column_set_title ( column, _("Numbering") );
  gtk_tree_view_column_set_attributes (column, cell,
				       "text", PAYMENT_METHODS_NUMBERING_COLUMN,
				       NULL);
  gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview), column);

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (treeview, "realize",
		    G_CALLBACK (gtk_tree_view_expand_all), NULL);
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      treeview );

  /* Fill tree, iter over with accounts */
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GtkCTreeNode *node_compte;
      GtkCTreeNode *node_debit;
      GtkCTreeNode *node_credit;
      gchar *ligne[2];
      GSList *liste_tmp;

      gtk_tree_store_append (model, &account_iter, NULL);
      gtk_tree_store_set (model, &account_iter,
			  PAYMENT_METHODS_NAME_COLUMN, NOM_DU_COMPTE,
			  PAYMENT_METHODS_NUMBERING_COLUMN, "",
			  PAYMENT_METHODS_TYPE_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			  -1 );

      /* Create the "Debit" node */
      gtk_tree_store_append (model, &debit_iter, &account_iter);
      gtk_tree_store_set (model, &debit_iter,
			  PAYMENT_METHODS_NAME_COLUMN, _("Debit"),
			  PAYMENT_METHODS_NUMBERING_COLUMN, "",
			  PAYMENT_METHODS_TYPE_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			  -1 );

      /* Create the "Debit" node */
      gtk_tree_store_append (model, &credit_iter, &account_iter);
      gtk_tree_store_set (model, &credit_iter,
			  PAYMENT_METHODS_NAME_COLUMN, _("Credit"),
			  PAYMENT_METHODS_NUMBERING_COLUMN, "",
			  PAYMENT_METHODS_TYPE_COLUMN, FALSE,
			  PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			  PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			  PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			  -1 );


      /* Iter over account payment methods */
      liste_tmp = TYPES_OPES;

      while ( liste_tmp )
	{
	  struct struct_type_ope *type_ope;
	  GtkTreeIter * parent_iter;
	  GtkTreeIter method_iter;
	  gboolean isdefault;
	  gchar * number;

	  type_ope = liste_tmp->data;

	  if ( type_ope -> no_type == TYPE_DEFAUT_DEBIT
	       ||
	       type_ope -> no_type == TYPE_DEFAUT_CREDIT )
	    isdefault = 1;
	  else
	    isdefault = 0;

	  if ( !type_ope -> signe_type )
	    parent_iter = &account_iter;
	  else
	    if ( type_ope -> signe_type == 1 )
	      parent_iter = &debit_iter;
	    else
	      parent_iter = &credit_iter;

	  if ( type_ope -> numerotation_auto )
	    {
	      number = itoa ( type_ope -> no_en_cours );
	    }
	  else
	    {
	      number = "";
	    }

	  /* Insert a child node */
	  gtk_tree_store_append (model, &method_iter, parent_iter);
	  gtk_tree_store_set (model, &method_iter,
			      PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type,
			      PAYMENT_METHODS_NUMBERING_COLUMN, number,
			      PAYMENT_METHODS_TYPE_COLUMN, type_ope -> signe_type,
			      PAYMENT_METHODS_DEFAULT_COLUMN, isdefault,
			      PAYMENT_METHODS_ACTIVABLE_COLUMN, type_ope->signe_type!=0,
			      PAYMENT_METHODS_VISIBLE_COLUMN, type_ope->signe_type!=0,
			      PAYMENT_METHODS_POINTER_COLUMN, type_ope,
			      -1 );

	  liste_tmp = liste_tmp -> next;
	}

      p_tab_nom_de_compte_variable++;
    }


  /* Create "Add" & "Remove" buttons */
  vbox = gtk_vbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
		       FALSE, FALSE, 0 );

  /* "Add payment method" button */
  bouton_ajouter_type = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_ajouter_type, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_type ),
		       "clicked",
		       (GtkSignalFunc ) ajouter_type_operation,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_type,
		       TRUE, FALSE, 5 );

  /* "Remove payment method" button */
  bouton_retirer_type = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_button_set_relief ( GTK_BUTTON ( bouton_retirer_type ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_retirer_type, FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_retirer_type ),
		       "clicked",
		       (GtkSignalFunc ) supprimer_type_operation,
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_retirer_type,
		       TRUE, FALSE, 5 );

  /* Payment method details */
  details_paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
						  _("Payment method details"));
  gtk_widget_set_sensitive ( details_paddingbox, FALSE );

  /* Payment method name */
  table = gtk_table_new ( 2, 2, FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
  gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
  gtk_box_pack_start ( GTK_BOX ( details_paddingbox ), table,
		       TRUE, TRUE, 6 );

  label = gtk_label_new ( COLON(_("Name")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_type_nom = new_text_entry ( NULL, modification_entree_nom_type );
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_type_nom, 1, 3, 0, 1,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );

  /* Automatic numbering */
  label = gtk_label_new ( COLON(_("Automatic numbering")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );
  entree_type_dernier_no = new_spin_button (NULL,
					    0, G_MAXDOUBLE, 
					    1, 5, 5, 
					    2, 0, (GCallback) modification_entree_type_dernier_no );
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_type_dernier_no, 1, 2, 1, 2,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  entree_automatic_numbering = 
    new_checkbox_with_title (_("Activate"),
			     NULL, activate_automatic_numbering);
  gtk_table_attach ( GTK_TABLE ( table ),
		     entree_automatic_numbering, 2, 3, 1, 2,
		     GTK_SHRINK, 0,
		     0, 0 );

  /* Payment method type */
  label = gtk_label_new ( COLON(_("Type")) );
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 2, 3,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create menu */
  bouton_signe_type = gtk_option_menu_new ();
  menu = gtk_menu_new();
  /* Neutral type */
  item = gtk_menu_item_new_with_label ( _("Neutral") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      NULL );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Debit type */
  item = gtk_menu_item_new_with_label ( _("Debit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Credit type */
  item = gtk_menu_item_new_with_label ( _("Credit") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_type_signe ),
			      GINT_TO_POINTER (2) );
  gtk_menu_append ( GTK_MENU ( menu ), item );
  /* Set menu */
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_signe_type ), menu );
  gtk_table_attach ( GTK_TABLE ( table ),
		     bouton_signe_type, 1, 3, 2, 3,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );

  return ( vbox_pref );
}


/**
 * Callback used when a payment method is selected in payment methods
 * list.
 */
gboolean
select_payment_method (GtkTreeSelection *selection,
		       GtkTreeModel *model)
{
  GtkTreeIter iter;
  GValue value_name = {0, };
  GValue value_visible = {0, };
  GValue value_numbering = {0, };
  GValue value_type = {0, };
  gboolean good;

  good = gtk_tree_selection_get_selected (selection, NULL, &iter);
  gtk_tree_model_get_value (model, &iter, 
			    PAYMENT_METHODS_VISIBLE_COLUMN, &value_visible);

  if (! good ||
      ! g_value_get_boolean(&value_visible))
    {
      entry_set_value ( entree_type_nom, NULL );
      spin_button_set_value ( entree_type_dernier_no, NULL );
      gtk_entry_set_text ( GTK_ENTRY ( entree_type_dernier_no ), "" );
      checkbox_set_value ( entree_automatic_numbering, NULL, TRUE );
      activate_automatic_numbering ( entree_automatic_numbering, NULL );
      /* We set menu to "Neutral" as a default*/
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_signe_type ), 0);	
      gtk_widget_set_sensitive ( details_paddingbox, FALSE );
    }
  else
    {
      struct struct_type_ope * type_ope;
      gtk_tree_model_get (model, &iter, 
			  PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			  -1);
      gtk_widget_set_sensitive ( details_paddingbox, TRUE );
      entry_set_value ( entree_type_nom, &(type_ope -> nom_type) );
      spin_button_set_value ( entree_type_dernier_no, 
			      (gboolean) &(type_ope -> no_en_cours) );
      checkbox_set_value ( entree_automatic_numbering, 
			   &(type_ope -> numerotation_auto), TRUE );
      activate_automatic_numbering ( entree_automatic_numbering, NULL );
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_signe_type ),
				    type_ope -> signe_type );
    }
}



/**
 * Callback used when a payment method is deselected in payment
 * methods list. 
 */
gboolean
deselect_payment_method (GtkTreeSelection *selection,
			 GtkTreeModel *model)
{
}



/** TODO: remove?  */
void selection_ligne_arbre_types ( GtkWidget *arbre,
				   GtkCTreeNode *node,
				   gint col,
				   GtkWidget *vbox )
{
}
void deselection_ligne_arbre_types ( GtkWidget *arbre,
				     GtkCTreeNode *node,
				     gint col,
				     GtkWidget *vbox )
{
}



/**
 * TODO: document
 *
 */
void modification_entree_nom_type ( void )
{
  struct struct_type_ope *type_ope;
  GtkWidget * menu;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GValue value_name = {0, };
  GValue value_visible = {0, };
  gboolean good, visible;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  good = gtk_tree_selection_get_selected (selection, NULL, &iter);

  gtk_tree_model_get ( model, &iter, 
		       PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
		       PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
		       -1 );

  if (good && visible)
    {
      gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			  PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type, 
			  -1);

      if ( (menu = creation_menu_types ( 1, compte_courant , 0 )))
	{
	  gint pos_type;

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				     menu );

	  pos_type = cherche_no_menu_type ( TYPE_DEFAUT_DEBIT );

	  if ( pos_type != -1 )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					  pos_type );
	  else
	    {
	      struct struct_type_ope *type;

	      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					    0 );
	      TYPE_DEFAUT_DEBIT = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
									"no_type" ));

	      /* on affiche l'entr�e des ch�ques si n�cessaire */

	      type = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
					 "adr_type" );

	      if ( type -> affiche_entree )
		gtk_widget_show ( widget_formulaire_operations[10] );
	    }

	  gtk_widget_show ( widget_formulaire_operations[9] );
	}
      else
	{
	  gtk_widget_hide ( widget_formulaire_operations[9] );
	  gtk_widget_hide ( widget_formulaire_operations[10] );
	}

    }
}




/**
 * TODO: document this
 *
 */
void modification_type_affichage_entree ( void )
{
}



/**
 * TODO: document this
 *
 */
void modification_type_numerotation_auto (void)
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_numerotation_automatique )))
    {
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 TRUE );
      type_ope -> numerotation_auto = 1;
    }
  else
    {
      type_ope -> numerotation_auto = 0;
      gtk_widget_set_sensitive ( entree_type_dernier_no,
				 FALSE );
    }
}



/**
 * TODO: document this
 */
void modification_entree_type_dernier_no ( void )
{
  struct struct_type_ope *type_ope;
  GtkWidget * menu;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GValue value_name = {0, };
  GValue value_visible = {0, };
  gboolean good, visible;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  good = gtk_tree_selection_get_selected (selection, NULL, &iter);

  gtk_tree_model_get ( model, &iter, 
		       PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
		       PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
		       -1 );

  if (good && visible)
    {
      gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			  PAYMENT_METHODS_NUMBERING_COLUMN, itoa(type_ope -> no_en_cours), 
			  -1);
    }
}


/* ************************************************************************************************************** */
void modification_type_signe ( gint *no_menu )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;
  GtkCTreeNode *node_parent;


/*   node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ), */
/* 			       "adr_node" ); */
/*   type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ), */
/* 					   node ); */

/*   /\*   s'il n'y a pas eu de changement, on vire *\/ */

/*   if ( GPOINTER_TO_INT ( no_menu ) == type_ope -> signe_type ) */
/*     return; */

/*   /\*   si on est sur neutre, on insensitive le par d�faut *\/ */

/*   if ( GPOINTER_TO_INT ( no_menu )) */
/*     gtk_widget_set_sensitive ( bouton_type_choix_defaut, */
/* 			       TRUE ); */
/*   else */
/*     { */
/*       gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut ), */
/* 				     FALSE ); */
/*       gtk_widget_set_sensitive ( bouton_type_choix_defaut, */
/* 				 FALSE ); */
/*     } */

/*   /\*   si le type chang� �tait par d�faut, on vire le par-d�faut qu'on met � 0 *\/ */

/*   if ( type_defaut_debit[type_ope->no_compte] == type_ope->no_type ) */
/*     { */
/*       type_defaut_debit[type_ope->no_compte] = 0; */
/*       gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				node, */
/* 				1, */
/* 				"" ); */
/*     } */
/*   else */
/*     if ( type_defaut_credit[type_ope->no_compte] == type_ope->no_type ) */
/*       { */
/* 	type_defaut_credit[type_ope->no_compte] = 0; */
/* 	gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				  node, */
/* 				  1, */
/* 				  "" ); */
/*       } */


/*   switch ( GPOINTER_TO_INT ( no_menu )) */
/*     { */

/*       /\*   cas le plus simple, on passe � neutre, dans ce cas le parent du node devient le compte *\/ */

/*     case 0: */
/*       node_parent = GTK_CTREE_ROW ( GTK_CTREE_ROW ( node ) -> parent ) -> parent; */

/*       gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ), */
/* 		       node, */
/* 		       node_parent, */
/* 		       NULL ); */
/*       break; */

/*       /\* si c'est un d�bit *\/ */

/*     case 1: */

/*       node_parent = GTK_CTREE_ROW ( node ) -> parent; */

/*       if ( GTK_CTREE_ROW ( node_parent ) -> level == 2 ) */
/* 	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent; */

/*       node_parent = GTK_CTREE_ROW ( node_parent ) -> children; */
/*       gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ), */
/* 		       node, */
/* 		       node_parent, */
/* 		       NULL ); */
/*       break; */

/*       /\* si c'est un credit *\/ */

/*     case 2: */
/*       node_parent = GTK_CTREE_ROW ( node ) -> parent; */

/*       if ( GTK_CTREE_ROW ( node_parent ) -> level == 2 ) */
/* 	node_parent = GTK_CTREE_ROW ( node_parent ) -> parent; */

/*       node_parent = GTK_CTREE_ROW ( GTK_CTREE_ROW ( node_parent ) -> children ) -> sibling; */
/*       gtk_ctree_move ( GTK_CTREE ( arbre_types_operations ), */
/* 		       node, */
/* 		       node_parent, */
/* 		       NULL ); */
/*       break; */

/*     } */

/*   type_ope -> signe_type = GPOINTER_TO_INT ( no_menu ); */


/*   /\*   pour les tris, il suffit de retirer les n�gatifs correspondant au type *\/ */
/*   /\* puis r�afficher la liste, sauf si on est pass� sur *\/ */
/*   /\* neutre et que les neutres sont inclus dans les d�bits et cr�dits *\/ */
/*   /\*     dans ce cas, on ajoute � la liste l'oppos� *\/ */

/*   if ( neutres_inclus_tmp[type_ope->no_compte] ) */
/*     { */
/*       if ( no_menu ) */
/* 	/\* 	  on retire le signe oppos� du type, juste au cas o� on est pass� d'un neutre � l'actuel *\/ */
/* 	liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte], */
/* 							      GINT_TO_POINTER ( -type_ope->no_type )); */
/*       else */
/* 	liste_tri_tmp[type_ope->no_compte] = g_slist_append ( liste_tri_tmp[type_ope->no_compte], */
/* 							      GINT_TO_POINTER ( -type_ope->no_type )); */
/*     } */

/*   remplit_liste_tri_par_type ( type_ope->no_compte ); */

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void modification_type_par_defaut ( void )
{
/*   struct struct_type_ope *type_ope; */
/*   GtkCTreeNode *node; */

/*   node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ), */
/* 			       "adr_node" ); */
/*   type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ), */
/* 					   node ); */


/*   if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_choix_defaut ))) */
/*     { */
/*       /\* on vient de choisir ce type par d�faut *\/ */

/*       GSList *liste_tmp; */
/*       struct struct_type_ope *type_ope_defaut; */
/*       GtkCTreeNode *node_defaut; */

      
/*       liste_tmp = liste_tmp_types[type_ope->no_compte]; */


/*       if ( type_ope->signe_type == 2 ) */
/* 	{ */
/* 	  /\* s'il y avait d�j� un d�faut, on vire la croix � c�t� de celui ci *\/ */

/* 	  if ( type_defaut_credit[type_ope->no_compte] ) */
/* 	    { */
/* 	      type_ope_defaut = g_slist_find_custom ( liste_tmp, */
/* 						      GINT_TO_POINTER (type_defaut_credit[type_ope->no_compte]), */
/* 						      (GCompareFunc) recherche_type_ope_par_no ) -> data; */

/* 	      node_defaut = gtk_ctree_find_by_row_data ( GTK_CTREE ( arbre_types_operations ), */
/* 							 GTK_CTREE_ROW ( node ) -> parent, */
/* 							 type_ope_defaut ); */
/* 	      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 					node_defaut, */
/* 					1, */
/* 					"" ); */


/* 	    } */

/* 	  type_defaut_credit[type_ope->no_compte] = type_ope->no_type; */
/* 	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				    node, */
/* 				    1, */
/* 				    "x" ); */
/* 	} */
/*       else */
/* 	{ */
/* 	  /\* s'il y avait d�j� un d�faut, on vire la croix � c�t� de celui ci *\/ */

/* 	  if ( type_defaut_debit[type_ope->no_compte] ) */
/* 	    { */
/* 	      type_ope_defaut = g_slist_find_custom ( liste_tmp, */
/* 						      GINT_TO_POINTER (type_defaut_debit[type_ope->no_compte]), */
/* 						      (GCompareFunc) recherche_type_ope_par_no ) -> data; */

/* 	      node_defaut = gtk_ctree_find_by_row_data ( GTK_CTREE ( arbre_types_operations ), */
/* 							 GTK_CTREE_ROW ( node ) -> parent, */
/* 							 type_ope_defaut ); */
/* 	      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 					node_defaut, */
/* 					1, */
/* 					"" ); */


/* 	    } */

/* 	  type_defaut_debit[type_ope->no_compte] = type_ope->no_type; */
/* 	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				    node, */
/* 				      1, */
/* 				    "x" ); */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       /\* on retire ce type du d�faut *\/ */

/*       if ( type_ope->signe_type == 2 ) */
/* 	{ */
/* 	  type_defaut_credit[type_ope->no_compte] = 0; */
/* 	  gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				    node, */
/* 				    1, */
/* 				    "" ); */
/* 	} */
/*       else */
/* 	if ( type_ope->signe_type == 1 ) */
/* 	  { */
/* 	    type_defaut_debit[type_ope->no_compte] = 0; */
/* 	    gtk_ctree_node_set_text ( GTK_CTREE ( arbre_types_operations ), */
/* 				      node, */
/* 				      1, */
/* 				      "" ); */
/* 	  } */
/*     } */
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type )
{

  return ( !(type_ope->no_type == GPOINTER_TO_INT(no_type)) );

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void ajouter_type_operation ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node_banque;
  GtkCTreeNode *nouveau_node;
  gint no_compte;
  gchar *ligne[2];


  node_banque = GTK_CLIST ( arbre_types_operations ) -> selection -> data;

  /* on remonte jusqu'au node de la banque */

  while ( GTK_CTREE_ROW ( node_banque ) -> level != 1 )
    node_banque = GTK_CTREE_ROW ( node_banque ) -> parent;

  no_compte = GPOINTER_TO_INT (gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
							     node_banque ));

  type_ope = malloc ( sizeof ( struct struct_type_ope ));

  if ( liste_tmp_types[no_compte] ) /* FIXME */
    type_ope -> no_type = ((struct struct_type_ope *)(g_slist_last ( liste_tmp_types[no_compte] )->data))->no_type + 1;
  else
    type_ope -> no_type = 1;

  type_ope -> nom_type = g_strdup ( _("New") );
  type_ope -> signe_type = 0;
  type_ope -> affiche_entree = 0;
  type_ope -> numerotation_auto = 0;
  type_ope -> no_en_cours = 0;
  type_ope -> no_compte = no_compte;

  liste_tmp_types[no_compte] = g_slist_append ( liste_tmp_types[no_compte], /* FIXME */
						type_ope );

  ligne[0] = type_ope -> nom_type;
  ligne[1] = NULL;

  nouveau_node = gtk_ctree_insert_node ( GTK_CTREE ( arbre_types_operations ),
					 node_banque,
					 NULL,
					 ligne,
					 0,
					 NULL, NULL,
					 NULL, NULL,
					 FALSE, FALSE );

  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_types_operations ),
				nouveau_node,
				type_ope );


  /* on ajoute ce type � la liste des tris */

  liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],	/* FIXME */
					      GINT_TO_POINTER ( type_ope -> no_type ));

  /*   si les neutres doivent �tre int�gr�s dans les d�bits cr�dits, on ajoute son oppos� */

  if ( neutres_inclus_tmp[no_compte] )
    liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
						GINT_TO_POINTER ( -type_ope -> no_type ));

  remplit_liste_tri_par_type ( no_compte );

  /* on ouvre le node de la banque au cas o� celui ci ne le serait pas */

  gtk_ctree_expand ( GTK_CTREE ( arbre_types_operations ),
		     node_banque );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void supprimer_type_operation ( void )
{
  struct struct_type_ope *type_ope;
  GtkCTreeNode *node;
  GSList *pointeur_tmp;
  GSList *ope_a_changer;
  gint save_pref;


  /* sera mis � 1 s'il faut sauver les pr�f�rences des types */

  save_pref = 0;

  /* r�cup�re le type concern� */

  node = gtk_object_get_data ( GTK_OBJECT ( entree_type_nom ),
			       "adr_node" );
  type_ope = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_types_operations ),
					   node );


  /*   on fait le tour du compte concern� pour voir si des op�s avaient ce type, */
  /*     si oui, on les met dans une liste */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + type_ope -> no_compte;
  pointeur_tmp = LISTE_OPERATIONS; /* FIXME!!! */
  ope_a_changer = NULL;

  while ( pointeur_tmp )
    {
      struct structure_operation *operation;

      operation = pointeur_tmp -> data;

      if ( operation -> type_ope == type_ope -> no_type )
	ope_a_changer = g_slist_append ( ope_a_changer,
					 operation );
      pointeur_tmp = pointeur_tmp -> next;
    }

  /*   � ce niveau, soit ope_a_changer est null, et on supprime le type dans la liste_tmp */
  /* donc possibiliter d'annuler */
  /* soit c'est pas nul, et on pr�sente un dialogue qui permet de rappatrier les op�s */
  /*     sur cet autre type ; par contre l� on ne peut annuler la suppression */

  if ( ope_a_changer )
    {
      /* des op�s sont � changer */

      GtkWidget *dialog;
      GtkWidget *label;
      gint resultat;
      GtkWidget * option_menu;
      GtkWidget *separateur;
      GtkWidget *hbox;
      GtkWidget *menu;
      gint nouveau_type;

      dialog = gnome_dialog_new ( _("Delete a method of payment"),
				  GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL );

      label = gtk_label_new ( _("Some transactions are still registered with this method of payment,\nthough this deletion is irreversible. The changes about the method\nof payment will be registered."));
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      separateur = gtk_hseparator_new ();
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   separateur,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( separateur );

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( hbox );



      label = gtk_label_new ( POSTSPACIFY(_("Move the transactions to")));
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      /* on va mettre ici le bouton des type de la liste tmp car on peut d�j� avoir */
      /* ajout� ou retir� des types */

      option_menu = gtk_option_menu_new ();
      menu = gtk_menu_new ();


      pointeur_tmp = liste_tmp_types[type_ope->no_compte];

      while ( pointeur_tmp )
	{
	  struct struct_type_ope *type;
	  GtkWidget *menu_item;

	  type = pointeur_tmp -> data;

	  if ( type -> no_type != type_ope -> no_type
	       &&
	       ( type -> signe_type == type_ope -> signe_type
		 ||
		 !type -> signe_type ))
	    {
	      menu_item = gtk_menu_item_new_with_label ( type -> nom_type );
	      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				    "no_type",
				    GINT_TO_POINTER ( type -> no_type ));
	      gtk_menu_append ( GTK_MENU ( menu ),
				menu_item );
	      gtk_widget_show ( menu_item );
	    }
	  pointeur_tmp = pointeur_tmp -> next;
	}

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
				 menu );
      gtk_widget_show ( menu );

      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   option_menu,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( option_menu );

      /*       s'il n'y a aucun autre types, on grise le choix de transfert */

      if ( !GTK_MENU_SHELL ( menu ) -> children )
	gtk_widget_set_sensitive ( hbox,
				   FALSE );

      resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

      if ( resultat )
	{
	  if ( GNOME_IS_DIALOG ( dialog ))
	    gnome_dialog_close ( GNOME_DIALOG ( dialog ));
	  return;
	}

      /* r�cup�ration du nouveau type d'op� */

      if ( GTK_MENU_SHELL ( menu ) -> children )
	nouveau_type = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
							       "no_type" ));
      else
	nouveau_type = 0;

      /* on change le type des op�s concern�es */

      pointeur_tmp = ope_a_changer;

      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;

	  operation = pointeur_tmp -> data;

	  operation -> type_ope = nouveau_type;
	  pointeur_tmp = pointeur_tmp -> next;
	}

      /* on sauvegarde les pr�f des types */

      save_pref = 1;

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
    }


  /* on vire le type de l'arbre */

  gtk_ctree_remove_node ( GTK_CTREE ( arbre_types_operations ),
			  node );

  /* on retire le no de type dans la liste de tri et on r�affiche la liste */

  liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte],
							GINT_TO_POINTER ( type_ope -> no_type ));

  if ( !type_ope -> signe_type && neutres_inclus_tmp[type_ope->no_compte] )
    liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte],
							  GINT_TO_POINTER ( -type_ope -> no_type ));

  remplit_liste_tri_par_type ( type_ope->no_compte );

  /*   si le type �tait par d�faut, on met le d�faut � 0 */

  if ( type_ope -> signe_type == 1
       &&
       type_defaut_debit[type_ope->no_compte] == type_ope -> no_type )
    type_defaut_debit[type_ope->no_compte] = 0;

  if ( type_ope -> signe_type == 2
       &&
       type_defaut_credit[type_ope->no_compte] == type_ope -> no_type )
    type_defaut_credit[type_ope->no_compte] = 0;

  liste_tmp_types[type_ope->no_compte] = g_slist_remove ( liste_tmp_types[type_ope->no_compte],
							  type_ope );
    

  if ( save_pref )
    changement_preferences ( fenetre_preferences,
			     8,
			     NULL );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* Appel�e quand on change le tri par date ou par type */
/* rend sensitif ou non la liste des types du tri */
/* ************************************************************************************************************** */

void modif_tri_date_ou_type ( void )
{
  gint no_compte;

/*   no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ), */
/* 						      "no_compte" )); */

/*   if ( (tri_tmp[no_compte] = !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_tri_date )))) */
/*     { */
/*       gtk_widget_set_sensitive ( bouton_type_neutre_inclut, */
/* 				 TRUE ); */
/*       gtk_widget_set_sensitive ( type_liste_tri, */
/* 				 TRUE ); */
/*     } */
/*   else */
/*     { */
/*       gtk_widget_set_sensitive ( bouton_type_neutre_inclut, */
/* 				 FALSE ); */
/*       gtk_widget_set_sensitive ( type_liste_tri, */
/* 				 FALSE ); */
/*     } */
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void inclut_exclut_les_neutres ( void )
{
  gint no_compte;
  GSList *liste_tmp;


  no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ),
						      "no_compte" ));

  if ( (neutres_inclus_tmp[no_compte] = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_type_neutre_inclut ))))
    {
      /* on inclut les neutres dans les d�bits et cr�dits */
      /*   on fait le tour de tous les types du compte, et pour chaque type neutre, */
      /* on rajoute son num�ro en n�gatif n�gatif � la liste */

      liste_tmp = liste_tri_tmp[no_compte];

      while ( liste_tmp )
	{
	  struct struct_type_ope *type_ope;

	  if ( GPOINTER_TO_INT ( liste_tmp->data ) > 0 )
	    {
	      type_ope = g_slist_find_custom ( liste_tmp_types[no_compte],
					       liste_tmp->data,
					       (GCompareFunc) recherche_type_ope_par_no ) -> data;

	      if ( !type_ope->signe_type )
		liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
							    GINT_TO_POINTER ( - GPOINTER_TO_INT ( liste_tmp->data )));

	    }
	  liste_tmp = liste_tmp -> next;
	}
    }
  else
    {
      /* on efface tous les nombres n�gatifs de la liste */

      liste_tmp = liste_tri_tmp[no_compte];

      while ( liste_tmp )
	{
	  if ( GPOINTER_TO_INT ( liste_tmp->data ) < 0 )
	    {
	      liste_tri_tmp[no_compte] = g_slist_remove ( liste_tri_tmp[no_compte],
							  liste_tmp -> data );
	      liste_tmp = liste_tri_tmp[no_compte];
	    }
	  else
	    liste_tmp = liste_tmp -> next;
	}
    }
   
  remplit_liste_tri_par_type ( no_compte );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void remplit_liste_tri_par_type ( gint no_compte )
{
  GSList *liste_tmp;

  gtk_clist_clear ( GTK_CLIST ( type_liste_tri ));
  deselection_type_liste_tri ();

  liste_tmp = liste_tri_tmp[no_compte];
  
  while ( liste_tmp )
    {
      GSList *liste_tmp2;
      struct struct_type_ope *type_ope;
      gchar *texte[1];
      gint no_ligne;

      liste_tmp2 = g_slist_find_custom ( liste_tmp_types[no_compte],
					 GINT_TO_POINTER ( abs ( GPOINTER_TO_INT ( liste_tmp -> data ))),
					 (GCompareFunc) recherche_type_ope_par_no );

      if ( liste_tmp2 )
	{
	  type_ope = liste_tmp2 -> data;

	  texte[0] = type_ope -> nom_type;

	  if ( type_ope -> signe_type == 1 )
	    texte[0] = g_strconcat ( texte[0],
				     " ( - )",
				     NULL );
	  else
	    if ( type_ope -> signe_type == 2 )
	      texte[0] = g_strconcat ( texte[0],
				       " ( + )",
				       NULL );
	    else
	      if ( neutres_inclus_tmp[no_compte] )
		{
		  /* si c'est un type neutre et qu'ils sont inclus, celui-ci est soit positif soit n�gatif */
	    
		  if ( GPOINTER_TO_INT ( liste_tmp -> data ) < 0 )
		    texte[0] = g_strconcat ( texte[0],
					     " ( - )",
					     NULL );
		  else
		    texte[0] = g_strconcat ( texte[0],
					     " ( + )",
					     NULL );

		}

	  no_ligne = gtk_clist_append ( GTK_CLIST ( type_liste_tri ),
					texte );

	  gtk_clist_set_row_data ( GTK_CLIST ( type_liste_tri ),
				   no_ligne,
				   liste_tmp -> data );
	}
      liste_tmp = liste_tmp -> next;
    }
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
void selection_type_liste_tri ( void )
{

  /* on rend sensible les boutons de d�placement */

  gtk_widget_set_sensitive ( vbox_fleches_tri,
			     TRUE );

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deselection_type_liste_tri ( void )
{

  /* on rend non sensible les boutons de d�placement */

  gtk_widget_set_sensitive ( vbox_fleches_tri,
			     FALSE );


}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deplacement_type_tri_haut ( void )
{

  if ( GTK_CLIST ( type_liste_tri ) -> selection -> data )
    {
      gtk_clist_swap_rows ( GTK_CLIST ( type_liste_tri ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) - 1 );
      save_ordre_liste_type_tri();
    }

}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deplacement_type_tri_bas ( void )
{
  if ( GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) < GTK_CLIST ( type_liste_tri ) -> rows )
    {
      gtk_clist_swap_rows ( GTK_CLIST ( type_liste_tri ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ),
			    GPOINTER_TO_INT ( GTK_CLIST ( type_liste_tri ) -> selection -> data ) + 1 );
      save_ordre_liste_type_tri();
    }
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* cette fonction est appel�e chaque fois qu'on modifie l'ordre de la liste des tris */
/* et elle save cet ordre dans la liste temporaire */
/* ************************************************************************************************************** */

void save_ordre_liste_type_tri ( void )
{
  gint no_compte;
  gint i;

  no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_tri_date ),
						      "no_compte" ));
  g_slist_free ( liste_tri_tmp[no_compte] );
  liste_tri_tmp[no_compte] = NULL;

  for ( i=0 ; i < GTK_CLIST ( type_liste_tri ) -> rows ; i++ )
    liste_tri_tmp[no_compte] = g_slist_append ( liste_tri_tmp[no_compte],
						gtk_clist_get_row_data ( GTK_CLIST ( type_liste_tri ),
									 i ));
}
/* ************************************************************************************************************** */





/* ************************************************************************************************************** */
/* Fonction creation_menu_types */
/* argument : 1 : renvoie un menu de d�bits */
/* 2 : renvoie un menu de cr�dits */
/* ou renvoie le tout si c'est d�sir� dans les param�tres */
/* l'origine est 0 si vient des op�rations, 1 si vient des �ch�ances, 2 pour ne pas mettre de signal quand il y a un chgt */
/* ************************************************************************************************************** */

GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine )
{
  GtkWidget *menu;
  GSList *liste_tmp;
  gpointer **save_ptab;

  save_ptab = p_tab_nom_de_compte_variable;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  /*   s'il n'y a pas de menu, on se barre */

  if ( !(liste_tmp = TYPES_OPES ))
    {
      p_tab_nom_de_compte_variable = save_ptab;
      return ( NULL );
    }

  menu = NULL;

  while ( liste_tmp )
    {
      struct struct_type_ope *type;

      type = liste_tmp -> data;

      if ( type -> signe_type == demande
	   ||
	   !type -> signe_type
	   ||
	   etat.affiche_tous_les_types )
	{
	  GtkWidget *item;

	  /* avant de mettre l'item, on cr�e le menu si n�cessaire */
	  /* le faire ici permet de retourner null si il n'y a rien */
	  /*   dans le menu (sinon, si rien dans les cr�dits, mais qque */
	  /* chose dans les d�bits, renvoie un menu vide qui sera affich� */

	  if ( !menu )
	    {
	      menu = gtk_menu_new();
	      
	      /* on associe au menu la valeur 1 pour menu de d�bit et 2 pour menu de cr�dit */

	      gtk_object_set_data ( GTK_OBJECT ( menu ),
				    "signe_menu",
				    GINT_TO_POINTER ( demande ) );
	      gtk_object_set_data ( GTK_OBJECT ( menu ),
				    "no_compte",
				    GINT_TO_POINTER ( compte ) );
	      gtk_widget_show ( menu );
	    }


	  item = gtk_menu_item_new_with_label ( type -> nom_type );

	  if ( !origine )
	    switch ( origine )
	      {
	      case 0:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_formulaire ),
					    (GtkObject *) type );
		break;
	      case 1:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_echeancier ),
					    (GtkObject *) type );
		break;
	      }

	  gtk_object_set_data ( GTK_OBJECT ( item ),
				"adr_type",
				type );
	  gtk_object_set_data ( GTK_OBJECT ( item ),
				"no_type",
				GINT_TO_POINTER ( type -> no_type ));
	  gtk_menu_append ( GTK_MENU ( menu ),
			    item );
	  gtk_widget_show ( item );
	}
      liste_tmp = liste_tmp -> next;
    }

  p_tab_nom_de_compte_variable = save_ptab;
  return ( menu );
}
/* ************************************************************************************************************** */


  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type */
/*   argument : le num�ro du type demand� */
/* renvoie la place demand�e dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entr�e du chq si n�cessaire */
/* retourne -1 si pas trouv� */
/* ************************************************************************************************************** */

gint cherche_no_menu_type ( gint demande )
{
  GList *liste_tmp;
  gint retour;
  gint i;

  if ( !demande )
    return ( FALSE );

  liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu ) -> children;
  retour = -1;
  i=0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	  struct struct_type_ope *type;

	  retour = i;

	  /* affiche l'entr�e chq du formulaire si n�cessaire */

	  type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				       "adr_type");

	  if ( type -> affiche_entree )
	    gtk_widget_show ( widget_formulaire_operations[10] );
	  else
	    gtk_widget_hide ( widget_formulaire_operations[10] );
	}
      i++;
      liste_tmp = liste_tmp -> next;
    }

  return ( retour );
}
/* ************************************************************************************************************** */



  

  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type_associe */
/*   argument : le num�ro du type demand� */
/* renvoie la place demand�e dans l'option menu du formulaire du type associ� */
/* retourne -1 si pas trouv� */
/* origine = 0 pour les op�rations */
/* origine = 1 pour les ventilations */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_associe ( gint demande,
				    gint origine )
{
  GList *liste_tmp;
  gint retour;
  gint i;

  if ( !demande )
    return ( FALSE );

  if ( origine )
    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_ventilation[5] ) -> menu ) -> children;
  else
    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ) -> menu ) -> children;

  retour = -1;
  i=0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	retour = i;

      i++;
      liste_tmp = liste_tmp -> next;
    }

  return ( retour );
}
/* ************************************************************************************************************** */




  
/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type_echeancier */
/*   argument : le num�ro du type demand� */
/* renvoie la place demand�e dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entr�e du chq si n�cessaire */
/* retourne -1 si pas trouv� */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_echeancier ( gint demande )
{
  GList *liste_tmp;
  gint retour;
  gint i;


  if ( !demande )
    return ( FALSE );

  liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_echeancier[7] ) -> menu ) -> children;
  retour = -1;
  i = 0;

  while ( liste_tmp && retour == -1 )
    {
      if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				 "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	  struct struct_type_ope *type;

	  retour = i;

	  /* affiche l'entr�e chq du formulaire si n�cessaire */

	  type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				       "adr_type");

	  /* soit c'est un type qui affiche l'entr�e et qui n'est pas num�rot� automatiquement */
	  /* soit c'est un type num�rot� auto et c'est une saisie */
 
	  if ( ( type -> affiche_entree && !type -> numerotation_auto)
	       ||
	       ( type -> numerotation_auto && !strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
							_("Input") )))
	    {
	      /* si c'est une saisie, mais le num�ro de chq */

	      if ( type -> numerotation_auto )
		{
		  entree_prend_focus ( widget_formulaire_echeancier[8] );
		  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[8] ),
				       itoa ( type -> no_en_cours + 1 ));
		}
	      gtk_widget_show ( widget_formulaire_echeancier[8] );
	    }
	  else
	    gtk_widget_hide ( widget_formulaire_echeancier[8] );
	}
      i++;
      liste_tmp = liste_tmp -> next;
    }

  if ( retour == -1 )
    return ( FALSE );
  else
    return ( retour );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_formulaire ( struct struct_type_ope *type )
{

  /* affiche l'entr�e de ch�que si n�cessaire */

  if ( type -> affiche_entree )
    {
      gtk_widget_show ( widget_formulaire_operations[10] );

      /* met le no suivant si n�cessaire */

      if ( type -> numerotation_auto )
	{
	  entree_prend_focus ( widget_formulaire_operations[10] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
			       itoa ( type -> no_en_cours  + 1));
	}
      else
	{
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
			       "" );
	  entree_perd_focus ( widget_formulaire_operations[10],
			      FALSE,
			      GINT_TO_POINTER ( 10 ));
	}
    }
  else
    gtk_widget_hide ( widget_formulaire_operations[10] );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_echeancier ( struct struct_type_ope *type )
{

  /* affiche l'entr�e de ch�que si n�cessaire */

  if ( ( type -> affiche_entree && !type -> numerotation_auto )
       ||
       ( type -> numerotation_auto && !strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
						_("Input") )))
    {
      /* si c'est une saisie, met le num�ro de chq */
      
      if ( type -> numerotation_auto )
	{
	  entree_prend_focus ( widget_formulaire_echeancier[8] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[8] ),
			       itoa ( type -> no_en_cours + 1 ));
	}
      gtk_widget_show ( widget_formulaire_echeancier[8] );
    }
  else
    gtk_widget_hide ( widget_formulaire_echeancier[8] );
}
/* ************************************************************************************************************** */
