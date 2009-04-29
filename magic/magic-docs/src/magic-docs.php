<?
/* This PHP file contains all the documentation for the various Magic Tools
that are shipped with Tux Paint, as well as code and templates to generate
individual HTML files for each of them, and an index.html that links to
them all. */

/* Bill Kendrick <bill@newbreedsoftware.com> */
/* 2008.04.28 */


/* Authors of the Magic tools: */

$AUTHOR_KENDRICK = "Bill Kendrick|bill@newbreedsoftware.com";
$AUTHOR_ALBERT = "Albert Cahalan|albert@users.sf.net";


/* Information about each of the tools:

   'name' is the name of the tool; the name for the HTML file is based on
   this (all lowercase, with spaces stripped)

   e.g.: "My Magic Tool"   (and the file will be "mymagictool.html")


   'author' is the author's name and email, separated by a '|' character;
   it may be an array.  Try to add authors as constant vars above, so they
   can be accurately reused or updated.

   e.g. "Joe Schmoe|joe@sch.org"
   or array("Joe Schmoe|joe@sch.org", "Another Guy|a.guy@inter.net")


   'desc' is the description, in HTML. (It will be wrapped in <p>...</p>).


   'see' is optional.  It should be the name of another tool to link to
   (same format as 'name'; it will be converted for use as a link).

   e.g. "Related Magic Tool"  (will link to "relatedmagictool.html")
   or array("Related One", "Related Two")


   NOTE: If an image "ex_shortname.png" exists in html/images/,
   it will be referred to in an <img> tag in the output.
*/


$tools = array(
  array('name'=>'Blocks',
   'desc'=>'This makes the picture blocky looking ("pixelated") wherever you drag the mouse.',
   'author'=>array($AUTHOR_KENDRICK,$AUTHOR_ALBERT)),

  array('name'=>'Blur',
   'desc'=>'This makes the picture fuzzy wherever you drag the mouse.',
   'author'=>array($AUTHOR_KENDRICK,$AUTHOR_ALBERT)),

  array('name'=>'Bricks',
   'desc'=>'These two tools intelligently paint large and small brick patterns on the canvas.  The bricks can be tinted various redish hues by selecting different colors in the color palette.',
   'author'=>$AUTHOR_ALBERT),

  array('name'=>'Calligraphy',
   'desc'=>'This paints on the canvas with a calligraphy pen. The quicker you move, the thinner the lines.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Cartoon',
   'desc'=>'This makes the picture look like a cartoon &mdash; with thick outlines and bright, solid colors &mdash; wherever you move the mouse.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Chalk',
   'desc'=>'This makes parts of the picture (where you move the mouse) look like a chalk drawing.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Darken',
   'desc'=>'This dakrens the colors wherever you drag the mouse.  (Do it to the same spot many times, and it will eventually become black.)',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>array('Fade', 'Tint')),

  array('name'=>'Distortion',
   'desc'=>'This slightly distorts the picture wherever you move the mouse.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Drip',
   'desc'=>'This makes the paint "drip" wherever you move the mouse.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Emboss',
   'desc'=>'This makes parts of your picture look "embossed."  Wherever there are sharp edges in your picture, the picture will look raised like it was stamped in metal.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Fade',
   'desc'=>'This fades the colors wherever you drag the mouse.  (Do it to the same spot many times, and it will eventually become white.)',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>array('Darken', 'Tint')),

  array('name'=>'Fill',
   'desc'=>'This floods the picture with a color.  It lets you quickly fill parts of the picture, as if it were a coloring book.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Flip',
   'desc'=>'Similar to "Mirror."  Click and the entire image will be turned upside-down.',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>'Mirror'),

  array('name'=>'Flower',
   'desc'=>'This tool draws small flowers, with leafy bases and stalks.  Click to set the base, then drag the mouse upwards to drawe the stalk, and finally release the mouse button to finish the flower.  It will be drawn in the currently-selected color.  The shape and length of the stalk depends on how you move the mouse while you drag.',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>'Grass'),

  array('name'=>'Foam',
   'desc'=>'Click and drag the mouse to draw foamy bubbles.  The more you drag the mouse in a particular spot, the more likely small bubbles will combine to form bigger bubbles.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Glass Tile',
   'desc'=>'Click and drag over your picture to make it look like it\'s being seen through glass tiles.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Grass',
   'desc'=>'This paints grass on the image. The higher up the canvas, the smaller the grass is drawn, giving an illusion of perspective.  The grass can be tinted various greenish hues by selecting different colors in the color palette.',
   'author'=>$AUTHOR_ALBERT,
   'see'=>'Flower'),

  array('name'=>'Kaleidoscope',
   'desc'=>'This paint brush draws in four places at the same time, mirroring symmetrically, both horizontally and vertically.  It uses the currently selected color.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Light',
   'desc'=>'This draws a glowing beam on the canvas, in the currently-selected color.  The more you use it on one spot, the more white it becomes.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Metal Paint',
   'desc'=>'Click and drag to draw shiny metal using the current color.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Mirror',
   'desc'=>'When you click the mouse in your picture with the "Mirror" magic effect selected, the entire image will be reversed, turning it into a mirror image.',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>'Flip'),

  array('name'=>'Negative',
   'desc'=>'This inverts the colors wherever you drag the mouse.  (e.g., white becomes black, and vice versa.)',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Rainbow',
   'desc'=>'This is similar to the paint brush, but as you move the mouse around, it cycles through a spectrum of bright colors.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Ripples',
   'desc'=>'Click in your picture to make water ripple distortions appear over it.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Shift',
   'desc'=>'This shifts your picture around the canvas.  Anything that gets shifts off an edge reappears on the opposite edge.',
   'author'=>$AUTHOR_KENDRICK),

  array('name'=>'Smudge',
   'desc'=>'This pushes the colors around under the mouse, like finger painting with wet paint.',
   'author'=>$AUTHOR_ALBERT),

  array('name'=>'Tint',
   'desc'=>'This changes the color (or hue) of the parts of the picture to the selected color.',
   'author'=>$AUTHOR_KENDRICK,
   'see'=>array('Fade', 'Darken')),

  array('name'=>'Waves',
   'desc'=>'Click to make the entire picture wavy.  Drag the mouse up and down to change the height of the ripples, and left and right to change the width.  Release the mouse button when it looks the way you like it.',
   'author'=>$AUTHOR_KENDRICK),

);

