                                   Tux Paint
                                 version 0.9.26

  A simple drawing program for children

           Copyright © 2002-2021 by divers contributeurs; cf AUTHORS.
                            http://www.tuxpaint.org/

                                  mars 9, 2021

     ----------------------------------------------------------------------

   +-------------------------------------------+
   |Table of Contents                          |
   |-------------------------------------------|
   |  * About Tux Paint                        |
   |  * Using Tux Paint                        |
   |       * Launching Tux Paint               |
   |       * Title Screen                      |
   |       * Main Screen                       |
   |       * Available Tools                   |
   |            * Outils de dessin             |
   |            * Other Controls               |
   |  * Loading Other Pictures into Tux Paint  |
   |  * Further Reading                        |
   |  * How to Get Help                        |
   |  * How to Participate                     |
   +-------------------------------------------+

     ----------------------------------------------------------------------

                                About Tux Paint

Qu'est-ce que "Tux Paint" ?

       Tux Paint est un programme de dessin gratuit conçu pour les jeunes
       enfants (enfants de 3 ans et plus). Il possède une interface simple et
       facile à utiliser, des effets sonores amusants et une mascotte de
       dessin animé pour encourager et guider les enfants lorsqu'ils
       utilisent le programme. Sont fournis une toile vierge et un ensemble
       d'outils de dessin pour aider votre enfant à être créatif.

Licence :

       Tux Paint est un projet Open Source, un logiciel libre publié sous la
       licence publique générale GNU (GPL). Il est gratuit et le «code
       source» du programme est disponible. (Cela permet à d'autres d'ajouter
       des fonctionnalités, de corriger des bogues et d'utiliser des parties
       du programme dans leur propre logiciel sous GPL.)

       See COPYING.txt for the full text of the GPL license.

Objectifs :

       Facile et amusant
               Tux Paint se veut un simple programme de dessin pour les
               jeunes enfants. Il ne s'agit pas d'un programme de dessin à
               usage général. Il se veut amusant et facile à utiliser. Les
               effets sonores et un personnage de dessin animé permettent à
               l'utilisateur de savoir ce qui se passe tout en le
               divertissant. Il existe également des pointeurs de souris, de
               style dessin animé, extra-larges.

       Flexibilité
               Tux Paint est extensible. Les brosses et les formes de "tampon
               en caoutchouc" peuvent être déposées et retirées. Par exemple,
               un enseignant peut apporter une collection de formes animales
               et demander à ses élèves de dessiner un écosystème. Chaque
               forme peut avoir un son qui l'accompagne et un texte est
               affiché lorsque l'enfant sélectionne la forme.

       Portabilité
               Tux Paint est portable sur diverses plates-formes
               informatiques: Windows, Macintosh, Linux, etc. L'interface est
               la même sur toutes. Tux Paint fonctionne correctement sur les
               systèmes plus anciens (comme un Pentium 133), et peut être
               modifié pour mieux fonctionner sur des systèmes lents.

       Simplicité
               Il n'y a pas d'accès direct à la complexité sous-jacente de
               l'ordinateur. L'image en cours est conservée lorsque le
               programme se ferme et réapparaît au redémarrage.
               L'enregistrement d'images ne nécessite pas la création de noms
               de fichiers ou l'utilisation du clavier. L'ouverture d'une
               image se fait en la sélectionnant dans une collection de
               vignettes. L'accès à d'autres fichiers sur l'ordinateur est
               restreint.

     ----------------------------------------------------------------------

                                Using Tux Paint

