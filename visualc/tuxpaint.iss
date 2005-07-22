[Setup]
AppName=Tux Paint
AppVerName=Tux Paint 0.9.15-cvs
AppPublisher=New Breed Software
AppPublisherURL=http://www.newbreedsoftware.com/tuxpaint/
AppSupportURL=http://www.newbreedsoftware.com/tuxpaint/
AppUpdatesURL=http://www.newbreedsoftware.com/tuxpaint/
DefaultDirName={pf}\TuxPaint
DefaultGroupName=Tux Paint
LicenseFile=.\bdist\docs\COPYING.txt
OutputDir=.\
OutputBaseFilename=tuxpaint-0.9.15-cvs-win32-installer
SetupIconFile=.\bdist\data\images\icon-win32.ico
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
Source: ".\bdist\tuxpaint.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\libintl-3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\libpng12.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\SDL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\SDL_image.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\SDL_mixer.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\SDL_ttf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\bdist\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\bdist\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\bdist\locale\*"; DestDir: "{app}\locale"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\bdist\tuxpaint-config.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[INI]
Filename: "{app}\tuxpaint.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.newbreedsoftware.com/tuxpaint/"

[Icons]
Name: "{code:MyGroupDir}\Configure Tux Paint"; Filename: "{app}\tuxpaint-config.exe"; Comment: "Tux Paint Config"
Name: "{code:MyGroupDir}\Tux Paint (Full Screen)"; Filename: "{app}\tuxpaint.exe"; Parameters: "--fullscreen"; Comment: "Start Tux Paint in Fullscreen mode"
Name: "{code:MyGroupDir}\Tux Paint (Windowed)"; Filename: "{app}\tuxpaint.exe"; Parameters: "--windowed"; Comment: "Start Tux Paint in a Window"
Name: "{code:MyGroupDir}\Readme"; Filename: "{app}\docs\html\README.html"; Comment: "View ReadMe"
Name: "{code:MyGroupDir}\Licence"; Filename: "{app}\docs\copying.txt"; Comment: "View License"
Name: "{code:MyGroupDir}\{cm:ProgramOnTheWeb,Tux Paint}"; Filename: "{app}\tuxpaint.url"; Comment: "{cm:ProgramOnTheWeb,Tux Paint}"
Name: "{code:MyGroupDir}\{cm:UninstallProgram,Tux Paint}"; Filename: "{uninstallexe}"; IconFilename: "{app}\data\images\tuxpaint-installer.ico"; Comment: "Remove Tux Paint"
Name: "{code:MyDesktopDir}\Tux Paint"; Filename: "{app}\tuxpaint.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\docs\html\README.html"; Description: "View the README file"; Flags: postinstall shellexec
Filename: "{app}\tuxpaint-config.exe"; Description: "{cm:LaunchProgram,Tux Paint Config}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\tuxpaint.url"

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

function MyAppDir(): String;
var
  Path: String;
begin
  Path := ExpandConstant('{pf}')
  if UsingWinNT() and (Restricted() or CurrentUserOnly()) then
  begin
    Path := GetShellFolderByCSIDL(CSIDL_PROFILE, True);
    if Path = '' then
      Path := RemoveBackslashUnlessRoot(ExtractFilePath(ExpandConstant('{userdocs}')));
    Path := Path + '\Programs'
  end
  Result := Path + '\TuxPaint'
end;

function MyGroupDir(Default: String): String;
var
  Path: String;
begin
  if Restricted() or CurrentUserOnly() then
    Path := ExpandConstant('{userprograms}')
  else
  begin
    Path := ExpandConstant('{commonprograms}')
  end
  Result := Path + '\Tux Paint'
end;

function MyDesktopDir(Default: String): String;
var
  Path: String;
begin
  if Restricted() or CurrentUserOnly() then
    Path := ExpandConstant('{userdesktop}')
  else
  begin
    Path := ExpandConstant('{commondesktop}')
  end
  Result := Path
end;

procedure CreateTheWizardPages;
var
  Page: TWizardPage;
  Enabled, AllUsers: Boolean;
begin
  Page := CreateCustomPage(wpLicense, 'Choose Installation Type', 'Who do you want to be able to use this program?');
  Enabled := NotRestricted() and UsingWinNT();
  AllUsers := NotRestricted() and UsingWinNT();
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
  CheckListBox2.AddRadioButton('All Users', '', 0, AllUsers, Enabled, nil);
  CheckListBox2.AddRadioButton('Current User Only', '', 0, not AllUsers, True, nil);
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


