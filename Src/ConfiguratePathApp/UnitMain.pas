unit UnitMain;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.CheckLst, Vcl.ExtCtrls,
  Vcl.Buttons, Vcl.ButtonGroup, IniFiles, ShellApi, System.Win.Registry,
  Vcl.ComCtrls;

const
  PLUGIN_PATH = 'MOTIONBUILDER_PLUGIN_PATH';
  SCRIPT_PATH = 'MOTIONBUILDER_PYTHON_STARTUP';

type
  TFormConfigurePath = class(TForm)
    GridPanel1: TGridPanel;
    Label1: TLabel;
    listPluginPath: TCheckListBox;
    Label2: TLabel;
    listScriptPath: TCheckListBox;
    buttonRefresh: TBitBtn;
    ButtonGroup1: TButtonGroup;
    ButtonGroup2: TButtonGroup;
    buttonApply: TButton;
    StatusBar1: TStatusBar;
    procedure buttonRefreshClick(Sender: TObject);
    procedure ButtonGroup1Items0Click(Sender: TObject);
    procedure ButtonGroup1Items1Click(Sender: TObject);
    procedure ButtonGroup1Items2Click(Sender: TObject);
    procedure listPluginPathDblClick(Sender: TObject);
    procedure ButtonGroup2Items0Click(Sender: TObject);
    procedure ButtonGroup2Items1Click(Sender: TObject);
    procedure buttonApplyClick(Sender: TObject);
    procedure ButtonGroup2Items2Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private declarations }
    procedure RefreshPath;
    procedure ApplyPath;
    procedure StoreConfig;
    procedure RetrieveConfig;
    function AskForAFolder : String;
  public
    { Public declarations }
  end;

var
  FormConfigurePath: TFormConfigurePath;

implementation

{$R *.dfm}

function GetEnvVariable(Name: string; User: Boolean = True): string;
var
  Str: array[0..255] of char;
begin
  with TRegistry.Create do
  try
    if User then
    begin
      RootKey := HKEY_CURRENT_USER;
      OpenKey('Environment', False);
      //OpenKeyReadOnly('Environment', False);
    end
    else
    begin
      RootKey := HKEY_LOCAL_MACHINE;
      OpenKey('SYSTEM\CurrentControlSet\Control\Session ' +
        'Manager\Environment', False);
      //OpenKeyReadOnly('SYSTEM\CurrentControlSet\Control\Session ' +
      //  'Manager\Environment', False);
    end;
    Result := ReadString(Name);
    ExpandEnvironmentStrings(PChar(Result), Str, 255);
    Result := Str;
  finally
    Free;
  end;
end;

procedure SetEnvVariable(Name, Value: string; User: Boolean = True);
var
  rv: DWORD;
begin
  with TRegistry.Create do
  try
    if User then
    begin
      RootKey := HKEY_CURRENT_USER;
      OpenKey('Environment', False);
      WriteString(Name, Value);
    end
    else
    begin
      RootKey := HKEY_LOCAL_MACHINE;
      OpenKey('SYSTEM\CurrentControlSet\Control\Session ' +
        'Manager\Environment', False);
    end;
    WriteString(Name, Value);
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, LParam
      (PChar('Environment')), SMTO_ABORTIFHUNG, 5000, rv);
  finally
    Free;
  end;
end;

procedure TFormConfigurePath.ApplyPath;
var
  s : string;
  cmd : string;
  I: Integer;
  result : Boolean;
begin
  s := '';

  for I := 0 to listPluginPath.Count-1 do
  begin
    if listPluginPath.Checked[I] then
        s := s + listPluginPath.Items[I] + ';';
  end;

  if (s.Length > 0) and (s.Chars[s.Length-1] = ';') then
    s := s.Remove( s.Length-1 );

  result := SetEnvironmentVariable(PWideChar(PLUGIN_PATH), PWideChar(s) );
  SetEnvVariable(PLUGIN_PATH, s);

  //
  s := '';

  for I := 0 to listScriptPath.Count-1 do
  begin
    if listScriptPath.Checked[I] then
      s := s + listScriptPath.Items[I] + ';';
  end;

  if (s.Length > 0) and (s.Chars[s.Length-1] = ';') then
    s := s.Remove( s.Length-1 );

  result := SetEnvironmentVariable(PWideChar(SCRIPT_PATH), PWideChar(s) );
  SetEnvVariable(SCRIPT_PATH, s);
end;

function TFormConfigurePath.AskForAFolder: String;
begin
  Result := '';
  with TFileOpenDialog.Create(nil) do
  try
    Options := [fdoPickFolders];
    if Execute then
        Result := FileName;
  finally
    Free;
  end;
end;

procedure TFormConfigurePath.buttonRefreshClick(Sender: TObject);
begin
  RefreshPath;
end;

procedure TFormConfigurePath.FormClose(Sender: TObject;
  var Action: TCloseAction);
begin
  StoreConfig;
end;

