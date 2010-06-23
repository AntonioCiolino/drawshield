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


function charge_lion ( &$node, &$mods ) {
  $lion_data = array (
    array ( 'armed', 'armed' ),
    array ( 'tongued', 'langued' ),
    array ( 'crowned', 'crowned' ),
  );

  $lion_types = array (
    array ( 'rampant', 'rampant' ),
    array ( 'salient', 'salient' ),
    array ( 'passant', 'passant' ),
    array ( 'statant', 'statant' ),
    array ( 'sejant',  'sejant' ),
    array ( 'sejant-affronte',  'sejant affronte'  ),
    array ( 'sejant-erect',  'sejant erect'  ),
    array ( 'couchant', 'couchant' ),
  );
  if ( ($match = search_match($lion_types)) != null ) {
    $type = 'lion-' . $match[0];
  } else { // default lion
    $type = 'lion-rampant';
  }
  $node->set_attribute('subtype', $type);
  return $lion_data;
}
?>