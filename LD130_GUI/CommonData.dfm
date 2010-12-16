object DataModuleCommon: TDataModuleCommon
  OldCreateOrder = False
  Left = 65534
  Top = 134
  Height = 291
  Width = 440
  object RzRegIniFile: TRzRegIniFile
    Path = 'Software\Boreal\LD130'
    PathType = ptRegistry
    RegAccess = [keySetValue, keyCreateSubKey, keyRead, keyWrite, keyAllAccess]
    Left = 32
    Top = 8
  end
end
