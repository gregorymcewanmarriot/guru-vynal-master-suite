#ifndef MyAppVersion
  #define MyAppVersion "0.1.2"
#endif

#ifndef SourceRoot
  #define SourceRoot "..\package"
#endif

#define MyAppName "GURU Vynil Master Suite"
#define MyPublisher "GURU"
#define MyStandaloneExe "GURU Vynil Master Suite.exe"
#define MyVST3Name "GURU Vynil Master Suite.vst3"

[Setup]
AppId={{C5EEAF2F-60C1-44AB-93B7-7E6191DAB9FA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyPublisher}
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyPublisher}
VersionInfoDescription={#MyAppName} Windows x64 installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}
DefaultDirName={autopf}\GURU\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=..\dist
OutputBaseFilename=GURU-Vynil-Master-Suite-Setup-{#MyAppVersion}-Windows-x64
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
DisableProgramGroupPage=yes
CloseApplications=no
RestartApplications=no
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyStandaloneExe}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#SourceRoot}\VST3\{#MyVST3Name}\*"; DestDir: "{commoncf64}\VST3\{#MyVST3Name}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceRoot}\Standalone\{#MyStandaloneExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\CHANGELOG.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyStandaloneExe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyStandaloneExe}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Run]
Filename: "{app}\{#MyStandaloneExe}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#MyVST3Name}"
