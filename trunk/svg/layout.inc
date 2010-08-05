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


// This function places a list of charges according to the placement string,
// unless modified by something in the mods array. Items in the mods array
// take precedence over the settings of the placement string.
// The placement string is organised as follows:
// 1) Each number of charges has its own substring, separated by '/'
//    * If there is an empty substring the next highest (non-empty)
//      substring will be used and the higher places ignored.
//    * If there is only one substring it will be used for all
//    * If there is no sensible arrangement for this number of charges
//      the substring should be 'x'
// 2) Within the substring the placement of each charge is given by
//    a sub-substring, these are separeted by ':'
// 3) The sub-substring may consist of up to 7 numbers, separated by
//    ','. The numbers are as follows:
//    a) The X position of the centre of the charge
//    b) The Y position of the centre of the charge
//    c) The X size of the charge bounding box
//    d) The Y size of the charge bounding box
//    e) The rotation (in degrees) of the charge
//    f) 1 if the charge is inverted, 0 otherwise
//    g) 1 if the charge is reversed, 0 otherwise
// If any of the numbers are ommitted they are assumed to be the
// same as the preceeding entries. If there are no preceeding
// entries they will use default values equivalent to:
//  /500,500,200,200,0,0,0/
// It should be assumed that the charge list will be provided in order,
// left to right then top to bottom

function place_charges ( $node, $placement, $mods = array() ) {
  global $dom;

  switch ($placement) {
    case '':
    case 'i':
      my_trigger_error('Internal (no placement)', E_SVG );
      return '';
    case 'n':
      my_trigger_error('Not meaningful placement', E_SVG );
      return '';
  }
  $num_of = $node->get_attribute('number');
  $addorsed = false;
  $facing = null;
  $between = null;
  foreach ( $node->child_nodes() as $child ) {
    if ( $child->node_name() == 'modifier' ) {
      switch ( $child->get_attribute('name' )) {
        case 'facing':
          $facing = $child->get_attribute('param');
          break;
        case 'between':
          $between = $child->first_child();
          $num_of += $between->get_attribute('number');
          break;
      }
    }
  }

  $retval = '';
  $cx = 500; // default values
  $cy = 500;
  $rot = 0;
  $inv = 0;
  $rev = 0;
  $bbx = 200;
  $bby = 200;
  $num_of_list = explode('/', $placement);
  // Choose the placement substring to use
  $placelist = 's'; // default, assume no room
  if ( $num_of > count( $num_of_list ) ) {
    $place_list = 's';
  } else {
    $place_list = $num_of_list[$num_of - 1];
    if ( $place_list == '' ) { // use next highest
      for ( $i = $num_of; $i < count($num_of_list); $i++ )
        if ( $num_of_list[$i] != '' ) $place_list = $num_of_list[$i];
    }
  }
  switch($place_list{0}) {
    case '':
      my_trigger_error('Internal (bad placement)', E_SVG );
      return '';
    case 's':
      my_trigger_error('Not enough space for charges', E_SVG );
      return '';
    case 't':
      my_trigger_error('Not done charge placement yet (sorry)', E_SVG );
      return '';
    case 'x':
      my_trigger_error('Wrong number of charges to place', E_SVG );
      return '';
  }
  $place_list = explode(':', $place_list);
  if ( count($place_list) < $num_of ) {
    my_trigger_error('Internal (bad place list)', E_SVG );
    return '';
  }
  for ( $count = 0; $count < $num_of; $count++ ) {
    if ( $place_list[$count] == '' ) {
      my_trigger_error('internal (bad place item)', E_SVG );
      continue;
    }
    $items = explode(',', $place_list[$count]);
    if ( isset($items[0]) and $items[0] != '' ) {
      $cx = $items[0];
      if ( in_array( 'sinister', $mods ))$cx = 1000 - $cx;
    }
    if ( isset($items[1]) and $items[1] != '' ) {
     // my_trigger_error('x ' . implode(' ',$mods), E_SVG );
      $cy = $items[1];
      if ( in_array( 'inverted', $mods )) $cy = 1200 - $cy;
      if ( in_array( 'enhanced', $mods )) $cy -= 200;
      if ( in_array( 'abased', $mods )) $cy += 200;
    }
    if ( isset($items[2]) and $items[3] != '' ) $bbx = $items[2];
    if ( isset($items[3]) and $items[2] != '' ) $bby = $items[3];
    if ( isset($items[4]) and $items[4] != '' ) {
      $rot = $items[4];
      if ( in_array ( 'sinister', $mods )) { if ( $rot != 0 ) { $rot += 90; } }
      if ( in_array ( 'inverted', $mods )) { if ( $rot != 0 ) { $rot -= 90; } }
    }
    if ( isset($items[5]) and $items[5] != '' ) $inv = $items[5];
    if ( isset($items[6]) and $items[6] != '' ) $rev = $items[6];
    if ( $between and $count != 1 )
      $retval .= PlaceCharge( $between, $bbx, $bby, $cx, $cy, $rot, $inv, $rev );
    else
      $retval .= PlaceCharge( $node, $bbx, $bby, $cx, $cy, $rot, $inv, $rev );
  }
  return $retval;
}