$fiidx = fopen("../html/index.html", "w");

fwrite($fiidx, page_header("List of Magic Tools"));

foreach ($tools as $t) {

  $shortname = str_replace(' ','', strtolower($t['name']));

  $out = page_header($t['name']);
  $out .= "<h2 align=\"center\">By ";
  if (is_array($t['author'])) {
    foreach ($t['author'] as $a) {
      list($authname, $authemail) = split('\|', $a);
      $out .= $authname." &lt;<a href=\"mailto:".$authemail."\">".$authemail."</a>&gt;<br>\n";
    }
  } else {
    list($authname, $authemail) = split('\|', $t['author']);
    $out .= $authname." &lt;<a href=\"mailto:".$authemail."\">".$authemail."</a>&gt;";
  }

  $out .= "</h2>\n";

  $out .= "<p>".$t['desc']."</p>\n";

  if (!empty($t['see'])) {
    $out .= "<p>See also: ";
    if (is_array($t['see'])) {
      foreach ($t['see'] as $s) {
        $out .= "<a href=\"".str_replace(' ', '', strtolower($s)).".html\">".$s."</a> ";
      }
    } else {
      $out .= "<a href=\"".str_replace(' ', '', strtolower($t['see'])).".html\">".$t['see']."</a>";
    }
    $out .= "</p>\n";
  }

  if (file_exists("../html/images/ex_".$shortname.".png")) {
    $out .= "<p align=center><img src=\"images/ex_".$shortname.".png\"></p>\n";
  }

  $out .= page_footer();

  $fi = fopen("../html/".$shortname.".html", "w");
  fwrite($fi, $out);
  fclose($fi);

  $link = "<li><a href=\"".$shortname.".html\">".$t['name']."</a></li>\n";

  fwrite($fiidx, $link);
}

fwrite($fiidx, page_footer());

fclose($fiidx);

function page_header($title)
{
  return "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n".
    "<body><html><head><title>Tux Paint \"Magic\" Tool: ".$title."</title>\n".
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=us-ascii\">\n".
    "</head>\n".
    "<body bgcolor=\"#FFFFFF\" text=\"#000000\" link=\"#0000FF\" vlink=\"#FF0000\" alink=\"#FF00FF\">\n".
    "<h1 align=\"center\">Tux Paint \"Magic\" Tool: ".$title."</h1>\n";
}

function page_footer()
{
  return "</body></html>";
}

?>
