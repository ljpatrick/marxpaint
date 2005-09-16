;
; This script needs the InnoSetup PreProcessor (ISPP) to compile correctly. 
; I downloaded a combined 'QuickStart Pack' from here:
; http://www.jrsoftware.org/isdl.php#qsp
;
; The version string is extracted from the executable.
;

#define PublisherName "New Breed Software"
#define PublisherURL  "http://www.newbreedsoftware.com/tuxpaint/"

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
#define AppReadme     "docs\html\README.html"
#define AppLicence    "docs\COPYING.txt"

#define BdistDir      ".\bdist"
#define AppVersion    GetStringFileInfo(BdistDir+"\"+AppExe, "FileVersion")

[Setup]
AppName={#AppName}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#PublisherName}
AppPublisherURL={#PublisherURL}
AppSupportURL={#PublisherURL}
AppUpdatesURL={#PublisherURL}
DefaultDirName={pf}\{#AppDirName}
DefaultGroupName={#AppGroupName}
LicenseFile={#BdistDir}\{#AppLicence}
OutputDir=.\
OutputBaseFilename={#AppPrefix}-{#AppVersion}-win32-installer
SetupIconFile={#BdistDir}\data\images\icon-win32.ico
Compression=lzma
SolidCompression=yes

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

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#BdistDir}\tuxpaint.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\libiconv-2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\libintl-3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\libpng12.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\SDL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\SDL_image.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\SDL_mixer.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\SDL_ttf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BdistDir}\data\*"; DestDir: "{app}\data"; Excludes: "CVS"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\docs\*"; DestDir: "{app}\docs"; Excludes: "CVS"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\locale\*"; DestDir: "{app}\locale"; Excludes: "CVS"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BdistDir}\tuxpaint-config.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[INI]
Filename: "{app}\{#AppName}.url"; Section: "InternetShortcut"; Key: "URL"; String: "{#PublisherURL}"

[Icons]
Name: "{code:MyGroupDir}\{#AppGroupName}\Configure {#AppName}"; Filename: "{app}\{#AppConfigExe}"; Comment: "{#AppConfigName}"
Name: "{code:MyGroupDir}\{#AppGroupName}\{#AppName} (Full Screen)"; Filename: "{app}\{#AppExe}"; Parameters: "--fullscreen"; Comment: "Start {#AppName} in Fullscreen mode"
Name: "{code:MyGroupDir}\{#AppGroupName}\{#AppName} (Windowed)"; Filename: "{app}\{#AppExe}"; Parameters: "--windowed"; Comment: "Start {#AppName} in a Window"
Name: "{code:MyGroupDir}\{#AppGroupName}\Readme"; Filename: "{app}\{#AppReadme}"; Comment: "View ReadMe"
Name: "{code:MyGroupDir}\{#AppGroupName}\Licence"; Filename: "{app}\{#AppLicence}"; Comment: "View License"
Name: "{code:MyGroupDir}\{#AppGroupName}\{cm:ProgramOnTheWeb,{#AppName}}"; Filename: "{app}\{#AppName}.url"; Comment: "{cm:ProgramOnTheWeb,{#AppName}}"
Name: "{code:MyGroupDir}\{#AppGroupName}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"; IconFilename: "{app}\data\images\tuxpaint-installer.ico"; Comment: "Remove {#AppName}"
Name: "{code:MyDesktopDir}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
Root: HKLM; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegValue}"; ValueType: string; ValueData: "{app}"; Check: AllUsers;
Root: HKCU; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegValue}"; ValueType: string; ValueData: "{app}"; Check: ThisUserOnly;
Root: HKLM; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegVersion}"; ValueType: string; ValueData: "{#AppVersion}"; Check: AllUsers;
Root: HKCU; Subkey: "SOFTWARE\{#AppRegKey}"; Flags: uninsdeletekey; ValueName: "{#AppRegVersion}"; ValueType: string; ValueData: "{#AppVersion}"; Check: ThisUserOnly;

[Run]
Filename: "{app}\{#AppReadme}"; Description: "View the README file"; Flags: postinstall shellexec
Filename: "{app}\{#AppConfigExe}"; Description: "{cm:LaunchProgram,{#AppConfigName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\{#AppName}.url"

[code]
const
  CSIDL_PROFILE = $0028;
  CSIDL_COMMON_PROGRAMS = $0017;
  CSIDL_COMMON_DESKTOPDIRECTORY = $0019;
var
  CheckListBox2: TNewCheckListBox;

function Restricted(): Boolean;
begin
  if IsAdminLoggedOn() or IsPowerUserLoggedOn() then
    Result := false
  else
    Result := true
end;

function NotRestricted(): Boolean;
begin
  Result := not Restricted()
end;

function CurrentUserOnly(): Boolean;
begin
  Result := CheckListBox2.Checked[2]
end;

function ThisUserOnly(): Boolean;
begin
  Result := Restricted() or CurrentUserOnly()
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
    end
  end
  Result := Path
end;

function MyGroupDir(Default: String): String;
var
  Path: String;
begin
  if ThisUserOnly() then
    Path := ExpandConstant('{userprograms}')
  else
    Path := ExpandConstant('{commonprograms}')
  Result := Path
end;

function MyDesktopDir(Default: String): String;
var
  Path: String;
begin
  if ThisUserOnly() then
    Path := ExpandConstant('{userdesktop}')
  else
    Path := ExpandConstant('{commondesktop}')
  Result := Path
end;

procedure CreateTheWizardPages;
var
  Page: TWizardPage;
  Enabled, InstallAllUsers: Boolean;
begin
  Page := CreateCustomPage(wpLicense, 'Choose Installation Type', 'Who do you want to be able to use this program?');
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
  end
end;

procedure InitializeWizard();
begin
  CreateTheWizardPages;
end;

#expr SaveToFile(AddBackslash(SourcePath) + "Preprocessed.iss")

