ABOUT

Flippy is an ultra-simple module that generates previous/next pagers for content
types. On the Node type administration screen, you can turn Flippy on or off
for each content type. It'll generate a unique pager for each content type, so
you can page through Articles or page through Blog posts, but you can't page
through all nodes on the site.

To change the position of the pager, visit the content type's Manage Display
page, where fields and other fieldish things can be dragged about. Flippy's
there, too.

If you turn on Flippy for a content type, but hide the pager in the Manage
Display page, its links will still be available in the theme layer when it comes
time to output node.tpl.php. If you like, you can weave the pager links right
into the node manually in your own templates.

HISTORY

Flippy started out as a quick and dirty port of Custom Pagers module to Drupal 7
but quickly spiraled into an over-engineered pile of wacky. It was gutted and
now it's very simple. It might get more features later.

USAGE

- Go to /admin/modules and enable Flippy module.
- Go to /admin/structure/types, edit the content type that you want to have flippy enabled.
- tick 'Build a pager for this content type' checkbox in 'Flippy settings' tab. 
- Select advanced options.
  - Whether to add semantic previous and next links to the document HEAD.
  - Whether to show empty links. For example, if there is no "next" node, the "next" label will be shown but without a link.
  - Input the label you want to display for the links. You can use token to display element of node.
  - Whether to display first/last links. Will ask you to input the label if you tick the checkbox.
  - Whether to loop through the nodes, there won't be first and last node.
  - Whether to show the random link. Label is needed if checked.
  - Set up 'Truncate label length'. Long text will be truncated.
  - Set up 'Truncate ellipse'.
  - Sort by other fields.
    - If checked, choose the field for sorting the pager.
    - Select pager order.
  - Click the save content type button to save the settings. 

REQUIREMENTS

- Drupal 7
- token module

AUTHOR

Jeff Eaton "eaton" (http://drupal.org/user/16496)
