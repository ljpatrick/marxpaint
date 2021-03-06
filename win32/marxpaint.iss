;
; This script needs the InnoSetup PreProcessor (ISPP) to compile correctly. 
; I downloaded a combined 'QuickStart Pack' from here:
; http://www.jrsoftware.org/isdl.php#qsp
;
; The version string is extracted from the executable.
;
; As of 2011.06.15, this integrates OpenCandy advertising module.
; However, by default, using "marxpaint.iss", the standard Marx Paint installer
; will be built.  Use "marxpaint-opencandy.iss" (and you will need the
; product key and secret, stored in a "marxpaint-opencandy-secrets.iss" file;
; Bill Kendrick has access to these), which sets up a #define and then
; #include's this file, to produce an installer with OpenCandy built-in.
; -bjk

; Should we change this to Marx4Kids? -bjk 2011.06.15
#define PublisherName "New Breed Software"
#define PublisherURL  "{code:MyPublisherURL}"

#define AppName       "Marx Paint"
#define AppDirName    "MarxPaint"
#define AppPrefix     "marxpaint"
#define AppRegKey     AppDirName
#define AppRegValue   "Install_Dir"
#define AppRegVersion "Version"

#define AppGroupName  AppName
#define AppExe        AppPrefix+".exe"
#define AppConfigName AppName+" Config"
#define AppConfigExe  AppPrefix+"-config.exe"
#define AppReadme     "{code:MyReadme}"
#define AppLicence    "{code:MyLicence}"

#define BdistDir      ".\bdist"
#define AppVersion    GetStringFileInfo(BdistDir+"\"+AppExe, "FileVersion")


#ifdef OpenCandy
#define OC_STR_MY_PRODUCT_NAME "Marx Paint"
;// Note: Please change the registry path to match your company name
;#define OC_STR_REGISTRY_PATH "Software\Marx Paint\OpenCandy"
#define OC_OCSETUPHLP_FILE_PATH ".\OCSetupHlp.dll"
#include 'marxpaint-opencandy-secrets.iss'
;#if OC_STR_MY_PRODUCT_NAME == "Open Candy Sample"
;	#pragma warning "Do not forget to change the product name from 'Open Candy Sample' to your company's product name before releasing this installer."	
;#endif
;#if OC_STR_KEY == "1401d0bd8048e1f0f4628dbec1a73092"
;	#pragma warning "Do not forget to change the test key '1401d0bd8048e1f0f4628dbec1a73092' to your company's product key before releasing this installer."
;#endif
;#if OC_STR_SECRET == "4564bdaf826bbe2115718d1643ecc19e"
;	#pragma warning "Do not forget to change the test secret '4564bdaf826bbe2115718d1643ecc19e' to your company's product secret before releasing this installer."
;#endif
;#if OC_STR_REGISTRY_PATH == "Software\Your Company\OpenCandy"
;	#pragma warning "Do not forget to change the test registry path 'Your Company' to your companies name before releasing this installer."
;#endif
;#if Pos(LowerCase("Software\OpenCandy"),LowerCase(OC_STR_REGISTRY_PATH)) != 0
;	#pragma warning "ERROR, your registry path has OpenCandy before your company name. Please place your company name before OpenCandy. eg Software\Your Company\OpenCandy"
;#endif
#endif

