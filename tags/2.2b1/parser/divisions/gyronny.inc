<?php /* Copyright 2010 Karl R. Wilcox 

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */


function division_gyronny( $node ) {
  global $dom;

  $of = array (
    array ( true, 'of' ),
  );
  $pieces = array (
    array ( true, 'pieces' ),
  );

  $node->set_attribute('linetype', linetype());
  $type = $node->get_attribute('subtype');
  if ( $type == 'quarterly-quartered' ) {
    $node->set_attribute('subtype','gyronny');
    $mod = make_mod('ofnum','8');
    $node->append_child($mod);
    return $node;
  }

  $state = save();

  if( ($match = search_match($of)) != null ) {
    if ( ($num = number()) != null ) {
      search_match($pieces);
      $mod = make_mod('ofnum', "$num");
      $node->append_child($mod);
    } else {
      restore($state);
    }
  }
  return $node;
}
?>