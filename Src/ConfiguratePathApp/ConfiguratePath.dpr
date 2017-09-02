program ConfiguratePath;

uses
  Vcl.Forms,
  UnitMain in 'UnitMain.pas' {FormConfigurePath};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TFormConfigurePath, FormConfigurePath);
  Application.Run;
end.