function PlaceCharge ( $node,  // images
                            $boundX, $boundY,  // size of bounding box
                            $centreX, $centreY,// centre of bounding box
                            $rotation = '0',
                            $inverted = '0',
                            $reversed = '0' ) {
  global $dom;

  $charge = getCharge( $node );
  $orientation = null;
  $tincture = null;
//  $parent = $node->parent_node();
  foreach ( $node->child_nodes() as $child ) {
    if ( $child->node_name() == 'modifier' ) {
      switch ( $child->get_attribute('name') ) {
        case 'inverted':
          $inverted = true;
          break;
        case 'reversed':
          $reversed = true;
          break;
        case 'orientation':
          $orientation = $child->get_attribute('param');
          break;
      }
    } elseif ( $child->node_name() == 'tincture' ) {
      $tincture = $child;
    }
  }
  if ( $tincture == null ) {
    $col = $dom->create_element('colour');
    $col->set_attribute('name', 'gules');
    $tincture = $dom->create_element('tincture');
    $tincture->append_child($col);
  }
  // If there are multiple charges, choose the one with the best match to the bounding box
  $BoundAR = $boundX / $boundY;
  //if ( ! array_key_exists('body', $charge) ) { // must be an array of different charge shapes
  //  $closestAR = 9999;
  //  $bestMatch = 0;
  //  foreach ( $charge as $chg ) {
  //    $thisAR = $chg['width'] / $chg['height'];
  //    if ( abs($thisAR - $boundAR) < $closestAR ) {
  //      $closestAR = abs($thisAR - $boundAR);
  //     $bestMatch = $chg;
  //    }
  //  }
  //  $charge = $bestMatch;
  //}
  $retval = $charge['body'];
  $chargeX = $charge['width'];
  $chargeY = $charge['height'];
    $Xflex = array_key_exists('wflex', $charge) ? $charge['wflex'] : 1;
    $Yflex = array_key_exists('hflex', $charge) ? $charge['hflex'] : 1;

  if ( $inverted ) {
    $retval = "<g transform=\"translate(0,$chargeY) scale(1,-1)\"><desc>inverted charge</desc>$retval</g>\n";
  }
  if ( $reversed ) {
    $retval = "<g transform=\"translate($chargeX,0) scale(-1,1)\"><desc>reversed charge</desc>$retval</g>\n";
  }
  if ( $orientation ) {
    $rotation = 0; // An explicit charge orientation overrides a rotation from the arrangment
    switch ( $orientation ) {
      case 'fesswise':
        $do_turn = ( $chargeY > $chargeX );
        if ( array_key_exists('turn_fesswise', $charge) ) { $do_turn = $charge['turn_fesswise']; }
        if ( $do_turn ) {
          $retval = "<g transform=\"translate($chargeY,0) rotate(90)\"><desc>fesswise charge</desc>$retval</g>\n";
          $temp = $chargeX; $chargeX = $chargeY; $chargeY = $temp;
          $temp = $Xflex; $Xflex = $Yflex; $Yflex = $temp;
        }
        break;
      case 'bendwise':
        $movY = $chargeX * 0.707;
        $retval = "<g transform=\"translate(0,$movY) rotate(-45)\"><desc>bendwise charge</desc>$retval</g>\n";
        $chargeX = max ( $chargeX, $chargeY ) * 1.414;
        $chargeY = $chargeX;
        break;
      case 'bendsinwise':
        $movX = $chargeY * 0.707;
        $retval = "<g transform=\"translate($movX,0) rotate(45)\"><desc>bendwise sinister charge</desc>$retval</g>\n";
        $chargeX = max ( $chargeX, $chargeY ) * 1.414;
        $chargeY = $chargeX;
        break;
      case 'palewise':
        $do_turn = $chargeX >= $chargeY;
        if ( array_key_exists('turn_palewise', $charge) ) { $do_turn = $charge['turn_palewise']; }
        if ( $do_turn  ) {
          $retval = "<g transform=\"translate(0,$chargeX) rotate(-90)\"><desc>palewise charge</desc>$retval</g>\n";
          $temp = $chargeX; $chargeX = $chargeY; $chargeY = $temp;
          $temp = $Xflex; $Xflex = $Yflex; $Yflex = $temp;
        }
        break;
    }
  }

  // Calculate the size of the image to fit into the bounding box,
  $Xscale = $boundX / $chargeX;
  $Yscale = $boundY / $chargeY;
  if ( $Xflex <= 1 and $Yflex <=1 ) { // no flexing, fixed AR
    $Xscale = $Yscale = min ( $Xscale, $Yscale );
  } else {
    $chargeAR = $chargeX / $chargeY;
    $MaxAR = $chargeAR * $Xflex;
    $MinAR = $chargeAR / $Yflex;
    if ( $BoundAR > $MaxAR ) { // BB is wider than we can be
      $limit = $MaxAR;
    } elseif ( $BoundAR < $MinAR ) { // BB is higher than we can be
      $limit = $MinAR;
    } else {
    $limit = 0;  // We can flex as required to fill whole BB
    // $Xscale and $Yscale are already set to the correct values
    }
    if ( $limit > 0 ) {
      if ( $Xscale < $Yscale ) {
        $Yscale = ($chargeX*$Xscale)/($limit*$chargeY);
      } else {
        $Xscale = ($limit * $chargeY * $Yscale) / $chargeX;
      }
    }
  }
  $sizeX = $chargeX * $Xscale;
  $sizeY = $chargeY * $Yscale;
  $posX = $centreX - ($sizeX / 2);
  $posY = $centreY - ($sizeY / 2);
  $transform = '';
  if ( ! array_key_exists('orient_to_ordinary', $charge)) { $rotation = 0 ;}
  if ( $rotation != 0 ) {
    $transform .= 'rotate(' . $rotation . ',' . $centreX . ',' . $centreY . ') ';
  }
  $transform .= " translate($posX,$posY) scale($Xscale, $Yscale) ";
  $retval = "<g transform=\"$transform\" >\n<title>" . $charge['title'] . '</title><desc>' . $charge['desc'] . "</desc>$retval</g>\n";

  $retval = apply_tincture( $tincture, $retval, "$chargeX,$chargeY", $rotation, $reversed, $inverted,
             1000-$posX, 1200-$posY, $Xscale, $Yscale  );
  return $retval;
}