Launching Tux Paint

  Linux/Unix Users

         Tux Paint devrait avoir mis une icône de lanceur dans vos menus KDE
         et / ou GNOME, sous «Graphiques».

         Autrement, vous pouvez exécuter la commande suivante à une invite du
         shell (par exemple, "$") :

           $ tuxpaint

         Si des erreurs se produisent, elles seront affichées sur le terminal
         ("stderr").

     ----------------------------------------------------------------------

  Windows Users

                                                      [Icône pour Tux Paint]  
                                                            Tux Paint         

         Si vous avez installé Tux Paint sur votre ordinateur en utilisant le
         'Tux Paint Installer', il vous aura demandé si vous vouliez un
         raccourci de menu 'Démarrer' et / ou un raccourci sur le bureau. Si
         vous avez accepté, vous pouvez simplement exécuter Tux Paint à
         partir de la section 'Tux Paint' de votre menu 'Démarrer' (par
         exemple, sous «Tous les programmes» sous Windows XP), ou en
         double-cliquant sur l'icône «Tux Paint» sur votre bureau .

         Si vous avez installé Tux Paint en utilisant le téléchargement
         'ZIP-file', ou si vous avez utilisé 'Tux Paint Installer', mais que
         vous avez choisi de ne pas installer de raccourcis, vous devrez
         double-cliquer sur l'icône "tuxpaint.exe" dans le dossier "Tux
         Paint" de votre ordinateur.

         Par défaut, le 'Tux Paint Installer' mettra le dossier de Tux Paint
         dans "C:\Program Files\",ou bien vous avez pu changer cela lors de
         l'exécution du programme d'installation.

         Si vous avez utilisé le téléchargement de 'ZIP-file', le dossier de
         Tux Paint sera là où vous l'avez placé lorsque vous avez décompressé
         le fichier ZIP.

     ----------------------------------------------------------------------

  macOS Users

         Double-cliquez simplement sur l'icône "Tux Paint".

     ----------------------------------------------------------------------

