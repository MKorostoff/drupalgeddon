<?php

/**
 * Implements theme_preprocess_page.
 * This fixes a bug in top_node where the page title fails to show up.
 *
 * @todo
 *   Generalize this fix to respect views page title settings and contribute it
 *   back to the top node project.  While we're at it, try and fix that problem
 *   where top_node replaces the $_GET['q'] variable.
 */
function whaleocalypse_preprocess_page( &$vars ) {
	$node = menu_get_object();
	if ( is_object( $node ) && $node->type == 'comic' ) {
		$vars['title'] = l( $node->title, 'node/' . $node->nid );
	}
	if ($vars['logo']) {
		//pass the site logo through the l() function
        $logo_image = '<img src="' . $vars['logo'] . '" alt="Home"/>';
	    $vars['logo'] = l($logo_image, 'main', array("html" => TRUE, "attributes" => array("title" => "Home", "rel" => "home", "id" => "logo")));
	}
}

/**
 * Implements theme_preprocess_comment.  This controls the display of the
 * username in comments.  Logic works like this:
 *
 * 1) if the commenter does not supply an author name, default to 'Anonymous'
 * 2) if the author does supply a name, show the name.
 * 3) if the author supplies a homepage, link the name to the homepage (i.e. <a href="homepage">author</a>)
 *
 * We'll also use this to get the gravatar if one exists and cache a 60x60 pixel
 * version using imagecache_external.
 *
 * @todo
 *   Fix it so that my personal picture and username link to mattkorostoff.com
 *   instead of the drupal user profile page.
 */
function whaleocalypse_preprocess_comment( &$vars ) {
	$cid = $vars['elements']['#comment']->cid;
	$author_name = $vars['elements']['#comment']->name;
	if ( 'Anonymous' == $author_name ) {
		$result = db_query( 'SELECT c.name,c.homepage,c.mail FROM {comment} c WHERE c.cid = :cid', array( ':cid' => $cid ) )->fetchAssoc();
		$vars['real_name'] = t( 'Anonymous' );

		$email = '';
		if ( $result['mail'] ) {
			$email = $result['mail'];
		}
		//Light weight gravatar implementation
		$gravatar = "http://www.gravatar.com/avatar/" .
			md5( strtolower( trim( $email ) ) ) .
			"?d=" . urlencode( 'identicon' ) .
			"&s=60";

		$vars['picture'] = '<div class="user-picture"><img src="' . $gravatar . '" /></div>';

		if ( $result['name'] ) {
			$vars['author'] = $result['name'];
		}

		if ( $result['homepage'] ) {
			$vars['author'] = l( $vars['author'], $result['homepage'], array( "attributes" => array( "target" => "_blank" ) ) );
		}
	}
}

/**
 * Add the comic strip name right above the body field.
 */
function whaleocalypse_preprocess_field( &$vars ) {
	if ( isset( $vars['element']['#bundle'] ) &&
		$vars['element']['#bundle'] == 'comic' &&
		isset( $vars['element']['#field_name'] ) &&
		$vars['element']['#field_name'] == 'body' ) {
		$vars['label'] = $vars['element']['#object']->title;
	}
	//Add the transcript expand js if the field is populated
	if ( $vars['element']['#field_name'] == 'field_transcript' ) {
		drupal_add_js( drupal_get_path( 'theme', 'whaleocalypse' ) .'/js/expand-transcript.js', array( 'scope' => 'footer', 'type' => 'file' ));
	}
	//Create the custom "story arc" functionality
	if ( isset($vars['element']['#field_name']) && $vars['element']['#field_name'] == 'field_story_arc') {
		$tid = $vars['element']['#object']->field_story_arc[LANGUAGE_NONE][0]['tid'];
		$nid = $vars['element']['#object']->nid;
		$story_arc_count = new EntityFieldQuery();
		$vars['story_arc_count'] = $story_arc_count->entityCondition('entity_type', 'node')
  												   ->entityCondition('bundle', 'comic')
  												   ->fieldCondition('field_story_arc', 'tid', $tid, '=')
  												   ->count()
  												   ->execute();

		$story_arc_position = new EntityFieldQuery();
		$story_arc_position = $story_arc_position->entityCondition('entity_type', 'node')
  												   ->entityCondition('bundle', 'comic')
  												   ->fieldCondition('field_story_arc', 'tid', $tid, '=')
  												   ->propertyOrderBy('created', 'ASC')
  												   ->execute();
  		
  		$vars['story_arc_position'] = array_search( $nid, array_keys( $story_arc_position['node'] ) ) + 1;
	}


	//Add the author URL if it is set.
	if ( isset($vars['element']['#field_name']) && $vars['element']['#field_name'] == 'field_contributing_author') {
		if ( isset( $vars['element']['#object']->field_contributing_author_url[LANGUAGE_NONE][0] ) ) {
			$vars['items'][0]['#markup'] = l($vars['items'][0]['#markup'], $vars['element']['#object']->field_contributing_author_url[LANGUAGE_NONE][0]['value'], array("attributes" => array("target" => "_blank") ) );
		}
		$vars['call_to_action'] = 'You can ' . l( 'write your own whaleocalypse too.', 'node/169', array() );
	}
}

function whaleocalypse_link($variables) {
	if ( isset( $_COOKIE['never_cache_again'] ) ) {
		$rand = substr(md5(microtime()),rand(0,26),5);
		return '<a href="' . check_plain(url($variables['path'], $variables['options'])) . '?' . $rand . '"' . drupal_attributes($variables['options']['attributes']) . '>' . ($variables['options']['html'] ? $variables['text'] : check_plain($variables['text'])) . '</a>';
	}
	else {
		return '<a href="' . check_plain(url($variables['path'], $variables['options'])) . '"' . drupal_attributes($variables['options']['attributes']) . '>' . ($variables['options']['html'] ? $variables['text'] : check_plain($variables['text'])) . '</a>';
	}
}