function calcPlace( $type, $number, $conjoined = 'false', $bb = '100,200,800,800'  ) {
  $wide_rows = array (
    '0',          // 0
    '1',          // 1
    '2',          // 2
    '2,1',        // 3
    '3,1',        // 4
    '3,2',        // 5
    '3,2,1',      // 6
    '3,3,1',      // 7
    '3,3,2',      // 8
    '3,3,3',      // 9
    '4,3,2,1',    // 10
    '4,3,3,1',    // 11
    '4,3,3,2',    // 12
    '4,3,3,3',    // 13
    '5,4,3,2',    // 14
    '5,4,3,2,1',  // 15
    '5,4,3,3,1',  // 16
    '5,4,3,3,2',  // 17
    '6,5,4,2,1',  // 18
    '6,5,4,3,1',  // 19
    '6,5,4,3,2',  // 20
  );
  $narrow_rows = array (
    '0',
    '1',
    '1,1',
    '1,1,1',
    '2,2',
    '2,2,1',
    '2,2,2',
    '3,2,2',
    '3,3,2',
    '3,3,3',
    '4,3,3',
    '4,4,3',
    '4,4,4',      // 12
    '4,4,3,2',    // 13
    '4,4,3,2,1',  // 14
    '4,4,4,3',    // 15
    '4,4,4,4',    // 16
    '4,4,4,4,1',  // 17
    '4,4,4,4,2',  // 18
    '4,4,4,4,3',  // 19
    '4,4,4,4,4',  // 20
  );
  if ( $number > 20 ) return $number;
  switch ( $type ) {
    case 'h': // single horizontal row
      $row_list = array($number);
      break;
    case 'v': // single vertical row
      $row_list = array_fill(0,$number,'1');
      break;
    case 'r': // use as rowlist
      $row_list = explode(',', $number);
      $number = array_sum($row_list);
      break;
    case 'w': // wide box
      $row_list = explode(',',$wide_rows[$number]);
      break;
    case 'n': // narrow box
      $row_list = explode(',',$narrow_rows[$number]);
      break;
  }
  
  $placement = str_repeat('x/', $number-1);
  $space_pct = 0.2;
  $max_row = max($row_list);
  $num_rows = count($row_list);
  list($bb_x,$bb_y,$bb_w,$bb_h) = explode(',',$bb);
  if ( $conjoined == 'true' ) {
    // Calculate the charge bounding box (same size for all)
    $cbb_height = $bb_h / $num_rows;
    $cbb_width = $bb_w / $max_row;
    for ( $i = 0; $i < $num_rows; $i++ ) {
      $centre_y = $bb_y + ($cbb_height * $i) + ($cbb_height / 2);
      for ( $j = 0; $j < $row_list[$i]; $j++ ) {
        if ( $row_list[$i] == $max_row and $max_row != 1 ) {
          $centre_x = $bb_x + ($cbb_width * $j) + ($cbb_width/2);
        } else {
          $centre_x = $bb_x + ((1000 - $bb_x - $bb_x - ($cbb_width * $row_list[$i]))/2) + ($cbb_width * $j) + ($cbb_width/2);
        }
        $placement .= sprintf ( "%.2f,%.2f,%.2f,%.2f:",$centre_x,$centre_y,$cbb_width,$cbb_height);
      }
    }
  } else {
    // Calculate the charge bounding box (same size for all)
    $vert_gap = ($num_rows == 1) ? 0 : ($bb_h * $space_pct) / ($num_rows-1);
    $cbb_height = ($num_rows == 1) ? $bb_h : ($bb_h * (1-$space_pct)) / $num_rows;
    $cbb_width = ($max_row == 1 ) ? $bb_w : ($bb_w * (1-$space_pct)) / $max_row;
    $horiz_gap = ($bb_w * $space_pct) / max(1,($max_row - 1));
    for ( $i = 0; $i < $num_rows; $i++ ) {
      $centre_y = $bb_y + (($cbb_height + $vert_gap) * $i) + ($cbb_height / 2);
      $offset = ($bb_w - ($cbb_width * $row_list[$i]) - ($horiz_gap * ($row_list[$i]-1)))/2;
      for ( $j = 0; $j < $row_list[$i]; $j++ ) {
        if ( $row_list[$i] != 1 ) {
          $centre_x = $bb_x + $offset + (($cbb_width + $horiz_gap) * $j) + ($cbb_width/2);
        } else {
          $centre_x = $bb_x + ($bb_w/2);
        }
        $placement .= sprintf ( "%.2f,%.2f,%.2f,%.2f:",$centre_x,$centre_y,$cbb_width,$cbb_height);
      }
    }
  }
  return $placement;
}?>