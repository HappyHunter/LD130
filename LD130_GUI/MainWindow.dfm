object FormMainWindow: TFormMainWindow
  Left = 292
  Top = 288
  Width = 800
  Height = 600
  Caption = 'LD130 Control'
  Color = clBtnFace
  Constraints.MaxHeight = 600
  Constraints.MaxWidth = 800
  Constraints.MinHeight = 600
  Constraints.MinWidth = 800
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object RzPanel1: TRzPanel
    Left = 0
    Top = 0
    Width = 784
    Height = 39
    Align = alTop
    BorderOuter = fsFlatRounded
    TabOrder = 0
    object RzLabel3: TRzLabel
      Left = 17
      Top = 12
      Width = 46
      Height = 13
      Caption = 'Com Port:'
    end
    object cbComPort: TRzComboBox
      Left = 72
      Top = 8
      Width = 75
      Height = 21
      AllowEdit = False
      Ctl3D = False
      FrameVisible = True
      ItemHeight = 13
      ParentCtl3D = False
      TabOrder = 0
      OnCloseUp = cbComPortCloseUp
      OnDropDown = cbComPortDropDown
      Items.Strings = (
        'No COM'
        'COM 1'
        'COM 2'
        'COM 3'
        'COM 4'
        'COM 5'
        'COM 6'
        'COM 7'
        'COM 8'
        'COM 9'
        'COM 10'
        'COM 11'
        'COM 12'
        'COM 13'
        'COM 14'
        'COM 15'
        'COM 16'
        'COM 17'
        'COM 18'
        'COM 19'
        'COM 20'
        'COM 21'
        'COM 22'
        'COM 23'
        'COM 24'
        'COM 25'
        'COM 26'
        'COM 27'
        'COM 28'
        'COM 29'
        'COM 30')
      Values.Strings = (
        '0'
        '1'
        '2'
        '3'
        '4'
        '5'
        '6'
        '7'
        '8'
        '9'
        '10'
        '11'
        '12'
        '13'
        '14'
        '15'
        '16'
        '17'
        '18'
        '19'
        '20'
        '21'
        '22'
        '23'
        '24'
        '25'
        '26'
        '27'
        '28'
        '29'
        '30')
    end
    object RzPanelVersion: TRzPanel
      Left = 244
      Top = 8
      Width = 149
      Height = 20
      BorderOuter = fsFlat
      Caption = 'GUI Version:'
      TabOrder = 1
    end
    object lbLog: TRzListBox
      Left = 398
      Top = 2
      Width = 384
      Height = 35
      Align = alRight
      FrameVisible = True
      ItemHeight = 13
      TabOrder = 2
    end
    object btnComConfig: TRzBitBtn
      Left = 200
      Top = 3
      Width = 37
      Height = 33
      FrameColor = clDefault
      Caption = '...'
      TabOrder = 3
      OnClick = btnComConfigClick
    end
    object rzRefresh: TRzBitBtn
      Left = 155
      Top = 3
      Width = 37
      Height = 33
      Hint = 'Refresh list of available COM ports'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 4
      OnClick = rzRefreshClick
      Glyph.Data = {
        F6060000424DF606000000000000360000002800000018000000180000000100
        180000000000C0060000EB0A0000EB0A00000000000000000000C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0BCBCBCA6A6A68F8F8F
        7C7C7C7776767777777777778686869B9B9BB4B4B4C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0BDBDBD8787874D
        4D4D4A4848655F608376789284869385878D8082756B6D555152444444696969
        AAAAAAC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0878787333232716B6BB1A2A4ADAFAD89A49E7BA19978A0997A9E968EA29DB1
        A9AA97898B4846464E4E4EB8B8B8C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C07F7D7D93898AC2B8B9769F97619E9292C0B5B3D6CCB7D9
        D0A9CFC578AEA34B8E8188A29CB8A9AA726C6DB0B0B0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0B8B7B8A79E9FBEB8B74A8A7E55A28E9EE3C8
        9AF4D198F6D39FF6D6A4F5D6ADF0D482C9B0217F6B68948BB9ACADAAA5A6C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0BFBFBFA6A0A1BCB2B33A807222
        8D6C4CCF9D5BEEBCAEFDE4DCFFF470FBD13DFBC13AF4B899F0D07DC5AA00684D
        69938AB4A6A8B5B3B4C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0B5B3B3BAAC
        AD6A918904755628B37F46D9ACE6FDF8FFFFFFFFFFFF6AF7CD11FAB540F3BFFB
        FFFFEDFAF32C9C7500644D9AA2A0B2A8A9C0C0BFC0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0B3ACADAFA9A918725D259C711CB589D8F5EFFFFFFFFFFFFFD1BDC342CD
        A519F6B8D5FAF0FFFFFFFFFFFFCAEBDD037754417C71BCAEAFBBB9BAC0C0C0C0
        C0C0C0C0C0C0C0C0BFBEBEBBB0B28394900A78591BA67651C0A6FFFFFFFEFAFB
        70717104503C08C191ADFFEDFFFFFFFFFFFFFFFFFFFFFFFF9AD1C0096854AEA9
        A8BBB7B8C0C0C0C0C0C0C0C0C0C0C0C0BFBDBEC2B4B562847C16876322AD829A
        DACCFFFFFFCECFCF178E7138E9BE44E0BA738480919090FFFFFFFCFBFB585858
        455B530C71599BA09EC1BABBC0C0C0C0C0C0C0C0C0C0C0C0C1C0C0C6B9BA4E79
        6E54A78FB7E8D9E4F5F2FFFFFFE5F0EEACEDDE6CD4BD41C6A925A18691C9BDFF
        FFFFE6E7E8068062168E62086C54969D9BC7C0C1C0C0C0C0C0C0C0C0C0C0C0C0
        C3C2C2CCC0C2597E732E7965F9EAEEFFFFFFFFFFFFFFFFFFE8D8DB5FA89A60CB
        B59BDFD1F9FFFEFFFFFFC5D0CE51BCA03BAE84086952A3A4A2CCC7C8C0C0C0C0
        C0C0C0C0C0C0C0C0C1C1C1D5CDCE7E8782147B5F99B0A6F9F5F6FFFFFFF6F0F2
        84A49D73CCBAE5F5F2FFFFFFFFFFFFFCF7F88DB1AA8FDBC56BB79E116351C0B6
        B6CCCACBC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0D8D6D7B2A5A51A675471BAA2CB
        CECCFFFBFCB9C2C090D1C29ADCCDF7FFFDFFFFFFF6F2F3B5B9B893CFC1BAEAD8
        3B957E446E64DCD1D2C6C5C6C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0CDCCCCDBD3
        D45F6C6623876DB9DDCFCBCACAACD5CCB1E6DAB0E3D8D6DBDBCECBCBB2BDBBAA
        D6CBC9F2E498CEBD0B5F4CACA6A4E2DEDFC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0BBB9B9E0DCDCC7B8B933574C33947AB7DFD0C9F0E3BAE7DBB5E5DAAED2
        C9AFD6CCBCE9DDCFF3E6A3D5C50E6B57717A75EDE5E6C1BDBEC0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0BAB3B0EEECEDB8A9A93051461D7A6277BFA9
        AFE0CEBFE9DAC2EDDEBEEADAA6D9C85EAA960B5D4A636F69F4EBECC4BEBFB2AE
        AFC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0B9B6B5C3BBB8F7F6F6CF
        C2C26669641A5042176751287C652B8069237660125C4A2C54489B9693FFFAFB
        BCB9B88E8888C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0BAB6B5C9C1BFF3F1F1F6F2F2D0C2C2A097968383807F817D8D8C89BCB1B0F6
        EBECFCFAFA8684835D5857BEBEBEC0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0BDBCD0C9C9E5E0E1F0EEEEF8F6F7FBF8F8FAF7
        F7F7F4F5E0DFDFA09F9E484644494746BBBBBBC0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C3C2C2D2CECFCBC7C7
        B8B3B4A6A3A393908F7C78785B58575855558A8A89C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C1C0C0BEBDBDB5B4B4ADABABA9A8A8B3B3B2C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0}
      Layout = blGlyphRight
    end
  end
  object StatusBar: TRzStatusBar
    Left = 0
    Top = 543
    Width = 784
    Height = 19
    BorderInner = fsNone
    BorderOuter = fsNone
    BorderSides = [sdLeft, sdTop, sdRight, sdBottom]
    BorderWidth = 0
    TabOrder = 1
    object RzStatusClock: TRzClockStatus
      Left = 721
      Top = 0
      Width = 63
      Height = 19
      Align = alRight
      Format = 'hh:nn:ss'
    end
    object RzFieldFirmwareVersion: TRzFieldStatus
      Left = 510
      Top = 0
      Width = 111
      Height = 19
      Align = alRight
      FieldLabel = 'Firmware:'
    end
    object RzStatusOperation: TRzStatusPane
      Left = 0
      Top = 0
      Width = 510
      Height = 19
      Align = alClient
    end
    object RzFieldActiveBank: TRzFieldStatus
      Left = 621
      Top = 0
      Height = 19
      Align = alRight
      FieldLabel = 'Active Bank:'
    end
  end
  object RzPanel3: TRzPanel
    Left = 0
    Top = 39
    Width = 784
    Height = 504
    Align = alClient
    BorderOuter = fsNone
    TabOrder = 2
    object RzPageControlParameters: TRzPageControl
      Left = 0
      Top = 0
      Width = 784
      Height = 504
      ActivePage = TabSheet1
      Align = alClient
      BoldCurrentTab = True
      UseColoredTabs = True
      ShowCardFrame = False
      TabIndex = 0
      TabOrder = 0
      FixedDimension = 19
      object TabSheet1: TRzTabSheet
        Color = clInfoBk
        OnShow = TabSheet1Show
        Caption = 'Lighting Parameters'
        object Bevel13: TBevel
          Left = 0
          Top = 0
          Width = 4
          Height = 485
          Align = alLeft
          Shape = bsSpacer
        end
        object RzTabBanks: TRzTabControl
          Left = 4
          Top = 0
          Width = 784
          Height = 485
          Align = alLeft
          BackgroundColor = clInfoBk
          BoldCurrentTab = True
          UseColoredTabs = True
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Verdana'
          Font.Style = []
          ParentBackgroundColor = False
          ParentFont = False
          SoftCorners = True
          TabIndex = 0
          TabOrder = 0
          TabOrientation = toBottom
          Tabs = <
            item
              Caption = 'Bank 0'
            end
            item
              Caption = 'Bank 1'
            end
            item
              Caption = 'Bank 2'
            end
            item
              Caption = 'Bank 3'
            end>
          OnChange = RzTabBanksChange
          FixedDimension = 19
          object btnActivateBank: TRzRapidFireButton
            Left = 337
            Top = 478
            Width = 105
            Height = 17
            Hint = 
              'Making this bank active means instructing the light controller t' +
              'o apply current bank configuration settings. Click here if you w' +
              'ant to see the effect of any changes made.'
            Action = actActivateBank
            Flat = True
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clDefault
            Font.Height = -11
            Font.Name = 'Verdana'
            Font.Style = [fsBold]
            ParentFont = False
            InitialDelay = 100
          end
          object RzSizePanel1: TRzSizePanel
            Left = 1
            Top = 1
            Width = 391
            Height = 464
            BorderInner = fsFlatRounded
            Color = clInfoBk
            Locked = True
            TabOrder = 1
            object RzGroupBox1: TRzGroupBox
              Left = 2
              Top = 22
              Width = 382
              Height = 48
              Align = alTop
              Caption = ' Voltage in %'
              TabOrder = 0
              object tbVoltageHead1: TRzTrackBar
                Left = 9
                Top = 15
                Width = 336
                Height = 27
                Max = 100
                PageSize = 10
                Position = 0
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                TabOrder = 0
              end
              object edVoltageHead1: TRzNumericEdit
                Left = 344
                Top = 17
                Width = 33
                Height = 21
                FrameVisible = True
                TabOrder = 1
                OnChange = EditChange
                DisplayFormat = ',0;(,0)'
              end
            end
            object RzGroupBox2: TRzGroupBox
              Left = 2
              Top = 70
              Width = 207
              Height = 392
              Align = alLeft
              Caption = 'Power'
              TabOrder = 1
              object tbChanel1Head1: TRzTrackBar
                Left = 8
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel1Head1Changing
                TabOrder = 0
              end
              object tbChanel2Head1: TRzTrackBar
                Left = 56
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel2Head1Changing
                TabOrder = 1
              end
              object tbChanel3Head1: TRzTrackBar
                Left = 104
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel3Head1Changing
                TabOrder = 2
              end
              object tbChanel4Head1: TRzTrackBar
                Left = 152
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel4Head1Changing
                TabOrder = 3
              end
              object edChanel1Head1: TRzNumericEdit
                Left = 6
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 4
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel2Head1: TRzNumericEdit
                Left = 54
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 5
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel3Head1: TRzNumericEdit
                Left = 102
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 6
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel4Head1: TRzNumericEdit
                Left = 150
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 7
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object pnlOverCurrentHead1Chanel1: TRzPanel
                Left = 8
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 8
              end
              object pnlOverCurrentHead1Chanel2: TRzPanel
                Left = 56
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 9
              end
              object pnlOverCurrentHead1Chanel3: TRzPanel
                Left = 104
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 10
              end
              object pnlOverCurrentHead1Chanel4: TRzPanel
                Left = 152
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 11
              end
              object grHead1Amplifier: TRzRadioGroup
                Left = 1
                Top = 352
                Width = 205
                Height = 39
                Align = alBottom
                Caption = 'Amplifier'
                Columns = 5
                ItemIndex = 0
                Items.Strings = (
                  '1X'
                  '2X'
                  '3X'
                  '4X'
                  '5X')
                TabOrder = 12
                OnChanging = grHead1AmplifierChanging
              end
              object grHead1Lock: TRzCheckGroup
                Left = 1
                Top = 314
                Width = 205
                Height = 38
                Align = alBottom
                Caption = 'Lock'
                Columns = 4
                Items.Strings = (
                  'Ch1'
                  'Ch2'
                  'Ch3'
                  'Ch4')
                TabOrder = 13
                CheckStates = (
                  0
                  0
                  0
                  0)
              end
            end
            object Panel1: TPanel
              Left = 2
              Top = 2
              Width = 382
              Height = 20
              Align = alTop
              BevelOuter = bvNone
              Caption = 'Output Head 1'
              Color = clInactiveCaption
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 15790320
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = [fsBold]
              ParentFont = False
              TabOrder = 2
            end
            object RzPanel8: TRzPanel
              Left = 209
              Top = 70
              Width = 175
              Height = 392
              Align = alClient
              BorderOuter = fsNone
              TabOrder = 3
              object RzGroupBox6: TRzGroupBox
                Left = 0
                Top = 287
                Width = 175
                Align = alBottom
                Caption = 'Output Strobe Control'
                TabOrder = 0
                object RzLabel1: TRzLabel
                  Left = 8
                  Top = 20
                  Width = 102
                  Height = 13
                  Caption = 'Strobe Delay (us)'
                end
                object RzLabel2: TRzLabel
                  Left = 8
                  Top = 56
                  Width = 118
                  Height = 13
                  Caption = 'Strobe Duration (us)'
                end
                object edDelayHead1: TRzNumericEdit
                  Left = 8
                  Top = 32
                  Width = 65
                  Height = 21
                  FrameVisible = True
                  TabOrder = 0
                  OnChange = EditChange
                  Max = 1000000
                  Min = 3
                  Value = 5
                  DisplayFormat = ',0;(,0)'
                end
                object edWidthHead1: TRzNumericEdit
                  Left = 8
                  Top = 68
                  Width = 65
                  Height = 21
                  FrameVisible = True
                  TabOrder = 1
                  OnChange = EditChange
                  Max = 1000000
                  Min = 1
                  Value = 5
                  DisplayFormat = ',0;(,0)'
                end
              end
              object RzGroupBox3: TRzGroupBox
                Left = 0
                Top = 0
                Width = 175
                Height = 287
                Align = alClient
                Caption = ' Trigger Control '
                TabOrder = 1
                object Bevel1: TBevel
                  Left = 1
                  Top = 14
                  Width = 3
                  Height = 272
                  Align = alLeft
                  Shape = bsSpacer
                end
                object Bevel2: TBevel
                  Left = 171
                  Top = 14
                  Width = 3
                  Height = 272
                  Align = alRight
                  Shape = bsSpacer
                end
                object RzPanel7: TRzPanel
                  Left = 4
                  Top = 14
                  Width = 167
                  Height = 272
                  Align = alClient
                  BorderOuter = fsNone
                  TabOrder = 0
                  object Bevel3: TBevel
                    Left = 0
                    Top = 0
                    Width = 167
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object Bevel4: TBevel
                    Left = 0
                    Top = 170
                    Width = 167
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object Bevel5: TBevel
                    Left = 0
                    Top = 267
                    Width = 167
                    Height = 5
                    Align = alBottom
                    Shape = bsSpacer
                  end
                  object Bevel6: TBevel
                    Left = 0
                    Top = 251
                    Width = 167
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object RzGroupBox5: TRzGroupBox
                    Left = 0
                    Top = 5
                    Width = 167
                    Height = 165
                    Align = alTop
                    Caption = 'Trigger Edge'
                    TabOrder = 0
                    object RzLine1: TRzLine
                      Left = 40
                      Top = 43
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine3: TRzLine
                      Left = 69
                      Top = 17
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saEnd
                    end
                    object RzLine4: TRzLine
                      Left = 88
                      Top = 17
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saStart
                    end
                    object RzLine2: TRzLine
                      Left = 71
                      Top = 17
                      Width = 22
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine5: TRzLine
                      Left = 90
                      Top = 42
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine6: TRzLine
                      Left = 40
                      Top = 67
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine7: TRzLine
                      Left = 69
                      Top = 66
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saStart
                    end
                    object RzLine8: TRzLine
                      Left = 70
                      Top = 93
                      Width = 22
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine9: TRzLine
                      Left = 88
                      Top = 67
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saEnd
                    end
                    object RzLine10: TRzLine
                      Left = 90
                      Top = 67
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object rbRaisingTriggerHead1: TRzRadioButton
                      Left = 24
                      Top = 37
                      Width = 17
                      Height = 17
                      Checked = True
                      TabOrder = 0
                      TabStop = True
                      OnClick = rbRaisingTriggerHead1Click
                    end
                    object rbFallingTriggerHead1: TRzRadioButton
                      Left = 24
                      Top = 75
                      Width = 17
                      Height = 17
                      TabOrder = 1
                      OnClick = rbFallingTriggerHead1Click
                    end
                    object rzDCModeHead1: TRzRadioButton
                      Left = 24
                      Top = 123
                      Width = 17
                      Height = 17
                      TabOrder = 2
                      OnClick = rzDCModeHead1Click
                    end
                    object RzPanel6: TRzPanel
                      Left = 48
                      Top = 107
                      Width = 57
                      Height = 49
                      BorderOuter = fsFlatBold
                      Caption = '-- DC-- '
                      TabOrder = 3
                    end
                  end
                  object rgTriggerSourceHead1: TRzRadioGroup
                    Left = 0
                    Top = 175
                    Width = 167
                    Height = 76
                    Align = alTop
                    Caption = 'Trigger Input'
                    ItemIndex = 0
                    Items.Strings = (
                      'Trigger 1'
                      'Trigger 2'
                      'Trigger 1 or 2')
                    SpaceEvenly = True
                    TabOrder = 1
                    OnChanging = rgTriggerSourceHead1Changing
                    OnClick = rgTriggerSourceHead1Click
                  end
                end
              end
            end
          end
          object RzSizePanel2: TRzSizePanel
            Left = 392
            Top = 1
            Width = 391
            Height = 464
            Align = alClient
            BorderInner = fsFlatRounded
            Color = clInfoBk
            Locked = True
            TabOrder = 0
            object Panel7: TPanel
              Left = 2
              Top = 2
              Width = 387
              Height = 20
              Align = alTop
              BevelOuter = bvNone
              Caption = 'Output Head 2'
              Color = clInactiveCaption
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 15790320
              Font.Height = -11
              Font.Name = 'MS Sans Serif'
              Font.Style = [fsBold]
              ParentFont = False
              TabOrder = 0
            end
            object RzGroupBox7: TRzGroupBox
              Left = 2
              Top = 22
              Width = 387
              Height = 48
              Align = alTop
              Caption = ' Voltage in %'
              TabOrder = 1
              object tbVoltageHead2: TRzTrackBar
                Left = 9
                Top = 15
                Width = 336
                Height = 27
                Max = 100
                PageSize = 10
                Position = 0
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                TabOrder = 0
              end
              object edVoltageHead2: TRzNumericEdit
                Left = 344
                Top = 17
                Width = 33
                Height = 21
                FrameVisible = True
                TabOrder = 1
                OnChange = EditChange
                DisplayFormat = ',0;(,0)'
              end
            end
            object RzGroupBox8: TRzGroupBox
              Left = 2
              Top = 70
              Width = 207
              Height = 392
              Align = alLeft
              Caption = 'Power'
              TabOrder = 2
              object tbChanel1Head2: TRzTrackBar
                Left = 8
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel1Head2Changing
                TabOrder = 0
              end
              object tbChanel2Head2: TRzTrackBar
                Left = 56
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel2Head2Changing
                TabOrder = 1
              end
              object tbChanel3Head2: TRzTrackBar
                Left = 104
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel3Head2Changing
                TabOrder = 2
              end
              object tbChanel4Head2: TRzTrackBar
                Left = 152
                Top = 40
                Width = 33
                Height = 241
                Max = 100
                Orientation = orVertical
                PageSize = 10
                Position = 0
                ThumbStyle = tsMixer
                TickStep = 5
                TrackOffset = 15
                TrackWidth = 4
                OnChange = TrackBarChange
                OnChanging = tbChanel4Head2Changing
                TabOrder = 3
              end
              object edChanel1Head2: TRzNumericEdit
                Left = 6
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 4
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel2Head2: TRzNumericEdit
                Left = 54
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 5
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel3Head2: TRzNumericEdit
                Left = 102
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 6
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object edChanel4Head2: TRzNumericEdit
                Left = 150
                Top = 288
                Width = 40
                Height = 21
                AutoSize = False
                FrameVisible = True
                TabOrder = 7
                OnChange = EditChange
                IntegersOnly = False
                Max = 100
                DisplayFormat = ',0.0;(,0.0)'
              end
              object pnlOverCurrentHead2Chanel1: TRzPanel
                Left = 8
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 8
              end
              object pnlOverCurrentHead2Chanel2: TRzPanel
                Left = 56
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 9
              end
              object pnlOverCurrentHead2Chanel3: TRzPanel
                Left = 104
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 10
              end
              object pnlOverCurrentHead2Chanel4: TRzPanel
                Left = 152
                Top = 16
                Width = 33
                Height = 25
                Hint = 'Undervoltage status'
                BorderOuter = fsFlatBold
                Caption = 'A'
                Color = clBackground
                ParentShowHint = False
                ShowHint = True
                TabOrder = 11
              end
              object grHead2Amplifier: TRzRadioGroup
                Left = 1
                Top = 352
                Width = 205
                Height = 39
                Align = alBottom
                BorderColor = clInfoBk
                Caption = 'Amplifier'
                Columns = 5
                ItemIndex = 0
                Items.Strings = (
                  '1X'
                  '2X'
                  '3X'
                  '4X'
                  '5X')
                TabOrder = 12
                OnChanging = grHead2AmplifierChanging
              end
              object grHead2Lock: TRzCheckGroup
                Left = 1
                Top = 314
                Width = 205
                Height = 38
                Align = alBottom
                Caption = 'Lock'
                Columns = 4
                Items.Strings = (
                  'Ch1'
                  'Ch2'
                  'Ch3'
                  'Ch4')
                TabOrder = 13
                CheckStates = (
                  0
                  0
                  0
                  0)
              end
            end
            object RzPanel11: TRzPanel
              Left = 209
              Top = 70
              Width = 180
              Height = 392
              Align = alClient
              BorderOuter = fsNone
              TabOrder = 3
              object RzGroupBox9: TRzGroupBox
                Left = 0
                Top = 287
                Width = 180
                Align = alBottom
                Caption = 'Output Strobe Control'
                TabOrder = 0
                object RzLabel5: TRzLabel
                  Left = 8
                  Top = 20
                  Width = 102
                  Height = 13
                  Caption = 'Strobe Delay (us)'
                end
                object RzLabel6: TRzLabel
                  Left = 8
                  Top = 56
                  Width = 118
                  Height = 13
                  Caption = 'Strobe Duration (us)'
                end
                object edDelayHead2: TRzNumericEdit
                  Left = 8
                  Top = 32
                  Width = 65
                  Height = 21
                  FrameVisible = True
                  TabOrder = 0
                  OnChange = EditChange
                  Max = 1000000
                  Min = 5
                  Value = 5
                  DisplayFormat = ',0;(,0)'
                end
                object edWidthHead2: TRzNumericEdit
                  Left = 8
                  Top = 68
                  Width = 65
                  Height = 21
                  FrameVisible = True
                  TabOrder = 1
                  OnChange = EditChange
                  Max = 1000000
                  Min = 5
                  Value = 5
                  DisplayFormat = ',0;(,0)'
                end
              end
              object RzGroupBox10: TRzGroupBox
                Left = 0
                Top = 0
                Width = 180
                Height = 287
                Align = alClient
                Caption = ' Trigger Control '
                TabOrder = 1
                object Bevel7: TBevel
                  Left = 1
                  Top = 14
                  Width = 3
                  Height = 272
                  Align = alLeft
                  Shape = bsSpacer
                end
                object Bevel8: TBevel
                  Left = 176
                  Top = 14
                  Width = 3
                  Height = 272
                  Align = alRight
                  Shape = bsSpacer
                end
                object RzPanel12: TRzPanel
                  Left = 4
                  Top = 14
                  Width = 172
                  Height = 272
                  Align = alClient
                  BorderOuter = fsNone
                  TabOrder = 0
                  object Bevel9: TBevel
                    Left = 0
                    Top = 0
                    Width = 172
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object Bevel10: TBevel
                    Left = 0
                    Top = 170
                    Width = 172
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object Bevel11: TBevel
                    Left = 0
                    Top = 267
                    Width = 172
                    Height = 5
                    Align = alBottom
                    Shape = bsSpacer
                  end
                  object Bevel12: TBevel
                    Left = 0
                    Top = 251
                    Width = 172
                    Height = 5
                    Align = alTop
                    Shape = bsSpacer
                  end
                  object RzGroupBox11: TRzGroupBox
                    Left = 0
                    Top = 5
                    Width = 172
                    Height = 165
                    Align = alTop
                    Caption = 'Trigger Edge'
                    TabOrder = 0
                    object RzLine11: TRzLine
                      Left = 40
                      Top = 43
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine12: TRzLine
                      Left = 69
                      Top = 17
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saEnd
                    end
                    object RzLine13: TRzLine
                      Left = 88
                      Top = 17
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saStart
                    end
                    object RzLine14: TRzLine
                      Left = 71
                      Top = 17
                      Width = 22
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine15: TRzLine
                      Left = 90
                      Top = 42
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine16: TRzLine
                      Left = 40
                      Top = 67
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine17: TRzLine
                      Left = 69
                      Top = 66
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saStart
                    end
                    object RzLine18: TRzLine
                      Left = 70
                      Top = 93
                      Width = 22
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object RzLine19: TRzLine
                      Left = 88
                      Top = 67
                      Width = 4
                      Height = 30
                      ArrowLength = 5
                      LineSlope = lsUp
                      ShowArrows = saEnd
                    end
                    object RzLine20: TRzLine
                      Left = 90
                      Top = 67
                      Width = 33
                      Height = 4
                      ArrowLength = 5
                      LineSlope = lsUp
                    end
                    object rbRaisingTriggerHead2: TRzRadioButton
                      Left = 24
                      Top = 37
                      Width = 17
                      Height = 17
                      Checked = True
                      TabOrder = 0
                      TabStop = True
                      OnClick = rbRaisingTriggerHead2Click
                    end
                    object rbFallingTriggerHead2: TRzRadioButton
                      Left = 24
                      Top = 75
                      Width = 17
                      Height = 17
                      TabOrder = 1
                      OnClick = rbFallingTriggerHead2Click
                    end
                    object rzDCModeHead2: TRzRadioButton
                      Left = 24
                      Top = 123
                      Width = 17
                      Height = 17
                      TabOrder = 2
                      OnClick = rzDCModeHead2Click
                    end
                    object RzPanel13: TRzPanel
                      Left = 48
                      Top = 107
                      Width = 57
                      Height = 49
                      BorderOuter = fsFlatBold
                      Caption = '-- DC-- '
                      TabOrder = 3
                    end
                  end
                  object rgTriggerSourceHead2: TRzRadioGroup
                    Left = 0
                    Top = 175
                    Width = 172
                    Height = 76
                    Align = alTop
                    Caption = 'Trigger Input'
                    ItemIndex = 0
                    Items.Strings = (
                      'Trigger 1'
                      'Trigger 2'
                      'Trigger 1 or 2')
                    SpaceEvenly = True
                    TabOrder = 1
                    OnChanging = rgTriggerSourceHead2Changing
                  end
                end
              end
            end
          end
        end
      end
      object TabSheet2: TRzTabSheet
        Color = 16776176
        OnShow = TabSheet2Show
        Caption = 'AdvancedParameters'
        object RzGroupBox4: TRzGroupBox
          Left = 0
          Top = 40
          Width = 201
          Height = 121
          Caption = 'Soft Trigger'
          Color = 16776176
          TabOrder = 0
          object RzLabel4: TRzLabel
            Left = 8
            Top = 48
            Width = 52
            Height = 13
            Caption = 'Period (ms)'
          end
          object RzBtnSoftTrigger: TRzBitBtn
            Left = 80
            Top = 48
            Width = 81
            Height = 41
            AllowAllUp = True
            Caption = 'Soft Trigger 1'
            TabOrder = 0
            OnClick = RzBtnSoftTriggerClick
          end
          object edAutoTriggerPeriod: TRzNumericEdit
            Left = 8
            Top = 64
            Width = 65
            Height = 21
            FrameVisible = True
            TabOrder = 1
            Max = 100000
            Min = 10
            Value = 500
            DisplayFormat = ',0;(,0)'
          end
          object cbAutoTrigger: TRzCheckBox
            Left = 8
            Top = 16
            Width = 177
            Height = 17
            Caption = 'Auto Trigger Bank 0 : Head 1'
            State = cbUnchecked
            TabOrder = 2
            OnClick = cbAutoTriggerClick
          end
        end
        object RzGroupBox12: TRzGroupBox
          Left = 208
          Top = 40
          Width = 209
          Height = 121
          Caption = 'EPROM Configuration'
          Color = 16776176
          TabOrder = 1
          object RzBitBtn2: TRzBitBtn
            Left = 8
            Top = 16
            Width = 81
            Height = 41
            Action = actLoadFromEPROM
            Caption = 'Load'
            Enabled = False
            TabOrder = 0
          end
          object RzBitBtn3: TRzBitBtn
            Left = 8
            Top = 64
            Width = 81
            Height = 41
            Action = actStoreToEPROM
            Caption = 'Store'
            Enabled = False
            TabOrder = 1
            Glyph.Data = {
              B60D0000424DB60D000000000000360000002800000030000000180000000100
              180000000000800D0000302E0000302E00000000000000000000FFFFFFFFFFFF
              FFFFFFFFFFFFFFF7FFC6C6C69CA59C8C948CA5A5A5D6CED6FFF7FFFFFFFFFFFF
              FFFFFFFFF7FFFFF7EFEFEFE7E7E7DEDEE7D6D6D6C6C6B5A5A5ADA5A5EFEFEFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCECECEADADAD9C9C9CB5B5B5DEDEDE
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFEFEFE7E7E7D6D6D6BDBD
              BDB5B5B5FFFFFFFFFFFFFFFFFFFFFFFFFFF7FFCECEC6427B42007308008C0000
              8C08007B08086B104A6B4A946373945A5A7B4A4A8442427331316B2121631818
              5A08085A0000420000390808D6DEDEFFFFFFFFFFFFFFFFFFFFFFFFD6D6D67373
              735252525A5A5A5A5A5A5252525252526B6B6B8C8C8C8C8C8C73737373737363
              63635A5A5A5252524A4A4A424242393939393939E7E7E7FFFFFFDED6D68C4252
              524231107B2129BD3110BD2100B51018DE2931CE4A42C64A21A5310021002100
              004A00004200005200006300006300006300006B10106B00005A1818DEDEDEFF
              FFFFE7E7E77B7B7B5A5A5A5A5A5A8484847B7B7B6B6B6B8C8C8C949494949494
              7373732929292929293939393939394242424A4A4A4A4A4A4A4A4A5252524A4A
              4A525252E7E7E7FFFFFFE7B5C64A000008731852DE6B31DE425A9C5A949C9400
              B51029C63952CE5A73E7844ACE63004A084A00005208085210106310106B1010
              7318186B18186308085A1818DEDEDEFFFFFFD6D6D6393939525252A5A5A59494
              948C8C8CA5A5A56B6B6B8C8C8C9C9C9CB5B5B59C9C9C3939393939394242424A
              4A4A5252525252525A5A5A5A5A5A4A4A4A525252E7E7E7FFFFFFEFDEE7215A10
              5ADE735AD663316B39EFDEEFFFFFFF6B946B10BD184ACE5A63BD6394EFA539BD
              5A1010005A08185221214A18185221216B29295A18186300006B1818DEDEDEFF
              FFFFEFEFEF4A4A4AADADADA5A5A5636363EFEFEFFFFFFF9494947B7B7B9C9C9C
              9C9C9CCECECE8C8C8C2121214A4A4A5252524A4A4A5252525A5A5A5252524A4A
              4A5A5A5AE7E7E7FFFFFFBDD6BD29A5396BAD738C9494F7D6EFD6F7DEB5E7BDFF
              EFFF5A9C5A29C63963CE7384CE847BE78C085A104A00086310105208084A0808
              5210107B1818730808631818DED6D6FFFFFFD6D6D67B7B7B9C9C9C9C9C9CEFEF
              EFEFEFEFD6D6D6FFFFFF8C8C8C8C8C8CA5A5A5B5B5B5BDBDBD4A4A4A39393952
              52524242424242424A4A4A636363525252525252E7E7E7FFFFFF8CBD9442AD52
              7BB584CECECEFFFFFF73C67B39B54AFFFFFFDECEDE52945A42BD527BD67B84E7
              94188C313908006B08105A10105A1010521010631818731010631010D6CECEFF
              FFFFB5B5B58C8C8CA5A5A5DEDEDEFFFFFFADADAD8C8C8CFFFFFFDEDEDE848484
              949494B5B5B5C6C6C66B6B6B3939395252524A4A4A4A4A4A4A4A4A5252525A5A
              5A525252DEDEDEFFFFFF7BB58442A54A7BD68452C6638CC6946BC67342B552AD
              D6ADFFFFFFEFD6E75A845A52B55A7BDE942994313910086310185A18185A1818
              5A18185218186B10106B1010CECECEFFFFFFA5A5A5848484B5B5B59C9C9CB5B5
              B5A5A5A58C8C8CCECECEFFFFFFEFEFEF848484949494BDBDBD73737339393952
              52525252525252525252524A4A4A525252525252DEDEDEFFFFFF8CBD8C31A539
              73D67B5ABD6352BD5A7BCE7B84CE8C73BD7BDEE7DEFFFFFFEFDEEF8CAD8C63DE
              6B218C314A0808731821632121632121632121632121631010731010CEC6C6FF
              FFFFB5B5B57B7B7BB5B5B59C9C9C949494B5B5B5B5B5B5A5A5A5EFEFEFFFFFFF
              EFEFEFADADADADADAD6B6B6B4242425A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5252
              525A5A5AD6D6D6FFFFFFBDD6BD188C2163CE6B6BC6737BCE8494CE9CADD6AD94
              D6A594C69CEFEFEFFFFFFFADDEB542CE5A1063105A08186B2121632121632121
              6321216321215A10106B1010D6C6C6FFFFFFD6D6D66B6B6BA5A5A5A5A5A5B5B5
              B5BDBDBDCECECEC6C6C6BDBDBDFFFFFFFFFFFFCECECE9494945252524A4A4A5A
              5A5A5A5A5A5A5A5A5A5A5A5A5A5A4A4A4A525252D6D6D6FFFFFFF7F7F7398C42
              39BD4A84D68C8CCE94B5CEB5BDD6BDBDD6C6A5CEA59CBDA5F7EFF7BDEFCE10AD
              313139187B10296B21216B21216B21216B21216B2121631010631010DEC6C6FF
              FFFFFFFFFF7373738C8C8CBDBDBDBDBDBDCECECED6D6D6D6D6D6C6C6C6BDBDBD
              FFFFFFDEDEDE7373734242425A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5252
              52525252DEDEDEFFFFFFFFFFFFB5ADA508730863DE7BADDEBDCED6CEE7D6DEDE
              D6DECEDECEADD6B5A5DEB542AD4A399439CEA5B56B21216B1818732929732929
              732929732929631818631010D6C6C6FFFFFFFFFFFFBDBDBD525252ADADADCECE
              CEDEDEDEE7E7E7E7E7E7DEDEDECECECECECECE8C8C8C7B7B7BC6C6C65A5A5A5A
              5A5A636363636363636363636363525252525252D6D6D6FFFFFFFFFFFFF7B5C6
              421800008C185ADE73ADDEB5DEE7DEDEDEDEC6EFC68CDE94109C21217B21D6D6
              CEEFDEDE6321216B18217329297329297329297329296B2121631818CEBDBDFF
              FFFFFFFFFFDEDEDE3939395A5A5AADADADCECECEEFEFEFE7E7E7E7E7E7C6C6C6
              6B6B6B636363DEDEDEEFEFEF5A5A5A5A5A5A6363636363636363636363635A5A
              5A525252CECECEFFFFFFFFFFFFEFCECE9410295A101018630810942129B54239
              B552189C2918942184AD7BF7D6DEFFFFFF947B7B5A1010733131733131733131
              7331317331316B2929631818CEBDBDFFFFFFFFFFFFE7E7E76B6B6B4A4A4A4A4A
              4A6B6B6B8484848C8C8C6B6B6B6B6B6BA5A5A5EFEFEFFFFFFF9494944A4A4A63
              63636363636363636363636363635A5A5A525252CECECEFFFFFFFFFFFFF7DEDE
              8C39317B18298C29395A1810392108392918421000632929CEADB5CEC6BD8C73
              6B4A00006B10107321217B31317B39397B31317B31317329296B1818C6B5B5FF
              FFFFFFFFFFFFFFFF7373736363636B6B6B4A4A4A3939394242423939395A5A5A
              C6C6C6CECECE8C8C8C3939395252525A5A5A6B6B6B6B6B6B6B6B6B6B6B6B6363
              635A5A5AC6C6C6FFFFFFFFFFFFFFE7E7AD52527329297B3939AD6B7BB57B8CB5
              7B8CB57B849C6363845A63846B6B7B524A8C5A5A94636B8C5A5A7B39397B4242
              7B4242844A4A844A4A842121C6B5B5FFFFFFFFFFFFFFFFFF9494946363636B6B
              6B9C9C9CA5A5A5A5A5A5A5A5A59494948484848C8C8C7373738484848C8C8C84
              84846B6B6B7373737373737B7B7B7B7B7B6B6B6BC6C6C6FFFFFFFFFFFFFFF7F7
              C663638C29296B3139E7EFE7DED6CE8C4A4AAD8C84E7EFE7E7DED6DECEC6DEDE
              D6D6DED6D6DED6DEEFE79C73737321218442427B42428C4A4A9C3131C6B5B5FF
              FFFFFFFFFFFFFFFFA5A5A56B6B6B636363FFFFFFDEDEDE7B7B7BA5A5A5FFFFFF
              E7E7E7DEDEDEE7E7E7E7E7E7E7E7E7EFEFEF9494945A5A5A7373737373737B7B
              7B7B7B7BC6C6C6FFFFFFFFFFFFFFFFFFC67373AD3939733939C6CECECEB5B563
              08088C4A4AD6CEC6CECEC6CEC6BDCEC6BDD6C6BDD6CEBDE7E7DEB594947B2929
              8C39397B3939A54A4AA53939C6B5B5FFFFFFFFFFFFFFFFFFADADAD8484846B6B
              6BD6D6D6CECECE4A4A4A7B7B7BD6D6D6D6D6D6CECECECECECED6D6D6D6D6D6EF
              EFEFB5B5B56363637373736B6B6B8C8C8C848484C6C6C6FFFFFFFFFFFFFFFFFF
              C68484BD4A4AAD5252BDBDB5CEC6BD7B3131945A5AD6C6C6CEC6BDCEC6BDCEC6
              BDCEC6BDCEC6BDDED6CEB59C9C7B3139843939A55252BD5A5A9C3939BDADADFF
              FFFFFFFFFFFFFFFFB5B5B5949494949494C6C6C6CECECE6B6B6B8C8C8CD6D6D6
              CECECECECECECECECECECECECECECEDEDEDEB5B5B56B6B6B7373738C8C8C9C9C
              9C7B7B7BC6C6C6FFFFFFFFFFFFFFFFFFC68484B53939B5525AC6B5B5CECEC684
              3131944A4ACEC6C6CECEC6CEC6BDCEC6BDCEC6BDCEC6BDDED6CEB5A59C6B3129
              944A4AC66B6BBD6363A54242A59494FFFFFFFFFFFFFFFFFFB5B5B58C8C8C9494
              94C6C6C6D6D6D66B6B6B848484D6D6D6D6D6D6CECECECECECECECECECECECEDE
              DEDEB5B5B55A5A5A848484A5A5A59C9C9C848484ADADADFFFFFFFFFFFFFFFFFF
              EFCECEC65A5A9C2929B59C9CD6D6CE944A428C4239C6BDB5CECEC6CEBDB5CEC6
              BDCEC6BDCEC6BDD6D6CEB5A5A5A54A4ACE6B6BC66B6BBD6B6BA54A4A948C8CFF
              FFFFFFFFFFFFFFFFE7E7E79C9C9C737373B5B5B5DEDEDE7B7B7B737373C6C6C6
              D6D6D6CECECECECECECECECECECECEDEDEDEBDBDBD8C8C8CADADADA5A5A5A5A5
              A58C8C8C9C9C9CFFFFFFFFFFFFFFFFFFFFFFFFEFC6C6C66B73CEBDBDD6D6CECE
              BDB5BDADA5C6BDADC6BDB5C6BDB5C6B5ADC6BDB5C6BDB5C6CEC6C6ADA5BD524A
              C6525AB55A5ABD5A5AB54A4A8C7B7BF7FFFFFFFFFFFFFFFFFFFFFFE7E7E7A5A5
              A5CECECEDEDEDECECECEBDBDBDC6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6D6
              D6D6C6C6C69494949C9C9C9494949C9C9C949494949494FFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFF7F7F7FFFFFFFFFFFFF7EFEFEFEFE7EFE7E7E7DE
              DEDED6D6D6CEC6D6D6D6CEC6BDC67B7BCE7B7BC67373CE6B6BAD42427B6363F7
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFEFEFEFE7E7E7D6D6D6DEDEDECECECEADADADB5B5B5ADADADADAD
              AD8C8C8C848484FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFF7F7FFF7F7FFEFEFE7C6C6C6ADADF7F7F7FFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
              FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDEDEDEC6C6C6FFFFFF}
            NumGlyphs = 2
          end
          object RzBitBtn4: TRzBitBtn
            Left = 112
            Top = 16
            Width = 81
            Height = 41
            Caption = 'Reset'
            Enabled = False
            TabOrder = 2
          end
        end
        object RzGroupBox13: TRzGroupBox
          Left = 0
          Top = 176
          Width = 417
          Height = 193
          Caption = 'Multi-Bank Configuration'
          Color = 16776176
          TabOrder = 2
          object RzLabel7: TRzLabel
            Left = 8
            Top = 85
            Width = 116
            Height = 13
            Caption = 'Bank Trigger Sequence:'
          end
          object RzbrUseTrigger1: TRzRadioButton
            Left = 8
            Top = 141
            Width = 281
            Height = 17
            Caption = 'Use "Trigger 1" Input to Select Next Bank'
            Checked = True
            Enabled = False
            TabOrder = 0
            TabStop = True
          end
          object RzrbUseNextSequenceInput: TRzRadioButton
            Left = 8
            Top = 165
            Width = 313
            Height = 17
            Caption = 'Use "Next Bank Sequence" Input to Select Next Bank'
            Enabled = False
            TabOrder = 1
          end
          object rzchkEnableBanks: TRzCheckBox
            Left = 8
            Top = 24
            Width = 201
            Height = 17
            Caption = 'Enable Multi Bank Configuration'
            State = cbUnchecked
            TabOrder = 2
            OnClick = rzchkEnableBanksClick
          end
          object RzBankSequence: TcxMaskEdit
            Left = 8
            Top = 104
            Width = 297
            Height = 21
            Properties.MaskKind = emkRegExpr
            Properties.EditMask = '(([0-3],{1}){1,256})*(([0-3])|([0-3],{1}))'
            Properties.MaxLength = 0
            TabOrder = 3
          end
          object rzbtnSendSequence: TRzBitBtn
            Left = 312
            Top = 96
            Width = 89
            Height = 41
            Caption = 'Send Sequence'
            TabOrder = 4
            OnClick = rzbtnSendSequenceClick
          end
          object rzchkAutoActivateBank: TRzCheckBox
            Left = 8
            Top = 48
            Width = 193
            Height = 17
            Hint = 
              'When a particular bank tab is selected automatically send the co' +
              'mmand to the controller to make this bank current. Select this o' +
              'ption if you want to see the results immediately for the configu' +
              'ration bank changes that you make.'
            Caption = 'Automatic bank activation'
            State = cbUnchecked
            TabOrder = 5
          end
        end
      end
      object tabLog: TRzTabSheet
        Caption = 'Log'
        object lbFullLog: TRzListBox
          Left = 0
          Top = 0
          Width = 784
          Height = 485
          Align = alClient
          FrameVisible = True
          ItemHeight = 13
          TabOrder = 0
        end
      end
    end
  end
  object TimerUpdate: TTimer
    Interval = 400
    OnTimer = TimerUpdateTimer
    Left = 504
  end
  object ActionList: TActionList
    Left = 408
    object actTriggerHead1: TAction
      Caption = 'Trigger'
      OnExecute = actTriggerHead1Execute
      OnUpdate = actTriggerHead1Update
    end
    object actStoreToEPROM: TAction
      Caption = 'Store'
      OnExecute = actStoreToEPROMExecute
      OnUpdate = actStoreToEPROMUpdate
    end
    object actLoadFromEPROM: TAction
      Caption = 'Load'
      OnExecute = actLoadFromEPROMExecute
      OnUpdate = actLoadFromEPROMUpdate
    end
    object actTriggerHead2: TAction
      Caption = 'Trigger'
      OnExecute = actTriggerHead2Execute
      OnUpdate = actTriggerHead2Update
    end
    object actActivateBank: TAction
      Caption = 'Activate Bank'
      Visible = False
      OnExecute = actActivateBankExecute
      OnUpdate = actActivateBankUpdate
    end
  end
  object RzVersionInfo: TRzVersionInfo
    Left = 568
  end
  object TimerAutoTrigger: TTimer
    Enabled = False
    OnTimer = TimerAutoTriggerTimer
    Left = 472
  end
  object RzRegIniFile: TRzRegIniFile
    Path = 'Software\Boreal\LD130'
    PathType = ptRegistry
    Left = 536
  end
  object TimerUpdateGUI: TTimer
    Enabled = False
    OnTimer = TimerUpdateGUITimer
    Left = 440
  end
end