Title Screen

       Lors du premier chargement de Tux Paint, un écran avec titre et
       crédits apparaîtra.

                                 [Écran-titre]

       Une fois le chargement terminé, appuyez sur une touche ou cliquez sur
       la souris pour continuer. (Ou, après environ 30 secondes,
       l'écran-titre disparaîtra automatiquement.)

     ----------------------------------------------------------------------

Main Screen

       L'écran principal est divisé en plusieurs sections :

       Côté gauche : la barre d'outils

               La barre d'outils contient les commandes pour dessiner et
               éditer.

  [Outils : Peindre, Tampon, Lignes, Formes, Texte, Magie, Étiquette, Défaire,
        Refaire, Gomme, Nouveau, Ouvrir, Sauvegarder, Imprimer, Quitter]

       Milieu : Toile pour Dessiner

               La plus grande partie de l'écran, au centre, est la toile de
               dessin. C'est évidemment là que vous dessinerez !

                                    [Toile]

               Remarque: la taille de la toile de dessin dépend de la taille
               de Tux Paint. Vous pouvez modifier la taille de Tux Paint à
               l'aide de l'outil de configuration tuxpaint-config, ou par
               d’autres moyens. Consultez la documentation OPTIONS pour plus
               de détails.

       Côté droit : sélecteur

               En fonction de l'outil en cours d'utilisation, le sélecteur
               affiche différentes choses. Par exemple, lorsque l'outil
               Pinceau est sélectionné, il affiche les différents pinceaux
               disponibles. Lorsque l'outil Tampon en caoutchouc est
               sélectionné, il affiche les différentes formes que vous pouvez
               utiliser.

                 [Selectors - Brushes, Letters, Shapes, Stamps]

       En bas : couleurs

               Une palette de couleurs disponibles s'affiche en bas de
               l'écran.

 [Colors - Black, White, Red, Pink, Orange, Yellow, Green, Cyan, Blue, Purple,
                                  Brown, Grey]

               À l'extrême droite se trouvent deux options de choix de
               couleur, le "sélecteur de couleurs", qui a le contour d'un
               compte-gouttes, et vous permet de choisir une couleur trouvée
               dans votre dessin, et la palette arc-en-ciel, qui vous permet
               de choisir une couleur dans une boîte contenant des milliers
               de couleurs.

               (REMARQUE: vous pouvez définir vos propres couleurs. Voir la
               documentation " Options ".)

       En bas : zone d'aide

               Tout en bas de l'écran, Tux, le pingouin Linux, fournit des
               conseils et d'autres informations pendant que vous dessinez.

(Par exemple : Choisis une forme. Clique pour démarrer le dessin , fais glisser
  et continue jusqu'à la taille désirée. Déplace-toi pour la faire tourner, et
                             clique pour dessiner.)

     ----------------------------------------------------------------------

Available Tools

  Outils de dessin

         Outil "Peinture" (pinceau)

                 L'outil Pinceau vous permet de dessiner à main levée, en
                 utilisant différents pinceaux (choisis dans le sélecteur à
                 droite) et couleurs (choisis dans la palette de couleurs qui
                 est en bas).

                 Si vous maintenez le bouton de la souris enfoncé et déplacez
                 la souris, elle dessine au fur et à mesure que vous vous
                 déplacez.

                 Pendant que vous dessinez, un son est joué. Plus la brosse
                 est grosse, plus le son est grave.

     ----------------------------------------------------------------------

         Outil "Tampon" (tampon de caoutchouc)

                 L'outil Tampon montre un ensemble de tampons en caoutchouc
                 ou d'autocollants. Il vous permet de coller des images
                 pré-dessinées ou photographiques (comme l'image d'un cheval,
                 d'un arbre ou de la lune) dans votre dessin.

                 Lorsque vous déplacez la souris sur le canevas, une forme
                 suit la souris, indiquant où le tampon sera placé, ainsi que
                 sa taille.

                 Il peut y avoir de nombreuses catégories de timbres (par ex.
                 animaux, plantes, espace extra-atmosphérique, véhicules,
                 personnes, etc.). Utilisez les flèches gauche et droite pour
                 parcourir les différentes collections.

                 Avant de `` tamponner '' une image sur votre dessin,
                 différents effets peuvent parfois être appliqués (en
                 fonction du tampon) :

                    * Certains tampons peuvent être colorés ou teintés. Si la
                      palette de couleurs sous le canevas est activée, vous
                      pouvez cliquer sur les couleurs pour changer la teinte
                      ou la couleur du tampon avant de le placer dans le
                      dessin.
                    * Les tampons peuvent être rétrécis et agrandis, en
                      cliquant dans l'ensemble de barres (de forme
                      triangulaire) en bas à droite; plus la barre est
                      grande, plus le tampon apparaîtra grand sur votre
                      dessin.
                    * De nombreux tampons peuvent être retournés
                      verticalement ou affichés sous forme d'image miroir à
                      l'aide des boutons de commande en bas à droite.

                 Les tampons peuvent avoir un effet sonore et / ou une
                 description orale (parlés). Les boutons en bas à gauche
                 (près de Tux, le pingouin Linux) vous permettent de rejouer
                 les effets sonores et la description du tampon actuellement
                 sélectionné.

                 (REMARQUE: Si l'option "nostampcontrols" est définie, Tux
                 Paint n'affichera pas les commandes Miroir, Retourner,
                 Réduire et Agrandir pour les tampons. Voir la documentation
                 " Options".)

     ----------------------------------------------------------------------

         "Lines" Tool

                 This tool lets you draw straight lines using the various
                 brushes and colors you normally use with the Paint Brush.

                 Click the mouse and hold it to choose the starting point of
                 the line. As you move the mouse around, a thin 'rubber-band'
                 line will show where the line will be drawn.

                 Let go of the mouse to complete the line. A "sproing!" sound
                 will play.

     ----------------------------------------------------------------------

         "Shapes" Tool

                 This tool lets you draw some simple filled, and un-filled
                 shapes.

                 Select a shape from the selector on the right (circle,
                 square, oval, etc.).

                 Use the options at the bottom right to choose the shape
                 tool's behavior:

                      Shapes from center
                              The shape will expand from where you initially
                              clicked, and will be centered around that
                              position. (This was Tux Paint's only behavior
                              through version 0.9.24.)

                      Shapes from corner
                              The shape will extend with one corner starting
                              from where you initially clicked. This is the
                              default method of most other traditional
                              drawing software. (This option was added
                              starting with Tux Paint version 0.9.25.)

                 Note: If shape controls are disabled (e.g., with the
                 "noshapecontrols" option), the controls will not be
                 presented, and the "shapes from center" method will be used.

                 In the canvas, click the mouse and hold it to stretch the
                 shape out from where you clicked. Some shapes can change
                 proportion (e.g., rectangle and oval may be wider than tall,
                 or taller than wide), others cannot (e.g., square and
                 circle).

                 Let go of the mouse when you're done stretching.

                      Normal Shapes Mode

                              Now you can move the mouse around the canvas to
                              rotate the shape.

                              Click the mouse button again and the shape will
                              be drawn in the current color.

                      Simple Shapes Mode
                              If simple shapes are enabled (e.g., with the
                              "simpleshapes" option), the shape will be drawn
                              on the canvas when you let go of the mouse
                              button. (There's no rotation step.)

     ----------------------------------------------------------------------

         "Text" and "Label" Tools

                 Choose a font (from the 'Letters' available on the right)
                 and a color (from the color palette near the bottom). Click
                 on the screen and a cursor will appear. Type text and it
                 will show up on the screen.

                 Press [Enter] or [Return] and the text will be drawn onto
                 the picture and the cursor will move down one line.

                 Alternatively, press [Tab] and the text will be drawn onto
                 the picture, but the cursor will move to the right of the
                 text, rather than down a line, and to the left. (This can be
                 useful to create a line of text with mixed colors, fonts,
                 styles and sizes.)

                 Clicking elsewhere in the picture while the text entry is
                 still active causes the current line of text to move to that
                 location (where you can continue editing it).

                      "Text" versus "Label"

                              The Text tool is the original text-entry tool
                              in Tux Paint. Text entered using this tool
                              can't be modified or moved later, since it
                              becomes part of the drawing. However, because
                              the text becomes part of the picture, it can be
                              drawn over or modified using Magic tool effects
                              (e.g., smudged, tinted, embossed, etc.)

                              When using the Label tool (which was added to
                              Tux Paint in version 0.9.22), the text 'floats'
                              over the image, and the details of the label
                              (the text, the position of the label, the font
                              choice and the color) get stored separately.
                              This allows the label to be repositioned or
                              edited later.

                              The Label tool can be disabled (e.g., by
                              selecting "Disable 'Label' Tool" in Tux Paint
                              Config. or running Tux Paint with the "nolabel"
                              option).

                      International Character Input

                              Tux Paint allows inputting characters in
                              different languages. Most Latin characters
                              (A-Z, ñ, è, etc.) can by entered directly. Some
                              languages require that Tux Paint be switched
                              into an alternate input mode before entering,
                              and some characters must be composed using
                              numerous keypresses.

                              When Tux Paint's locale is set to one of the
                              languages that provide alternate input modes, a
                              key is used to cycle through normal (Latin
                              character) and locale-specific mode or modes.

                              Currently supported locales, the input methods
                              available, and the key to toggle or cycle
                              modes, are listed below. Note: Many fonts do
                              not include all characters for all languages,
                              so sometimes you'll need to change fonts to see
                              the characters you're trying to type.

                                 * Japanese — Romanized Hiragana and
                                   Romanized Katakana — right [Alt] key
                                 * Korean — Hangul 2-Bul — right [Alt] key or
                                   left [Alt] key
                                 * Traditional Chinese — right [Alt] key or
                                   left [Alt] key
                                 * Thai — right [Alt] key

                      On-screen Keyboard

                              An optional on-screen keyboard is available for
                              the Text and Label tools, which can provide a
                              variety of layouts and character composition
                              (e.g., composing "a" and "e" into "æ"). See the
                              "Options" and "Extending Tux Paint"
                              documentation for more information.

     ----------------------------------------------------------------------

         "Fill" Tool

                 The 'Fill' tool 'flood-fills' a contiguous area of your
                 drawing with a color of your choice. Three fill options are
                 offered:
                    * Solid — click once to fill an area with a solid color.
                    * Linear — click and then drag to fill the area with
                      color that fades away (a gradient) towards where you
                      drag the mouse.
                    * Radial — click once to fill an area with a color that
                      fades away (a gradient) radially, centered on where you
                      clicked.

                 Note: Prior to Tux Paint 0.9.24, this was a Magic tool (see
                 below). Note: Prior to Tux Paint 0.9.26, this tool only
                 offered the 'Solid' method of filling.

     ----------------------------------------------------------------------

         "Magic" Tool (Special Effects)

                 The Magic tool is actually a set of special tools. Select
                 one of the 'magic' effects from the selector on the right.
                 Then, depending on the tool, you can either click and drag
                 around the picture, and/or simply click the picture once, to
                 apply the effect.

                 If the tool can be used by clicking and dragging, a
                 'painting' button will be available on the left, below the
                 list of Magic tools on the right side of the screen. If the
                 tool can affect the entire picture at once, an 'entire
                 picture' button will be available on the right.

                 See the instructions for each Magic tool (in the
                 'magic-docs' folder).

     ----------------------------------------------------------------------

         "Eraser" Tool

                 This tool is similar to the Paint Brush. Wherever you click
                 (or click and drag), the picture will be erased. (This may
                 be white, some other color, or to a background picture,
                 depending on the picture.)

                 A number of eraser sizes are available, both round and
                 square.

                 As you move the mouse around, a square outline follows the
                 pointer, showing what part of the picture will be erased to
                 white.

                 As you erase, a 'squeaky clean' eraser wiping sound is
                 played.

     ----------------------------------------------------------------------

  Other Controls

         "Undo" Command

                 Clicking this tool will undo the last drawing action. You
                 can even undo more than once!

                 Note: You can also press [Control] + [Z] on the keyboard to
                 Undo.

     ----------------------------------------------------------------------

         "Redo" Command

                 Clicking this tool will redo the drawing action you just
                 un-did with the 'Undo' button.

                 As long as you don't draw again, you can redo as many times
                 as you had undone!

                 Note: You can also press [Control] + [R] on the keyboard to
                 Redo.

     ----------------------------------------------------------------------

         "New" Command

                 Clicking the 'New' button will start a new drawing. A dialog
                 will appear where you may choose to start a new picture
                 using a solid background color, or using a 'Starter' or
                 'Template' image (see below). You will first be asked
                 whether you really want to do this.

                 Note: You can also press [Control] + [N] on the keyboard to
                 start a new drawing.

                 'Starter' & Template Images

                   'Starters' can behave like a page from a coloring book — a
                   black-and-white outline of a picture, which you can then
                   color in, and the black outline remains intact — or like a
                   3D photograph, where you draw in between a foreground and
                   background layer.

                   'Templates' are similar, but simply provide a background
                   drawing to work off of. Unlike 'Starters', there is no
                   layer that remains in the foreground of anything you draw
                   in the picture.

                   When using the 'Eraser' tool, the original image from the
                   'Starter' or 'Template' will reappear. The 'Flip' and
                   'Mirror' Magic tools affect the orientation of the
                   'Starter' or 'Template', as well.

                   When you load a 'Starter' or 'Template', draw on it, and
                   then click 'Save,' it creates a new picture file — it
                   doesn't overwrite the original, so you can use it again
                   later (by accessing it from the 'New' dialog).

     ----------------------------------------------------------------------

         "Open" Command

                 This shows you a list of all of the pictures you've saved.
                 If there are more than can fit on the screen, use the up and
                 down arrows at the top and bottom of the list to scroll
                 through the list of pictures.

                 Click a picture to select it, and then...

                      * Click the green 'Open' button at the lower left of
                        the list to load the selected picture.

                        (Alternatively, you can double-click a picture's icon
                        to load it.)

                      * Click the brown 'Erase' (trash can) button at the
                        lower right of the list to erase the selected
                        picture. (You will be asked to confirm.)

                        Note: As of version 0.9.22, the picture will be
                        placed in your desktop's trash can, on Linux only.

                      * Click the 'Export' button near the lower right to
                        export the image to your export folder. (e.g.,
                        "~/Pictures/TuxPaint/")

                      * Click the blue 'Slides' (slide projector) button at
                        the lower left to go to slideshow mode. See "Slides",
                        below, for details.

                      * Click the red 'Back' arrow button at the lower right
                        of the list to cancel and return to the picture you
                        were drawing.

                 If choose to open a picture, and your current drawing hasn't
                 been saved, you will be prompted as to whether you want to
                 save it or not. (See "Save," below.)

                 Note: You can also press [Control] + [O] on the keyboard to
                 bring up the 'Open' dialog.

     ----------------------------------------------------------------------

         "Save" Command

                 This saves your current picture.

                 If you haven't saved it before, it will create a new entry
                 in the list of saved images. (i.e., it will create a new
                 file)

                 Note: It won't ask you anything (e.g., for a filename). It
                 will simply save the picture, and play a "camera shutter"
                 sound effect.

                 If you have saved the picture before, or this is a picture
                 you just loaded using the "Open" command, you will first be
                 asked whether you want to save over the old version, or
                 create a new entry (a new file).

                 Note: If either the "saveover" or "saveovernew" options are
                 set, it won't ask before saving over. See the "Options"
                 documentation.

                 Note: You can also press [Control] + [S] on the keyboard to
                 save.

     ----------------------------------------------------------------------

         "Print" Command

                 Click this button and your picture will be printed!

                 On most platforms, you can also hold the [Alt] key (called
                 [Option] on Macs) while clicking the 'Print' button to get a
                 printer dialog. Note that this may not work if you're
                 running Tux Paint in fullscreen mode. See below.

                      Disabling Printing

                              The "noprint" option can be set, which will
                              disable Tux Paint's 'Print' button.

                              See the "Options" documentation.

                      Restricting Printing

                              The "printdelay" option can be set, which will
                              only allow occasional printing — once every so
                              many seconds, as configured by you.

                              For example, with "printdelay=60" in Tux
                              Paint's configuration file, printing can only
                              occur once per minute (60 seconds).

                              See the "Options" documentation.

                      Printing Commands

                              (Linux and Unix only)

                              Tux Paint prints by generating a PostScript
                              representation of the drawing and sending it to
                              an external program. By default, the program
                              is:

                                lpr

                              This command can be changed by setting a
                              "printcommand" option in Tux Paint's
                              configuration file.

                              An alternative print command can be invoked by
                              holding the "[Alt]" key on the keyboard while
                              clicking clicking the 'Print' button, as long
                              as you're not in fullscreen mode, an
                              alternative program is run. By default, the
                              program is KDE's graphical print dialog:

                                kprinter

                              This command can be changed by setting a
                              "altprintcommand" option in Tux Paint's
                              configuration file.

                              See the "Options" documentation.

                      Printer Settings

                              (Windows and macOS)

                              By default, Tux Paint simply prints to the
                              default printer with default settings when the
                              'Print' button is pushed.

                              However, if you hold the [Alt] (or [Option])
                              key on the keyboard while clicking the 'Print'
                              button, as long as you're not in fullscreen
                              mode, your operating system's printer dialog
                              will appear, where you can change the settings.

                              You can have the printer configuration changes
                              stored between Tux Paint sessions by setting
                              the "printcfg" option.

                              If the "printcfg" option is used, printer
                              settings will be loaded from the file
                              "printcfg.cfg" in your personal folder (see
                              below). Any changes will be saved there as
                              well.

                              See the "Options" documentation.

                      Printer Dialog Options

                              By default, Tux Paint only shows the printer
                              dialog (or, on Linux/Unix, runs the
                              "altprintcommand"; e.g., "kprinter" instead of
                              "lpr") if the [Alt] (or [Option]) key is held
                              while clicking the 'Print' button.

                              However, this behavior can be changed. You can
                              have the printer dialog always appear by using
                              "--altprintalways" on the command-line, or
                              "altprint=always" in Tux Paint's configuration
                              file. Conversely, you can prevent the
                              [Alt]/[Option] key from having any effect by
                              using "--altprintnever", or "altprint=never".

                              See the "Options" documentation.

     ----------------------------------------------------------------------

         "Slides" Command (under "Open")

                 The 'Slides' button is available in the 'Open' dialog. It
                 can be used to play a simple animation within Tux Paint, or
                 a slideshow of pictures. It can also export an animated GIF
                 based on the chosen images.

                      Chosing pictures

                              When you enter the 'Slides' section of Tux
                              Paint, it displays a list of your saved files,
                              just like the 'Open' dialog.

                              Click each of the images you wish to display in
                              a slideshow-style presentation, one by one. A
                              digit will appear over each image, letting you
                              know in which order they will be displayed.

                              You can click a selected image to unselect it
                              (take it out of your slideshow). Click it again
                              if you wish to add it to the end of the list.

                      Set playback speed

                              A sliding scale at the lower left of the screen
                              (next to the 'Play' button) can be used to
                              adjust the speed of the slideshow or animated
                              GIF, from slowest to fastest. Choose the
                              leftmost setting to disable automatic
                              advancement during playback within Tux Paint —
                              you will need to press a key or click to go to
                              the next slide (see below).

                              Note: The slowest setting does not
                              automatically advance through the slides. Use
                              it for when you want to step through them
                              manually. (This does not apply to an exported
                              animated GIF.)

                      Playback in Tux Paint

                              To play a slideshow within Tux Paint, click the
                              'Play' button. (Note: If you hadn't selected
                              ANY images, then ALL of your saved images will
                              be played in the slideshow!)

                              During the slideshow, press [Space], [Enter] or
                              [Return], or the [Right arrow] — or click the
                              'Next' button at the lower left — to manually
                              advance to the next slide. Press [Left arrow]
                              to go back to the previous slide.

                              Press [Escape], or click the 'Back' button at
                              the lower right, to exit the slideshow and
                              return to the slideshow image selection screen.

                      Exporting an animated GIF

                              Click the 'GIF Export' button near the lower
                              right to have Tux Paint generate an animated
                              GIF file based on the selected images.

                              Note: At least two images must be selected. (To
                              export a single image, use the 'Export' option
                              from the main 'Open' dialog.) If no images are
                              selected, Tux Paint will NOT attempt to
                              generate a GIF based on all saved images.

                              Pressing [Escape] during the export process
                              will abort the process, and return you to the
                              'Slideshow' dialog.

                 Click 'Back' in the slideshow image selection screen to
                 return to the 'Open' dialog.

     ----------------------------------------------------------------------

         "Quit" Command

                 Clicking the 'Quit' button, closing the Tux Paint window, or
                 pushing the [Escape] key will quit Tux Paint.

                 You will first be prompted as to whether you really want to
                 quit.

                 If you choose to quit, and you haven't saved the current
                 picture, you will first be asked if wish to save it. If it's
                 not a new image, you will then be asked if you want to save
                 over the old version, or create a new entry. (See "Save"
                 above.)

                 Note: If the image is saved, it will be reloaded
                 automatically the next time you run Tux Paint -- unless the
                 "startblank" option is set.

                 Note: The 'Quit' button within Tux Paint, and quitting via
                 the [Escape] key, may be disabled, via the "noquit" option.

                 In that case, the "window close" button on Tux Paint's title
                 bar (if not in fullscreen mode) or the [Alt] + [F4] key
                 sequence may be used to quit.

                 If neither of those are possible, the key sequence of
                 [Shift] + [Control] + [Escape] may be used to quit.

                 See the "Options" documentation.

     ----------------------------------------------------------------------

         Sound Muting

                 There is no on-screen control button at this time, but by
                 using the [Alt] + [S] keyboard sequence, sound effects can
                 be disabled and re-enabled (muted and unmuted) while the
                 program is running.

                 Note that if sounds are completely disabled via the
                 "nosound" option, the [Alt] + [S] key combination has no
                 effect. (i.e., it cannot be used to turn on sounds when the
                 parent/teacher wants them disabled.)

     ----------------------------------------------------------------------

                     Loading Other Pictures into Tux Paint

     Tux Paint's 'Open' dialog only displays pictures you created with Tux
     Paint. So what do you do if you want to load some other drawinng or even
     a photograph into Tux Paint, so you can edit or draw on it?

     You can simply convert the picture to the format Tux Paint uses — PNG
     (Portable Network Graphic) — and place it in Tux Paint's "saved"
     directory/folder. Here is where to find it (by default):

     Windows 10, 8, 7, Vista
             Inside the user's "AppData" folder, e.g.:
             "C:\Users\username\AppData\Roaming\TuxPaint\saved\".

     Windows 2000, XP
             Inside the user's "Application Data" folder, e.g.: "C:\Documents
             and Settings\username\Application Data\TuxPaint\saved\".

     macOS
             Inside the user's "Library" folder, e.g.:
             "/Users/username/Library/Application Support/Tux Paint/saved/".

     Linux/Unix
             Inside a hidden ".tuxpaint" directory, in the user's home
             directory ("$HOME"), e.g. "/home/username/.tuxpaint/saved/".

     Note: It is also from this folder that you can copy or open pictures
     drawn in Tux Paint using other applications, though the 'Export' option
     from Tux Paint's 'Open' dialog can be used to copy them to a location
     that's easier and safer to access.

Using the import script, "tuxpaint-import"

       Linux and Unix users can use the "tuxpaint-import" shell script which
       gets installed when you install Tux Paint. It uses some NetPBM tools
       to convert the image ("anytopnm"), resize it so that it will fit in
       Tux Paint's canvas ("pnmscale"), and convert it to a PNG ("pnmtopng").

       It also uses the "date" command to get the current time and date,
       which is the file-naming convention Tux Paint uses for saved files.
       (Remember, you are never asked for a 'filename' when you go to save or
       open pictures!)

       To use this script, simply run it from a command-line prompt, and
       provide it the name(s) of the file(s) you wish to convert.

       They will be converted and placed in your Tux Paint "saved" directory.
       (Note: If you're doing this for a different user (e.g., your child)
       you'll need to make sure to run the command under their account.)

       Example:

         $ tuxpaint-import grandma.jpg
         grandma.jpg -> /home/username/.tuxpaint/saved/20211231012359.png
         jpegtopnm: WRITING A PPM FILE

       The first line ("tuxpaint-import grandma.jpg") is the command to run.
       The following two lines are output from the program while it's
       working.

       Now you can load Tux Paint, and a version of that original picture
       will be available under the 'Open' dialog. Just double-click its icon!

Importing Pictures Manually

       Windows, macOS, and Haiku users who wish to import arbitrary images
       into Tux Paint must do so via a manual process.

       Load a graphics program that is capable of both loading your picture
       and saving a PNG format file. (See the documentation file "PNG.html"
       for a list of suggested software, and other references.)

       When Tux Paint loads an image that's not the same size as its drawing
       canvas, it scales (and sometimes smears the edges of) the image so
       that it fits within the canvas.

       To avoid having the image stretched or smeared, you can resize it to
       Tux Paint's canvas size. This size depends on the size of the Tux
       Paint window, or resolution at which Tux Paint is run, if in
       fullscreen. (Note: The default resolution is 800x600.) See
       "Calculating Image Dimensions", below.

       Save the picture in PNG format. It is highly recommended that you name
       the filename using the current date and time, since that's the
       convention Tux Paint uses:

         YYYYMMDDhhmmss.png

         * YYYY = Year
         * MM = Month (two digits, "01"-"12")
         * DD = Day of month (two digits, "01"-"31")
         * HH = Hour (two digits, in 24-hour format, "00"-"23")
         * mm = Minute (two digits, "00"-"59")
         * ss = Seconds (two digits, "00"-"59")

       Example: "20210731110500.png", for July 31, 2021 at 11:05am.

       Place this PNG file in your Tux Paint "saved" directory/folder. (See
       above.)

  Calculating Image Dimensions

         This part of the documentation needs to be rewritten, since the new
         "buttonsize" option was added. For now, try drawing and saving an
         image within Tux Paint, then determine what size (pixel width and
         height) it came out to, and try to match that when scaling the
         picture(s) you're importing into Tux Paint.

     ----------------------------------------------------------------------

                                Further Reading

     Other documentation included with Tux Paint (found in the "docs"
     folder/directory) includes:
       * Documentation sur l'outil 'Magie' ("magic-docs")
         Documentation for each of the currently-installed 'Magic' tools.
       * AUTHORS.txt
         List of authors and contributors.
       * CHANGES.txt
         Summary of what has changed between releases of Tux Paint.
       * COPYING.txt
         Tux Paint's software license, the GNU General Public License (GPL)
       * INSTALL.html
         Instructions for compiling and installing Tux Paint, when
         applicable.
       * EXTENDING.html
         Detailed instructions on extending Tux Paint: creating brushes,
         stamps, starters, and templates; adding fonts; and creating new
         on-screen keyboard layouts and input methods.
       * OPTIONS.html
         Detailed instructions on command-line and configuration-file
         options, for those who don't want to use the Tux Paint Config. tool
         to manage Tux Paint's configuration.
       * PNG.html
         Notes on creating PNG format bitmapped (raster) images for use in
         Tux Paint.
       * SVG.html
         Notes on creating SVG format vector images for use in Tux Paint.
       * SIGNALS.html
         Information about the POSIX signals that Tux Paint responds to.

     ----------------------------------------------------------------------

                                How to Get Help

     If you need help, there are numerous ways to interact with Tux Paint
     developers and other users.

       * Report bugs or request new features via the project's bug-tracking
         system
       * Participate in the various project mailing lists
       * Chat with developers and other users over IRC
       * Contact the developers directly

     To learn more, visit the "Contact" page of the official Tux Paint
     website: http://tuxpaint.org/contact/

     ----------------------------------------------------------------------

                               How to Participate

     Tux Paint is a volunteer-driven project, and we're happy to accept your
     help in a variety of ways.

       * Translate Tux Paint to another language
       * Improve existing translations
       * Create artwork (stamps, starters, templates, brushes)
       * Add or improve features or magic tools
       * Create classroom curriculum
       * Promote or help support others using Tux Paint

     To learn more, visit the "Help Us" page of the official Tux Paint
     website: http://tuxpaint.org/help/