procedure TFormConfigurePath.FormCreate(Sender: TObject);
begin
  RetrieveConfig;
end;

procedure TFormConfigurePath.buttonApplyClick(Sender: TObject);
begin
  ApplyPath;
end;

procedure TFormConfigurePath.ButtonGroup1Items0Click(Sender: TObject);
var
  s: string;
begin
  s := AskForAFolder();

  if (s <> '') and (listPluginPath.Items.IndexOf(s) = -1) then
    listPluginPath.Items.Add(s);
end;

procedure TFormConfigurePath.ButtonGroup1Items1Click(Sender: TObject);
begin
  listPluginPath.DeleteSelected;
end;

procedure TFormConfigurePath.ButtonGroup1Items2Click(Sender: TObject);
begin
  listPluginPath.Clear;
end;

procedure TFormConfigurePath.ButtonGroup2Items0Click(Sender: TObject);
var
  s : string;
begin
  s := AskForAFolder();

  if (s <> '') and (listScriptPath.Items.IndexOf(s) = -1) then
    listScriptPath.Items.Add(s);
end;

procedure TFormConfigurePath.ButtonGroup2Items1Click(Sender: TObject);
begin
  listScriptPath.DeleteSelected;
end;

procedure TFormConfigurePath.ButtonGroup2Items2Click(Sender: TObject);
begin
  listScriptPath.Clear;
end;

procedure TFormConfigurePath.listPluginPathDblClick(Sender: TObject);
var
  s: string;
begin
  if listPluginPath.ItemIndex >= 0 then
  begin
    s := AskForAFolder();
    if s <> '' then
        listPluginPath.Items[listPluginPath.ItemIndex] := s;
  end;
end;

procedure TFormConfigurePath.RefreshPath;
var
  value: String;
  strList: TStringList;
  strItem : String;
  index : integer;
begin
  strList := TStringList.Create;

  try
    strList.Delimiter := ';';

    value := GetEnvVariable(PLUGIN_PATH);
    strList.DelimitedText := value;

    listPluginPath.CheckAll(TCheckBoxState.cbUnchecked);
    listScriptPath.CheckAll(TCheckBoxState.cbUnchecked);

    for strItem in strList do
    begin

      if strItem = '' then
        continue;

      index := listPluginPath.Items.IndexOf(strItem);

      if index >= 0 then
      begin
        listPluginPath.Checked[index] := True;
      end
      else begin
        index := listPluginPath.Items.Add(strItem);
        listPluginPath.Checked[index] := True;
      end;
    end;

    //
    //

    value := GetEnvVariable(SCRIPT_PATH);
    strList.DelimitedText := value;

    for strItem in strList do
    begin
      if strItem = '' then
        continue;

      index := listScriptPath.Items.IndexOf(strItem);

      if index >= 0 then
      begin
        listScriptPath.Checked[index] := True;
      end
      else begin
        index := listScriptPath.Items.Add(strItem);
        listScriptPath.Checked[index] := True;
      end;
    end;

  finally
    strList.Free;
  end;

end;

procedure TFormConfigurePath.RetrieveConfig;
var
  i: integer;
  value: integer;
  path: string;
begin
  path := ExtractFilePath(ParamStr(0));

  if FileExists(path + '/pluginPath.txt') then
    listPluginPath.Items.LoadFromFile(path + '/pluginPath.txt');

  if FileExists(path + '/scriptPath.txt')  then
    listScriptPath.Items.LoadFromFile(path + '/scriptPath.txt');

  if FileExists(path + '/checkStates.dat') then
    with TFileStream.Create(path + '/checkStates.dat', fmOpenRead) do
    try
      for i := 0 to listPluginPath.Count-1 do
      begin
        Read(value, sizeof(integer));
        listPluginPath.Checked[i] := (value > 0);
      end;

      //
      for i := 0 to listScriptPath.Count-1 do
      begin
        Read(value, sizeof(integer));
        listScriptPath.Checked[i] := (value > 0);
      end;
    finally
      Free;
    end;
end;

procedure TFormConfigurePath.StoreConfig;
var
  i: Integer;
  valueZero, valueOne: Integer;
  path : string;
begin
  path := ExtractFilePath(ParamStr(0));

  listPluginPath.Items.SaveToFile(path + '/pluginPath.txt');
  listScriptPath.Items.SaveToFile(path + '/scriptPath.txt');

  with TFileStream.Create(path + '/checkStates.dat', fmCreate or fmOpenWrite) do
  try
    valueZero := 0;
    valueOne := 1;

    for i := 0 to listPluginPath.Count-1 do
      if listPluginPath.Checked[i] then
        Write(valueOne, sizeof(integer))
      else
        Write(valueZero, sizeof(integer));

    //
    for i := 0 to listScriptPath.Count-1 do
      if listScriptPath.Checked[i] then
        Write(valueOne, sizeof(integer))
      else
        Write(valueZero, sizeof(integer));
  finally
    Free;
  end;
end;

end.
