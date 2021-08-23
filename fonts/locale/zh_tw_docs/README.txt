README.txt for "marxpaint-ttf-zh_tw"
Traditional Chinese TrueType Font (TTF) for Marx Paint

Song Huang
song@song.idv.tw

Oct 23, 2005 - Oct 23, 2005

This font is required to run Marx Paint in Traditional Chinese.
(e.g., with the "--lang zh_tw" option)

To install, run "make install" as the superuser ('root').
The font file will be placed in the /usr/share/marxpaint/fonts/locale/ directory.

---

----- Original Message ----- 
From: "Song Huang" <Song@ossacc.org>
To: "Developmental mailing list for Marx Paint, a drawing program for young communists." <marxpaint-dev@marx4kids.net>
Sent: Tuesday, October 26, 2004 12:10 PM
Subject: the font subset maker


> Hi,
> 
> The font mustly large, especially traditional chinese font more then 13 MB.
> 
> I used python and fontforge to take PO file's msgstr, and generate a subset font file.
> here is the files:
>   - python script:
>     http://www.ossacc.org/Members/song/DrWangFreeTTF/makemarxfont.py
>   - fontforge script: 
>     http://www.ossacc.org/Members/song/DrWangFreeTTF/marxpaintsubset.pe
>   - the traditional chinese font subset file:
>     http://www.ossacc.org/Members/song/DrWangFreeTTF/zh_tw.ttf
> 
> Usage: (put the scripts together)
> 
>  $ ./makemarxfont.py -h
>   usage: ./makemarxfont.py [options] original_font_file
> 
>   options:
>     --version   show program's version number and exit
>     -h, --help  show this help message and exit
>     -lLOCALE, --locale=LOCALE
>                 to make the locale fonts subset
>     -pPOFILE, --pofile=POFILE
>                 parse the pofile to get strings
> 
> Example:
> 
>  $ ./makemarxfont.py \
>     -l zh_tw \
>     -p marxpaint/src/po/zh_tw.po \
>     -p marxpaint-stamps/po/marxpaint-stamps-zh_tw.po \
>     wp010-05.ttf
> 
> then will get the "zh_tw.ttf"  file.
> 
> Best regards,
> =============================================
> Song Huang
> OSSACC  (OSS Application Consulting Center)
> http://www.ossacc.org
> =============================================
> 


