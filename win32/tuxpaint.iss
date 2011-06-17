;
; This script needs the InnoSetup PreProcessor (ISPP) to compile correctly. 
; I downloaded a combined 'QuickStart Pack' from here:
; http://www.jrsoftware.org/isdl.php#qsp
;
; The version string is extracted from the executable.
;
; As of 2011.06.15, this integrates OpenCandy advertising module.
; However, by default, using "tuxpaint.iss", the standard Tux Paint installer
; will be built.  Use "tuxpaint-opencandy.iss" (and you will need the
; product key and secret, stored in a "tuxpaint-opencandy-secrets.iss" file;
; Bill Kendrick has access to these), which sets up a #define and then
; #include's this file, to produce an installer with OpenCandy built-in.
; -bjk

; Should we change this to Tux4Kids? -bjk 2011.06.15
#define PublisherName "New Breed Software"
#define PublisherURL  "{code:MyPublisherURL}"

#define AppName       "Tux Paint"
#define AppDirName    "TuxPaint"
#define AppPrefix     "tuxpaint"
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
#define OC_STR_MY_PRODUCT_NAME "Tux Paint"
;// Note: Please change the registry path to match your company name
;#define OC_STR_REGISTRY_PATH "Software\Tux Paint\OpenCandy"
#define OC_OCSETUPHLP_FILE_PATH ".\OCSetupHlp.dll"
#include 'tuxpaint-opencandy-secrets.iss'
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
;FIXME - It would be good if we showed the localized license -bjk 2011.06.15
#ifdef OpenCandy
  LicenseFile={#BdistDir}\docs\COPYING-OC.txt
  OutputBaseFilename={#AppPrefix}-{#AppVersion}-win32-installer-opencandy
#else
  LicenseFile={#BdistDir}\docs\COPYING.txt
  OutputBaseFilename={#AppPrefix}-{#AppVersion}-win32-installer
#endif
SetupIconFile={#BdistDir}\data\images\tuxpaint-installer.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin

[Languages]
Name: "eng"; MessagesFile: "compiler:Default.isl"
Name: "bra"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "cat"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "cze"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "dan"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dut"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "fre"; MessagesFile: "compiler:Languages\French.isl"
Name: "ger"; MessagesFile: "compiler:Languages\German.isl"
Name: "hun"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "ita"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "nor"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "pol"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "por"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "rus"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "slo"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "esp"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "jpn"; MessagesFile: "compiler:Languages\Japanese.isl"

; Additional, Candidate official translations
Name: "chs"; MessagesFile: "compiler:Languages\ChineseSimp-12-5.1.11.isl"

; Additional, Unofficial translations
Name: "gal"; MessagesFile: "compiler:Languages\Galician-2-5.1.11.isl"
Name: "cht"; MessagesFile: "compiler:Languages\ChineseTrad-2-5.1.11.isl"

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
Name: "{code:MyGroupDir}\{groupname}\{#AppName} (Full Screen)"; Filename: "{app}\{#AppExe}"; Parameters: "--fullscreen"; Comment: "Start {#AppName} in Fullscreen mode"
Name: "{code:MyGroupDir}\{groupname}\{#AppName} (Windowed)"; Filename: "{app}\{#AppExe}"; Parameters: "--windowed"; Comment: "Start {#AppName} in a Window"
Name: "{code:MyGroupDir}\{groupname}\Readme"; Filename: "{app}\{#AppReadme}"; Comment: "View ReadMe"
Name: "{code:MyGroupDir}\{groupname}\Licence"; Filename: "{app}\{#AppLicence}"; Comment: "View License"
Name: "{code:MyGroupDir}\{groupname}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"; IconFilename: "{app}\data\images\tuxpaint-installer.ico"; Comment: "Remove {#AppName}"
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

  if lang = 'bra' then
    if TwoLetter = true then
      lc := 'pt'
    else
      lc := 'pt_br'
  else
  if lang = 'cat' then
    lc := 'ca'
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
  if lang = 'fre' then
    lc := 'fr'
  else
  if lang = 'ger' then
    lc := 'de'
  else
  if lang = 'hun' then
    lc := 'hu'
  else
  if lang = 'ita' then
    lc := 'it'
  else
  if lang = 'nor' then
    lc := 'nn'
  else
  if lang = 'pol' then
    lc := 'pl'
  else
  if lang = 'por' then
    lc := 'pt'
  else
  if lang = 'rus' then
    lc := 'ru'
  else
  if lang = 'slo' then
    lc := 'sk'
  else
  if lang = 'chs' then
    if TwoLetter = true then
      lc := 'zh'
    else
      lc := 'zh_cn'
  else
  if lang = 'esp' then
    lc := 'es'
  else
  if lang = 'gal' then
    lc := 'gl'
  else
  if lang = 'jpn' then
    lc := 'ja'
  else
  if lang = 'cht' then
    if TwoLetter = true then
      lc := 'zh'
    else
      lc := 'zh_tw'
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

  if lang = 'nl' then
    readme := 'nl\html\README.html'
  else
  if lang = 'fr' then
    readme := 'fr\html\README1.html'
  else
  if lang = 'it' then
    readme := 'it\html\README.html'
  else
  if lang = 'es' then
    readme := 'es\html\LEEME.html'
  else
  if lang = 'gl' then
    readme := 'gl\html\README.html'
  else
  if lang = 'zh_tw' then
    readme := 'zh_tw\html\README.html'
  else
    readme := 'html\README.html';

  Result := 'docs\'+readme
end;

function MyLicence(Default: String): String;
var
  lang, licence: String;
begin
  lang := Lang2Gettext(false);

  if lang = 'pt_br' then
    licence := 'pt_br\COPYING_pt_BR.txt'
  else
  if lang = 'ca' then
    licence := 'ca\COPYING.txt'
  else
  if lang = 'cs' then
    licence := 'cs\COPYING.txt'
  else
  if lang = 'da' then
    licence := 'da\COPYING.txt'
  else
  if lang = 'nl' then
    licence := 'nl\COPYING_nl.txt'
  else
  if lang = 'fr' then
    licence := 'fr\COPIER.txt'
  else
  if lang = 'de' then
    licence := 'de\KOPIE.txt'
  else
  if lang = 'it' then
    licence := 'it\COPIATURA.txt'
  else
  if lang = 'pl' then
    licence := 'pl\LICENCJA-GNU.txt'
  else
  if lang = 'es' then
    licence := 'es\COPIADO.txt'
  else
    licence := 'COPYING.txt';

  Result := 'docs\'+licence
end;

function MyPublisherURL(Default: String): String;
var
  lang: String;
begin
  lang := Lang2Gettext(false);

  if lang = 'chs' then
    lang := 'zh-cn'
  else
  if lang = 'cht' then
    lang := 'zh-tw';

  Result := 'http://www.tuxpaint.org/?lang='+lang
end;

#expr SaveToFile(AddBackslash(SourcePath) + 'Preprocessed.iss')

