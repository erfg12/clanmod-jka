VERSION 5.00
Begin VB.Form Form1 
   BackColor       =   &H00000000&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Clan Mod - Administration Bitvalue Calculator"
   ClientHeight    =   4560
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   7050
   Icon            =   "admin_calc.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4560
   ScaleWidth      =   7050
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox Check32 
      BackColor       =   &H00000000&
      Caption         =   "AmVSTR"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   44
      Top             =   2880
      Width           =   1455
   End
   Begin VB.CheckBox Check22 
      BackColor       =   &H00000000&
      Caption         =   "ChangeMode"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   43
      Top             =   2640
      Width           =   1455
   End
   Begin VB.CheckBox Check31 
      BackColor       =   &H00000000&
      Caption         =   "Who IP"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   42
      Top             =   2640
      Width           =   1215
   End
   Begin VB.CheckBox Check30 
      BackColor       =   &H00000000&
      Caption         =   "AddModel"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   41
      Top             =   2880
      Width           =   1215
   End
   Begin VB.CheckBox Check29 
      BackColor       =   &H00000000&
      Caption         =   "LockTeam"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   40
      Top             =   2880
      Width           =   1215
   End
   Begin VB.CheckBox Check28 
      BackColor       =   &H00000000&
      Caption         =   "Slap"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   39
      Top             =   2400
      Width           =   1215
   End
   Begin VB.CheckBox Check27 
      BackColor       =   &H00000000&
      Caption         =   "Sleep"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   38
      Top             =   2400
      Width           =   1215
   End
   Begin VB.CheckBox Check26 
      BackColor       =   &H00000000&
      Caption         =   "Punish"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   37
      Top             =   2640
      Width           =   1215
   End
   Begin VB.CheckBox Check25 
      BackColor       =   &H00000000&
      Caption         =   "AddEffect"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   36
      Top             =   2160
      Width           =   1215
   End
   Begin VB.CheckBox Check24 
      BackColor       =   &H00000000&
      Caption         =   "Weather"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   35
      Top             =   1920
      Width           =   1215
   End
   Begin VB.CheckBox Check23 
      BackColor       =   &H00000000&
      Caption         =   "Monk"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   34
      Top             =   2640
      Width           =   1095
   End
   Begin VB.CheckBox Check21 
      BackColor       =   &H00000000&
      Caption         =   "ForceTeam"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   33
      Top             =   2400
      Width           =   1215
   End
   Begin VB.CheckBox Check20 
      BackColor       =   &H00000000&
      Caption         =   "CSPrint"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   32
      Top             =   2400
      Width           =   1215
   End
   Begin VB.CheckBox Check19 
      BackColor       =   &H00000000&
      Caption         =   "LockName"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   31
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CheckBox Check18 
      BackColor       =   &H00000000&
      Caption         =   "Rename"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   30
      Top             =   1440
      Width           =   1215
   End
   Begin VB.CheckBox Check17 
      BackColor       =   &H00000000&
      Caption         =   "Empower"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   29
      Top             =   1200
      Width           =   1215
   End
   Begin VB.PictureBox Picture1 
      BackColor       =   &H80000007&
      BorderStyle     =   0  'None
      Height          =   735
      Left            =   1440
      Picture         =   "admin_calc.frx":08CA
      ScaleHeight     =   735
      ScaleWidth      =   3495
      TabIndex        =   28
      Top             =   120
      Width           =   3495
   End
   Begin VB.Timer Timer1 
      Interval        =   1
      Left            =   4680
      Top             =   4680
   End
   Begin VB.TextBox Text1 
      Alignment       =   2  'Center
      Appearance      =   0  'Flat
      BackColor       =   &H00404040&
      ForeColor       =   &H00FFFFFF&
      Height          =   285
      Left            =   120
      TabIndex        =   25
      Top             =   4200
      Width           =   6855
   End
   Begin VB.OptionButton Option5 
      BackColor       =   &H00000000&
      Caption         =   "Admin level4"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4200
      TabIndex        =   20
      Top             =   3720
      Width           =   1215
   End
   Begin VB.OptionButton Option4 
      BackColor       =   &H00000000&
      Caption         =   "Admin level5"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   5520
      TabIndex        =   19
      Top             =   3720
      Width           =   1215
   End
   Begin VB.OptionButton Option3 
      BackColor       =   &H00000000&
      Caption         =   "Admin level3"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   2880
      TabIndex        =   18
      Top             =   3720
      Width           =   1215
   End
   Begin VB.OptionButton Option2 
      BackColor       =   &H00000000&
      Caption         =   "Admin level2"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1560
      TabIndex        =   17
      Top             =   3720
      Width           =   1215
   End
   Begin VB.OptionButton Option1 
      BackColor       =   &H00000000&
      Caption         =   "Admin level1"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   240
      TabIndex        =   16
      Top             =   3720
      Value           =   -1  'True
      Width           =   1215
   End
   Begin VB.CheckBox Check16 
      BackColor       =   &H00000000&
      Caption         =   "ChangeMap"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   15
      Top             =   2880
      Width           =   1215
   End
   Begin VB.CheckBox Check15 
      BackColor       =   &H00000000&
      Caption         =   "GrantAdmin"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   14
      Top             =   2160
      Width           =   1215
   End
   Begin VB.CheckBox Check14 
      BackColor       =   &H00000000&
      Caption         =   "Slay"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   13
      Top             =   1920
      Width           =   1095
   End
   Begin VB.CheckBox Check13 
      BackColor       =   &H00000000&
      Caption         =   "Splat"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   12
      Top             =   1680
      Width           =   1095
   End
   Begin VB.CheckBox Check12 
      BackColor       =   &H00000000&
      Caption         =   "Scale"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   11
      Top             =   1440
      Width           =   1095
   End
   Begin VB.CheckBox Check11 
      BackColor       =   &H00000000&
      Caption         =   "AdminBan"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   3360
      TabIndex        =   10
      Top             =   1200
      Width           =   1095
   End
   Begin VB.CheckBox Check10 
      BackColor       =   &H00000000&
      Caption         =   "DemiGod"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   9
      Top             =   2160
      Width           =   1095
   End
   Begin VB.CheckBox Check9 
      BackColor       =   &H00000000&
      Caption         =   "Terminator"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   8
      Top             =   1920
      Width           =   1095
   End
   Begin VB.CheckBox Check8 
      BackColor       =   &H00000000&
      Caption         =   "InsultSilence"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   7
      Top             =   1680
      Width           =   1215
   End
   Begin VB.CheckBox Check7 
      BackColor       =   &H00000000&
      Caption         =   "NPC"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   6
      Top             =   1440
      Width           =   1095
   End
   Begin VB.CheckBox Check6 
      BackColor       =   &H00000000&
      Caption         =   "amKick"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   1800
      TabIndex        =   5
      Top             =   1200
      Width           =   1095
   End
   Begin VB.CheckBox Check5 
      BackColor       =   &H00000000&
      Caption         =   "amBan"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   2160
      Width           =   1095
   End
   Begin VB.CheckBox Check4 
      BackColor       =   &H00000000&
      Caption         =   "Protect"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   3
      Top             =   1920
      Width           =   1095
   End
   Begin VB.CheckBox Check3 
      BackColor       =   &H00000000&
      Caption         =   "Silence"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   1680
      Width           =   1095
   End
   Begin VB.CheckBox Check2 
      BackColor       =   &H00000000&
      Caption         =   "Freeze"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   1440
      Width           =   1095
   End
   Begin VB.CheckBox Check1 
      BackColor       =   &H00000000&
      Caption         =   "AdminTele"
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   1200
      Width           =   1095
   End
   Begin VB.Shape Shape2 
      BorderColor     =   &H00FFFFFF&
      BorderWidth     =   3
      Height          =   255
      Left            =   5880
      Top             =   3240
      Width           =   855
   End
   Begin VB.Label Label6 
      Alignment       =   2  'Center
      BackColor       =   &H00404040&
      Caption         =   "ALL"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   5880
      TabIndex        =   27
      Top             =   3240
      Width           =   855
   End
   Begin VB.Label Label5 
      Caption         =   " "
      Height          =   375
      Left            =   2400
      TabIndex        =   26
      Top             =   4680
      Width           =   255
   End
   Begin VB.Label Label4 
      Caption         =   """"
      Height          =   255
      Left            =   2760
      TabIndex        =   24
      Top             =   4680
      Width           =   255
   End
   Begin VB.Label Label3 
      Caption         =   "0"
      Height          =   255
      Left            =   3120
      TabIndex        =   23
      Top             =   4680
      Width           =   1455
   End
   Begin VB.Label Label2 
      Caption         =   "set cm_adminControl1"
      Height          =   255
      Left            =   120
      TabIndex        =   22
      Top             =   4680
      Width           =   2175
   End
   Begin VB.Shape Shape1 
      BackColor       =   &H000000FF&
      BorderColor     =   &H00FFFFFF&
      BorderWidth     =   3
      Height          =   255
      Left            =   4920
      Top             =   3240
      Width           =   855
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackColor       =   &H00404040&
      Caption         =   "Reset"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   255
      Left            =   4920
      TabIndex        =   21
      Top             =   3240
      Width           =   855
   End
   Begin VB.Line Line2 
      BorderColor     =   &H00FFFFFF&
      BorderWidth     =   3
      X1              =   0
      X2              =   7080
      Y1              =   4080
      Y2              =   4080
   End
   Begin VB.Line Line1 
      BorderColor     =   &H00FFFFFF&
      BorderWidth     =   3
      X1              =   0
      X2              =   7080
      Y1              =   3600
      Y2              =   3600
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Check1_Click()
If Check1.Value = 0 Then
Label3.Caption = Label3 - 1
Else
Label3.Caption = Label3 + 1
End If
End Sub

Private Sub Check10_Click()
If Check10.Value = 0 Then
Label3.Caption = Label3 - 512
Else
Label3.Caption = Label3 + 512
End If
End Sub

Private Sub Check11_Click()
If Check11.Value = 0 Then
Label3.Caption = Label3 - 1024
Else
Label3.Caption = Label3 + 1024
End If

End Sub

Private Sub Check12_Click()
If Check12.Value = 0 Then
Label3.Caption = Label3 - 2048
Else
Label3.Caption = Label3 + 2048
End If
End Sub

Private Sub Check13_Click()
If Check13.Value = 0 Then
Label3.Caption = Label3 - 4096
Else
Label3.Caption = Label3 + 4096
End If

End Sub

Private Sub Check14_Click()
If Check14.Value = 0 Then
Label3.Caption = Label3 - 8192
Else
Label3.Caption = Label3 + 8192
End If

End Sub

Private Sub Check15_Click()
If Check15.Value = 0 Then
Label3.Caption = Label3 - 16384
Else
Label3.Caption = Label3 + 16384
End If

End Sub

Private Sub Check16_Click()
If Check16.Value = 0 Then
Label3.Caption = Label3 - 32768
Else
Label3.Caption = Label3 + 32768
End If

End Sub

Private Sub Check17_Click()
If Check17.Value = 0 Then
Label3.Caption = Label3 - 65536
Else
Label3.Caption = Label3 + 65536
End If

End Sub

Private Sub Check18_Click()
If Check18.Value = 0 Then
Label3.Caption = Label3 - 131072
Else
Label3.Caption = Label3 + 131072
End If

End Sub

Private Sub Check19_Click()
If Check19.Value = 0 Then
Label3.Caption = Label3 - 262144
Else
Label3.Caption = Label3 + 262144
End If

End Sub

Private Sub Check2_Click()
If Check2.Value = 0 Then
Label3.Caption = Label3 - 2
Else
Label3.Caption = Label3 + 2
End If

End Sub

Private Sub Check20_Click()
If Check20.Value = 0 Then
Label3.Caption = Label3 - 524288
Else
Label3.Caption = Label3 + 524288
End If
End Sub

Private Sub Check21_Click()
If Check21.Value = 0 Then
Label3.Caption = Label3 - 1048576
Else
Label3.Caption = Label3 + 1048576
End If
End Sub

Private Sub Check22_Click()
If Check22.Value = 0 Then
Label3.Caption = Label3 - 2097152
Else
Label3.Caption = Label3 + 2097152
End If
End Sub

Private Sub Check23_Click()
If Check23.Value = 0 Then
Label3.Caption = Label3 - 4194304
Else
Label3.Caption = Label3 + 4194304
End If
End Sub

Private Sub Check24_Click()
If Check24.Value = 0 Then
Label3.Caption = Label3 - 8388608
Else
Label3.Caption = Label3 + 8388608
End If
End Sub

Private Sub Check25_Click()
If Check25.Value = 0 Then
Label3.Caption = Label3 - 16777216
Else
Label3.Caption = Label3 + 16777216
End If
End Sub

Private Sub Check26_Click()
If Check26.Value = 0 Then
Label3.Caption = Label3 - 33554432
Else
Label3.Caption = Label3 + 33554432
End If
End Sub

Private Sub Check27_Click()
If Check27.Value = 0 Then
Label3.Caption = Label3 - 67108864
Else
Label3.Caption = Label3 + 67108864
End If
End Sub

Private Sub Check28_Click()
If Check28.Value = 0 Then
Label3.Caption = Label3 - 134217728
Else
Label3.Caption = Label3 + 134217728
End If
End Sub

Private Sub Check29_Click()
If Check29.Value = 0 Then
Label3.Caption = Label3 - 268435456
Else
Label3.Caption = Label3 + 268435456
End If
End Sub

Private Sub Check3_Click()
If Check3.Value = 0 Then
Label3.Caption = Label3 - 4
Else
Label3.Caption = Label3 + 4
End If
End Sub

Private Sub Check30_Click()
If Check30.Value = 0 Then
Label3.Caption = Label3 - 536870912
Else
Label3.Caption = Label3 + 536870912
End If

End Sub

Private Sub Check31_Click()
If Check31.Value = 0 Then
Label3.Caption = Label3 - 1073741824
Else
Label3.Caption = Label3 + 1073741824
End If

End Sub

Private Sub Check32_Click()
If Check32.Value = 0 Then
Label3.Caption = Label3 - 2147483648#
Else
Label3.Caption = Label3 + 2147483648#
End If

End Sub

Private Sub Check4_Click()
If Check4.Value = 0 Then
Label3.Caption = Label3 - 8
Else
Label3.Caption = Label3 + 8
End If

End Sub

Private Sub Check5_Click()
If Check5.Value = 0 Then
Label3.Caption = Label3 - 16
Else
Label3.Caption = Label3 + 16
End If

End Sub

Private Sub Check6_Click()
If Check6.Value = 0 Then
Label3.Caption = Label3 - 32
Else
Label3.Caption = Label3 + 32
End If

End Sub

Private Sub Check7_Click()
If Check7.Value = 0 Then
Label3.Caption = Label3 - 64
Else
Label3.Caption = Label3 + 64
End If

End Sub

Private Sub Check8_Click()
If Check8.Value = 0 Then
Label3.Caption = Label3 - 128
Else
Label3.Caption = Label3 + 128
End If

End Sub

Private Sub Check9_Click()
If Check9.Value = 0 Then
Label3.Caption = Label3 - 256
Else
Label3.Caption = Label3 + 256
End If

End Sub

Private Sub Label1_Click()
Option1.Value = True
Check1.Value = 0
Check2.Value = 0
Check3.Value = 0
Check4.Value = 0
Check5.Value = 0
Check6.Value = 0
Check7.Value = 0
Check8.Value = 0
Check9.Value = 0
Check10.Value = 0
Check11.Value = 0
Check12.Value = 0
Check13.Value = 0
Check14.Value = 0
Check15.Value = 0
Check16.Value = 0
Check17.Value = 0
Check18.Value = 0
Check19.Value = 0
Check20.Value = 0
Check21.Value = 0
Check22.Value = 0
Check23.Value = 0
Check24.Value = 0
Check25.Value = 0
Check26.Value = 0
Check27.Value = 0
Check28.Value = 0
Check29.Value = 0
Check30.Value = 0
Check31.Value = 0
Check32.Value = 0
Label3.Caption = "0"
End Sub

Private Sub Label6_Click()
Check1.Value = 1
Check2.Value = 1
Check3.Value = 1
Check4.Value = 1
Check5.Value = 1
Check6.Value = 1
Check7.Value = 1
Check8.Value = 1
Check9.Value = 1
Check10.Value = 1
Check11.Value = 1
Check12.Value = 1
Check13.Value = 1
Check14.Value = 1
Check15.Value = 1
Check16.Value = 1
Check17.Value = 1
Check18.Value = 1
Check19.Value = 1
Check20.Value = 1
Check21.Value = 1
Check22.Value = 1
Check23.Value = 1
Check24.Value = 1
Check25.Value = 1
Check26.Value = 1
Check27.Value = 1
Check28.Value = 1
Check29.Value = 1
Check30.Value = 1
Check31.Value = 1
Check32.Value = 1
End Sub

Private Sub Option1_Click()
Label2.Caption = "set cm_adminControl1 "
End Sub

Private Sub Option2_Click()
Label2.Caption = "set cm_adminControl2 "
End Sub

Private Sub Option3_Click()
Label2.Caption = "set cm_adminControl3 "
End Sub

Private Sub Option4_Click()
Label2.Caption = "set cm_adminControl5 "
End Sub

Private Sub Option5_Click()
Label2.Caption = "set cm_adminControl4 "
End Sub

Private Sub Timer1_Timer()
Text1.Text = Label2 & Label5 & Label4 & Label3 & Label4
End Sub