[Setup]
AppName={#AppName}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#PublisherName}
AppPublisherURL={#PublisherURL}
AppSupportURL={#PublisherURL}
AppUpdatesURL={#PublisherURL}
DefaultDirName={pf}\{#AppDirName}
DefaultGroupName={#AppGroupName}
OutputDir=.\
#ifdef OpenCandy
  LicenseFile={#BdistDir}\docs\COPYING-OC.txt
  OutputBaseFilename={#AppPrefix}-{#AppVersion}-win32-installer-opencandy
#else
  LicenseFile={#BdistDir}\docs\COPYING\COPYING-en.txt
  OutputBaseFilename={#AppPrefix}-{#AppVersion}-win32-installer
#endif
SetupIconFile={#BdistDir}\data\images\marxpaint-installer.ico
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin

[Languages]
Name: "afr"; MessagesFile: "compiler:Languages\Afrikaans.isl"
Name: "alb"; MessagesFile: "compiler:Languages\Albanian.isl"
Name: "ara"; MessagesFile: "compiler:Languages\Arabic.isl"
Name: "arm"; MessagesFile: "compiler:Languages\Armenian.islu"
Name: "ast"; MessagesFile: "compiler:Languages\Asturian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-es-COPIADO.txt"
Name: "baq"; MessagesFile: "compiler:Languages\Basque.isl"
Name: "bel"; MessagesFile: "compiler:Languages\Belarusian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-be.txt"
Name: "ben"; MessagesFile: "compiler:Languages\Bengali.islu"
Name: "bos"; MessagesFile: "compiler:Languages\Bosnian.isl"
Name: "bra"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING_pt_BR.txt"
Name: "bul"; MessagesFile: "compiler:Languages\Bulgarian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-bg.txt"
Name: "cat"; MessagesFile: "compiler:Languages\Catalan.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-ca.txt"
Name: "chs"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-zh_cn.txt"
Name: "cht"; MessagesFile: "compiler:Languages\ChineseTraditional.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-zh_tw_GNU_GPL_Chinese.txt"
Name: "cnr"; MessagesFile: "compiler:Languages\Montenegrian.isl"
Name: "cos"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "cze"; MessagesFile: "compiler:Languages\Czech.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-cs.txt"
Name: "dan"; MessagesFile: "compiler:Languages\Danish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-da.txt"
Name: "dut"; MessagesFile: "compiler:Languages\Dutch.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-nl.txt"
Name: "enb"; MessagesFile: "compiler:Languages\EnglishBritish.isl"
Name: "eng"; MessagesFile: "compiler:Default.isl"
Name: "epo"; MessagesFile: "compiler:Languages\Esperanto.isl"
Name: "esp"; MessagesFile: "compiler:Languages\Spanish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-es-COPIADO.txt"
Name: "est"; MessagesFile: "compiler:Languages\Estonian.isl"
Name: "fas"; MessagesFile: "compiler:Languages\Farsi.isl"
Name: "fin"; MessagesFile: "compiler:Languages\Finnish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-fi.txt"
Name: "fre"; MessagesFile: "compiler:Languages\French.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-fr-COPIER.txt"
Name: "gal"; MessagesFile: "compiler:Languages\Galician.isl"
Name: "geo"; MessagesFile: "compiler:Languages\Georgian.islu"
Name: "ger"; MessagesFile: "compiler:Languages\German.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-de-KOPIE.txt"
Name: "gla"; MessagesFile: "compiler:Languages\ScottishGaelic.isl"
Name: "gre"; MessagesFile: "compiler:Languages\Greek.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-el.txt"
Name: "heb"; MessagesFile: "compiler:Languages\Hebrew.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-he.txt"
Name: "hin"; MessagesFile: "compiler:Languages\Hindi.islu"
Name: "hun"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "ice"; MessagesFile: "compiler:Languages\Icelandic.isl"
Name: "ind"; MessagesFile: "compiler:Languages\Indonesian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-id.txt"
Name: "ita"; MessagesFile: "compiler:Languages\Italian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-it-COPIATURA.txt"
Name: "jpn"; MessagesFile: "compiler:Languages\Japanese.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-ja.txt"
Name: "kaz"; MessagesFile: "compiler:Languages\Kazakh.islu"
Name: "kor"; MessagesFile: "compiler:Languages\Korean.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-ko.txt"
Name: "kur"; MessagesFile: "compiler:Languages\Kurdish.isl"
Name: "lav"; MessagesFile: "compiler:Languages\Latvian.isl"
Name: "lij"; MessagesFile: "compiler:Languages\Ligurian.isl"
Name: "lit"; MessagesFile: "compiler:Languages\Lithuanian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-lt.txt"
Name: "ltz"; MessagesFile: "compiler:Languages\Luxemburgish.isl"
Name: "mac"; MessagesFile: "compiler:Languages\Macedonian.isl"
Name: "mar"; MessagesFile: "compiler:Languages\Marathi.islu"
Name: "may"; MessagesFile: "compiler:Languages\Malaysian.isl"
Name: "mon"; MessagesFile: "compiler:Languages\Mongolian.isl"
Name: "nep"; MessagesFile: "compiler:Languages\Nepali.islu"
Name: "nno"; MessagesFile: "compiler:Languages\NorwegianNynorsk.isl"
Name: "nor"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "occ"; MessagesFile: "compiler:Languages\Occitan.isl"
Name: "pol"; MessagesFile: "compiler:Languages\Polish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-pl-LICENCJA-GNU.txt"
Name: "por"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "rum"; MessagesFile: "compiler:Languages\Romanian.isl"
Name: "rus"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-ro.txt"
Name: "scc"; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-sr.txt"
Name: "scl"; MessagesFile: "compiler:Languages\SerbianLatin.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-sr.txt"
Name: "scr"; MessagesFile: "compiler:Languages\Croatian.isl"
Name: "sin"; MessagesFile: "compiler:Languages\Sinhala.islu"
Name: "slo"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slv"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "swe"; MessagesFile: "compiler:Languages\Swedish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-sv.txt"
Name: "tai"; MessagesFile: "compiler:Languages\Thai.isl"
Name: "tat"; MessagesFile: "compiler:Languages\Tatar.isl"
Name: "tur"; MessagesFile: "compiler:Languages\Turkish.isl"; LicenseFile: "{#BdistDir}\docs\COPYING\COPYING-tr.txt"
Name: "uig"; MessagesFile: "compiler:Languages\Uyghur.islu"
Name: "ukr"; MessagesFile: "compiler:Languages\Ukrainian.isl"        
Name: "uzb"; MessagesFile: "compiler:Languages\Uzbek.isl"
Name: "vie"; MessagesFile: "compiler:Languages\Vietnamese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "{#BdistDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\data\*"; DestDir: "{app}\data"; Excludes: "CVS"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\docs\*"; DestDir: "{app}\docs"; Excludes: "CVS,Makefile,*~"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\etc\*"; DestDir: "{app}\etc"; Flags: skipifsourcedoesntexist ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\lib\*"; DestDir: "{app}\lib"; Flags: skipifsourcedoesntexist ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\im\*"; DestDir: "{app}\im"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\locale\*"; DestDir: "{app}\locale"; Excludes: "CVS"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\..\libdocs\*"; DestDir: "{app}\docs\libdocs"; Excludes: "CVS,Makefile,*~"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

#ifdef OpenCandy
	Source: "{#OC_OCSETUPHLP_FILE_PATH}"; Flags: dontcopy ignoreversion;
#endif



[INI]
Filename: "{code:MyGroupDir}\{groupname}\{cm:ProgramOnTheWeb,{#AppName}}.url"; Section: "InternetShortcut"; Key: "URL"; String: "{#PublisherURL}"

[Icons]
Name: "{code:MyGroupDir}\{groupname}\Configure {#AppName}"; Filename: "{app}\{#AppConfigExe}"; Comment: "{#AppConfigName}"
Name: "{code:MyGroupDir}\{groupname}\{#AppName} (Full Screen)"; Filename: "{app}\{#AppExe}"; Parameters: "--fullscreen native"; Comment: "Start {#AppName} in Fullscreen mode"
Name: "{code:MyGroupDir}\{groupname}\{#AppName} (Windowed)"; Filename: "{app}\{#AppExe}"; Parameters: "--windowed"; Comment: "Start {#AppName} in a Window"
Name: "{code:MyGroupDir}\{groupname}\Readme"; Filename: "{app}\{#AppReadme}"; Comment: "View ReadMe"
Name: "{code:MyGroupDir}\{groupname}\Licence"; Filename: "{app}\{#AppLicence}"; Comment: "View License"
Name: "{code:MyGroupDir}\{groupname}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"; IconFilename: "{app}\data\images\marxpaint-installer.ico"; Comment: "Remove {#AppName}"
Name: "{code:MyDesktopDir}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
Root: HKLM; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegValue}"; ValueType: string; ValueData: "{app}"; Check: AllUsers;
Root: HKCU; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegValue}"; ValueType: string; ValueData: "{app}"; Check: ThisUserOnly;
Root: HKLM; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegVersion}"; ValueType: string; ValueData: "{#AppVersion}"; Check: AllUsers;
Root: HKCU; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegVersion}"; ValueType: string; ValueData: "{#AppVersion}"; Check: ThisUserOnly;

[Run]
Filename: "{app}\{#AppReadme}"; Description: "View the README file"; Flags: postinstall shellexec skipifsilent
Filename: "{app}\{#AppConfigExe}"; Description: "{cm:LaunchProgram,{#AppConfigName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{code:MyGroupDir}\{groupname}\{cm:ProgramOnTheWeb,{#AppName}}.url"

[code]
#ifdef OpenCandy
#include 'OCSetupHlp.iss'
#endif

const
  CSIDL_PROFILE = $0028;
  CSIDL_COMMON_PROGRAMS = $0017;
  CSIDL_COMMON_DESKTOPDIRECTORY = $0019;
var
  InstallTypePageID: Integer;
  CheckListBox2: TNewCheckListBox;
#ifdef OpenCandy
 	OCtszInstallerLanguage: OCTString;
#endif
function Restricted(): Boolean;
begin
  Result := not (IsAdminLoggedOn() or IsPowerUserLoggedOn())
end;

function NotRestricted(): Boolean;
begin
  Result := not Restricted()
end;

function Is9xME(): Boolean;
begin
  Result := not UsingWinNT()
end;

function CurrentUserOnly(): Boolean;
begin
  Result := CheckListBox2.Checked[2]
end;

function ThisUserOnly(): Boolean;
begin
  Result := (Restricted() or CurrentUserOnly()) and UsingWinNT()
end;

function AllUsers(): Boolean;
begin
  Result := not ThisUserOnly()
end;

function MyAppDir(): String;
var
  Path: String;
begin
  Path := ExpandConstant('{reg:HKLM\SOFTWARE\{#AppRegKey},{#AppRegValue}|{pf}\{#AppDirName}}');
  if ThisUserOnly() then
  begin
    Path := ExpandConstant('{reg:HKCU\SOFTWARE\{#AppRegKey},{#AppRegValue}|__MissingKey__}');
    if Path = '__MissingKey__' then
    begin
      Path := GetShellFolderByCSIDL(CSIDL_PROFILE, True);
      if Path = '' then
        Path := RemoveBackslashUnlessRoot(ExtractFilePath(ExpandConstant('{userdocs}')));
      Path := Path + '\Programs\{#AppDirName}'
    end;
  end;
  Result := Path
end;

function MyGroupDir(Default: String): String;
var
  Path: String;
begin
  if ThisUserOnly() then
    Path := ExpandConstant('{userprograms}')
  else
    Path := ExpandConstant('{commonprograms}');
  Result := Path;
end;

function MyDesktopDir(Default: String): String;
var
  Path: String;
begin
  if ThisUserOnly() then
    Path := ExpandConstant('{userdesktop}')
  else
    Path := ExpandConstant('{commondesktop}');
  Result := Path;
end;

procedure CreateTheWizardPages;
var
  Page: TWizardPage;
  Enabled, InstallAllUsers: Boolean;
begin
  Page := CreateCustomPage(wpLicense, 'Choose Installation Type', 'Who do you want to be able to use this program?');
  InstallTypePageID := Page.ID;
  Enabled := NotRestricted();
  InstallAllUsers := NotRestricted();
  CheckListBox2 := TNewCheckListBox.Create(Page);
  CheckListBox2.Width := Page.SurfaceWidth;
  CheckListBox2.Height := ScaleY(97);
  CheckListBox2.BorderStyle := bsNone;
  CheckListBox2.ParentColor := True;
  CheckListBox2.MinItemHeight := WizardForm.TasksList.MinItemHeight;
  CheckListBox2.ShowLines := False;
  CheckListBox2.WantTabs := True;
  CheckListBox2.Parent := Page.Surface;
  CheckListBox2.AddGroup('Installation Type:', '', 0, nil);
  CheckListBox2.AddRadioButton('All Users', '', 0, InstallAllUsers, Enabled, nil);
  CheckListBox2.AddRadioButton('Current User Only', '', 0, not InstallAllUsers, True, nil);
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectDir then
  begin
    WizardForm.DirEdit.Text := MyAppDir();
  end;
  begin
   #ifdef OpenCandy
	 OpenCandyCurPageChanged(CurPageID);
	 #endif
  end;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := (PageID = InstallTypePageID) and Is9xME();
  begin
   #ifdef OpenCandy
	 Result := OpenCandyShouldSkipPage(PageID);
   #endif
  end;
end;

#ifdef OpenCandy
function NextButtonClick(CurPageID: Integer): Boolean;
begin
	Result := OpenCandyNextButtonClick(CurPageID);
end;
#endif

function BackButtonClick(CurPageID: Integer): Boolean;
begin
	Result := true; // Allow action by default

  #ifdef OpenCandy
	OpenCandyBackButtonClick(CurPageID);
	#endif
end;

procedure DeinitializeSetup();
begin
  #ifdef OpenCandy
	OpenCandyDeinitializeSetup();
	#endif
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  #ifdef OpenCandy
	OpenCandyCurStepChanged(CurStep);
	#endif
end;

function Lang2Gettext(TwoLetter: Boolean): String;
var
  lang, lc: String;
begin
  lang := ActiveLanguage();

  if lang = 'afr' then
    lc := 'af'
  else
  if lang = 'alb' then
    lc := 'sq'
  else
  if lang = 'ara' then
    lc := 'ar'
  else
  if lang = 'arm' then
    lc := 'hy'
  else
  if lang = 'ast' then
    if TwoLetter = true then
      lc := 'es'
    else
      lc := 'ast'
  else
  if lang = 'baq' then
    lc := 'eu'
  else
  if lang = 'ben' then
    lc := 'bn'
  else
  if lang = 'bel' then
    lc := 'be'
  else
  if lang = 'bos' then
    lc := 'bs'
  else
  if lang = 'bra' then
    if TwoLetter = true then
      lc := 'pt'
    else
      lc := 'pt_br'
  else
  if lang = 'bul' then
    lc := 'bg'
  else
  if lang = 'cat' then
    lc := 'ca'
  else
  if lang = 'chs' then
    if TwoLetter = true then
      lc := 'zh'
    else
      lc := 'zh_cn'
  else
  if lang = 'cht' then
    if TwoLetter = true then
      lc := 'zh'
    else
      lc := 'zh_tw'
  else
  if lang = 'cnr' then
    if TwoLetter = true then
      lc := 'sr'
    else
      lc := 'sr_latin'
  else
  if lang = 'cos' then
    lc := 'co'
  else
  if lang = 'cze' then
    lc := 'cs'
  else
  if lang = 'dan' then
    lc := 'da'
  else
  if lang = 'dut' then
    lc := 'nl'
  else
  if lang = 'enb' then
    if TwoLetter = true then
      lc := 'en'
    else
      lc := 'en_gb'
  else
  if lang = 'epo' then
    lc := 'eo'
  else
  if lang = 'esp' then
    lc := 'es'
  else
  if lang = 'est' then
    lc := 'et'
  else
  if lang = 'fas' then
    lc := 'fa'
  else
  if lang = 'fin' then
    lc := 'fi'
  else
  if lang = 'fre' then
    lc := 'fr'
  else
  if lang = 'gal' then
    lc := 'gl'
  else
  if lang = 'geo' then
    lc := 'ka'
  else
  if lang = 'ger' then
    lc := 'de'
  else
  if lang = 'gla' then
    lc := 'gd'
  else
  if lang = 'gre' then
    lc := 'el'
  else
  if lang = 'heb' then
    lc := 'he'
  else
  if lang = 'hin' then
    lc := 'hi'
  else
  if lang = 'hun' then
    lc := 'hu'
  else
  if lang = 'ice' then
    lc := 'is'
  else
  if lang = 'ind' then
    lc := 'id'
  else
  if lang = 'ita' then
    lc := 'it'
  else
  if lang = 'jpn' then
    lc := 'ja'
  else
  if lang = 'kaz' then
    lc := 'kk'
  else
  if lang = 'kor' then
    lc := 'ko'
  else
  if lang = 'kur' then
    lc := 'ku'
  else
  if lang = 'lav' then
    lc := 'lv'
  else
  if lang = 'lit' then
    lc := 'lt'
  else
  if lang = 'ltz' then
    lc := 'lb'
  else
  if lang = 'mac' then
    lc := 'mk'
  else
  if lang = 'mar' then
    lc := 'mr'
  else
  if lang = 'may' then
    lc := 'ms'
  else
  if lang = 'mon' then
    lc := 'mn'
  else
  if lang = 'nep' then
    lc := 'ne'
  else
  if lang = 'nno' then
    lc := 'nn'
  else
  if lang = 'nor' then
    lc := 'nn'
  else
  if lang = 'occ' then
    lc := 'oc'
  else
  if lang = 'pol' then
    lc := 'pl'
  else
  if lang = 'por' then
    lc := 'pt'
  else
  if lang = 'rum' then
    lc := 'ro'
  else
  if lang = 'rus' then
    lc := 'ru'
  else
  if lang = 'scc' then
    lc := 'sr'
  else
  if lang = 'scl' then
    if TwoLetter = true then
      lc := 'sr'
    else
      lc := 'sr_latin'
  else
  if lang = 'scr' then
    lc := 'hr'
  else
  if lang = 'sin' then
    lc := 'si'
  else
  if lang = 'slo' then
    lc := 'sk'
  else
  if lang = 'slv' then
    lc := 'sl'
  else
  if lang = 'swe' then
    lc := 'sv'
  else
  if lang = 'tai' then
    lc := 'th'
  else
  if lang = 'tat' then
    lc := 'tt'
  else
  if lang = 'tur' then
    lc := 'tr'
  else
  if lang = 'uig' then
    lc := 'ug'
  else
  if lang = 'ukr' then
    lc := 'uk'
  else
  if lang = 'urd' then
    lc := 'ur'
  else
  if lang = 'uzb' then
    lc := 'uz'
  else
  if lang = 'vie' then
    lc := 'vi'
  else
    lc := 'en';
  Result := lc
end;

procedure InitializeWizard();
begin
  begin
    CreateTheWizardPages;  
  end
#ifdef OpenCandy
  OCtszInstallerLanguage := Lang2Gettext(true);
	OpenCandyAsyncInit('{#OC_STR_MY_PRODUCT_NAME}', '{#OC_STR_KEY}', '{#OC_STR_SECRET}', OCtszInstallerLanguage, {#OC_INIT_MODE_NORMAL});
  #endif
end;

function MyReadme(Default: String): String;
var
  lang, readme: String;
begin
  lang := Lang2Gettext(false);

  if lang = 'es' then
    readme := 'es_ES.UTF-8\html\README.html'
  else
  if lang = 'fr' then
    readme := 'fr_FR.UTF8\html\README.html'
  else
  if lang = 'gl' then
    readme := 'gl_GL.UTF-8\html\README.html'
  else
  if lang = 'it' then
    readme := 'it\html\README.html'
  else
  if lang = 'ja' then
    readme := 'ja_JP.UTF-8\html\README.html'
  else
  if lang = 'nl' then
    readme := 'nl\html\README.html'
  else
  if lang = 'ru' then
    readme := 'ru\html\README.html'
  else
  if lang = 'zh_cn' then
    readme := 'zh_cn\html\README.html'
  else
  if lang = 'zh_tw' then
    readme := 'zh_tw\html\README.html'
  else
    readme := 'en\html\README.html';

  Result := 'docs\'+readme
end;

function MyLicence(Default: String): String;
var
  lang, licence: String;
begin
  lang := Lang2Gettext(false);

  if lang = 'pt_br' then
    licence := 'COPYING_pt_BR.txt'
  else
  if lang = 'be' then
    licence := 'COPYING-be.txt'
  else
  if lang = 'bg' then
    licence := 'COPYING-bg.txt'
  else
  if lang = 'ca' then
    licence := 'COPYING-ca.txt'
  else
  if lang = 'cs' then
    licence := 'COPYING-cs.txt'
  else
  if lang = 'cy' then
    licence := 'COPYING-cy.txt'
  else
  if lang = 'da' then
    licence := 'COPYING-da.txt'
  else
  if lang = 'de' then
    licence := 'COPYING-de-KOPIE.txt'
  else
  if lang = 'el' then
    licence := 'COPYING-el.txt'
  else
  if lang = 'es' then
    licence := 'COPYING-es-COPIADO.txt'
  else
  if lang = 'fi' then
    licence := 'COPYING-fi.txt'
  else
  if lang = 'fr' then
    licence := 'COPYING-fr-COPIER.txt'
  else
  if lang = 'he' then
    licence := 'COPYING-he.txt'
  else
  if lang = 'id' then
    licence := 'COPYING-id.txt'
  else
  if lang = 'it' then
    licence := 'COPYING-it-COPIATURA.txt'
  else
  if lang = 'ja' then
    licence := 'COPYING-ja.txt'
  else
  if lang = 'ko' then
    licence := 'COPYING-ko.txt'
  else
  if lang = 'lt' then
    licence := 'COPYING-lt.txt'
  else
  if lang = 'nl' then
    licence := 'COPYING-nl.txt'
  else
  if lang = 'pl' then
    licence := 'COPYING-pl-LICENCJA-GNU.txt'
  else
  if lang = 'ro' then
    licence := 'COPYING-ro.txt'
  else
  if lang = 'sr' then
    licence := 'COPYING-sr.txt'
  else
  if lang = 'sv' then
    licence := 'COPYING-sv.txt'
  else
  if lang = 'tr' then
    licence := 'COPYING-tr.txt'
  else
  if lang = 'zh_cn' then
    licence := 'COPYING-zh_cn.txt'
  else
  if lang = 'zh_tw' then
    licence := 'COPYING-zh_tw_GNU_GPL_Chinese.txt'
  else
    licence := 'COPYING-en.txt';

  Result := 'docs\COPYING\'+licence
end;

function MyPublisherURL(Default: String): String;
var
  lang: String;
begin
  lang := Lang2Gettext(false);

  if lang = 'eng' then
    lang := 'en_US'
  else
  if lang = 'enb' then
    lang := 'en_GB'
  else
  if lang = 'cat' then
    lang := 'ca_ES'
  else
  if lang = 'dan' then
    lang := 'da_DK'
  else
  if lang = 'esp' then
    lang := 'es_ES'
  else
  if lang = 'fin' then
    lang := 'fi_FI'
  else
  if lang = 'fre' then
    lang := 'fr_FR'
  else
  if lang = 'geo' then
    lang := 'ka_GE'
  else
  if lang = 'gre' then
    lang := 'el_GR'
  else
  if lang = 'ita' then
    lang := 'it_IT'
  else
  if lang = 'jpn' then
    lang := 'ja_JP'
  else
  if lang = 'mon' then
    lang := 'mn_MN'
  else
  if lang = 'dut' then
    lang := 'nl_NL'
  else
  if lang = 'nno' then
    lang := 'nn_NO'
  else
  if lang = 'pol' then
    lang := 'pl_PL'
  else
  if lang = 'rus' then
    lang := 'ru_RU';

  Result := 'http://www.marxpaint.org/?lang='+lang
end;

#expr SaveToFile(AddBackslash(SourcePath) + 'Preprocessed.iss')

