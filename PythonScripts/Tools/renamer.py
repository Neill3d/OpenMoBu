
#########################################################
##
## Renamer tool script for Autodesk MotionBuilder
##
## Author Sergey Solohin (Neill3d) 2013
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
#########################################################

from pyfbsdk import *
from pyfbsdk_additions import *

buttonName = FBButton()
buttonPrefix = FBButton()
editName = FBEdit()
editPrefix = FBEdit()
buttonRemPrefix = FBButton()
editRemPrefix = FBEdit()
region7 = FBLabel()
buttonPostfix = FBButton()
editPostfix = FBEdit()
buttonRemPost = FBButton()
editRemPost = FBEdit()
region12 = FBLabel()
buttonEnum = FBButton()
region14 = FBLabel()
editBaseNum = FBEdit()
region16 = FBLabel()
editStepNum = FBEdit()
buttonMask = FBButton()
region19 = FBLabel()
editMask = FBEdit()
region21 = FBLabel()
editReplace = FBEdit()
buttonRename = FBButton()

def BtnCallbackbuttonRename(control, event):
    list = FBModelList()
    FBGetSelectedModels(list)
    for i, obj in enumerate(list):
      #base name
      name = obj.Name
      #prefix
      if (buttonName.State==1): name = editName.Text
      if (buttonRemPrefix.State==1): 
        n = int(editRemPrefix.Text)
        name = name[n:]
      if (buttonPrefix.State==1): name = editPrefix.Text + name
      #postfix
      if (buttonRemPost.State==1):
        n = int(editRemPost.Text)
        print(name)
        name = name[:-n]
        print(name)
      if (buttonPostfix.State==1): name = name + editPostfix.Text
      # enumerate
      if (buttonEnum.State==1):
        base = int(editBaseNum.Text)
        step = int(editStepNum.Text)
        name = name + str((base + step*i))
      # replace by mask
      if (buttonMask.State==1):
        name = name.replace(editMask.Text, editReplace.Text)
      #result
      obj.Name = name


def PopulateTool(t):
    #populate regions here

    x = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonName","buttonName", x, y, w, h)

    t.SetControl("buttonName", buttonName)
    buttonName.Visible = True
    buttonName.ReadOnly = False
    buttonName.Enabled = True
    buttonName.Hint = ""
    buttonName.Caption = "Base name"
    buttonName.State = 0
    buttonName.Style = FBButtonStyle.kFBCheckbox
    buttonName.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonName.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(50,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPrefix","buttonPrefix", x, y, w, h)

    t.SetControl("buttonPrefix", buttonPrefix)
    buttonPrefix.Visible = True
    buttonPrefix.ReadOnly = False
    buttonPrefix.Enabled = True
    buttonPrefix.Hint = ""
    buttonPrefix.Caption = "Prefix"
    buttonPrefix.State = 0
    buttonPrefix.Style = FBButtonStyle.kFBCheckbox
    buttonPrefix.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonPrefix.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editName","editName", x, y, w, h)

    t.SetControl("editName", editName)
    editName.Visible = True
    editName.ReadOnly = False
    editName.Enabled = True
    editName.Hint = ""
    editName.Text = ""
    editName.PasswordMode = False
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(50,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editPrefix","editPrefix", x, y, w, h)

    t.SetControl("editPrefix", editPrefix)
    editPrefix.Visible = True
    editPrefix.ReadOnly = False
    editPrefix.Enabled = True
    editPrefix.Hint = ""
    editPrefix.Text = ""
    editPrefix.PasswordMode = False
    
    x = FBAddRegionParam(60,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(95,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(85,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonRemPrefix","buttonRemPrefix", x, y, w, h)

    t.SetControl("buttonRemPrefix", buttonRemPrefix)
    buttonRemPrefix.Visible = True
    buttonRemPrefix.ReadOnly = False
    buttonRemPrefix.Enabled = True
    buttonRemPrefix.Hint = ""
    buttonRemPrefix.Caption = "Remove prefix"
    buttonRemPrefix.State = 0
    buttonRemPrefix.Style = FBButtonStyle.kFBCheckbox
    buttonRemPrefix.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonRemPrefix.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(95,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(55,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editRemPrefix","editRemPrefix", x, y, w, h)

    t.SetControl("editRemPrefix", editRemPrefix)
    editRemPrefix.Visible = True
    editRemPrefix.ReadOnly = False
    editRemPrefix.Enabled = True
    editRemPrefix.Hint = ""
    editRemPrefix.Text = "0"
    editRemPrefix.PasswordMode = False
    
    x = FBAddRegionParam(210,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(95,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(65,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region7","region7", x, y, w, h)

    t.SetControl("region7", region7)
    region7.Visible = True
    region7.ReadOnly = False
    region7.Enabled = True
    region7.Hint = ""
    region7.Caption = "chars"
    region7.Style = FBTextStyle.kFBTextStyleNone
    region7.Justify = FBTextJustify.kFBTextJustifyCenter
    region7.WordWrap = False
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(140,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPostfix","buttonPostfix", x, y, w, h)

    t.SetControl("buttonPostfix", buttonPostfix)
    buttonPostfix.Visible = True
    buttonPostfix.ReadOnly = False
    buttonPostfix.Enabled = True
    buttonPostfix.Hint = ""
    buttonPostfix.Caption = "Postfix"
    buttonPostfix.State = 0
    buttonPostfix.Style = FBButtonStyle.kFBCheckbox
    buttonPostfix.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonPostfix.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(140,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editPostfix","editPostfix", x, y, w, h)

    t.SetControl("editPostfix", editPostfix)
    editPostfix.Visible = True
    editPostfix.ReadOnly = False
    editPostfix.Enabled = True
    editPostfix.Hint = ""
    editPostfix.Text = ""
    editPostfix.PasswordMode = False
    
    x = FBAddRegionParam(60,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(185,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(85,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonRemPost","buttonRemPost", x, y, w, h)

    t.SetControl("buttonRemPost", buttonRemPost)
    buttonRemPost.Visible = True
    buttonRemPost.ReadOnly = False
    buttonRemPost.Enabled = True
    buttonRemPost.Hint = ""
    buttonRemPost.Caption = "Remove postfix"
    buttonRemPost.State = 0
    buttonRemPost.Style = FBButtonStyle.kFBCheckbox
    buttonRemPost.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonRemPost.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(185,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(55,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editRemPost","editRemPost", x, y, w, h)

    t.SetControl("editRemPost", editRemPost)
    editRemPost.Visible = True
    editRemPost.ReadOnly = False
    editRemPost.Enabled = True
    editRemPost.Hint = ""
    editRemPost.Text = "0"
    editRemPost.PasswordMode = False
    
    x = FBAddRegionParam(215,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(185,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(60,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region12","region12", x, y, w, h)

    t.SetControl("region12", region12)
    region12.Visible = True
    region12.ReadOnly = False
    region12.Enabled = True
    region12.Hint = ""
    region12.Caption = "chars"
    region12.Style = FBTextStyle.kFBTextStyleNone
    region12.Justify = FBTextJustify.kFBTextJustifyCenter
    region12.WordWrap = False
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(245,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(145,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonEnum","buttonEnum", x, y, w, h)

    t.SetControl("buttonEnum", buttonEnum)
    buttonEnum.Visible = True
    buttonEnum.ReadOnly = False
    buttonEnum.Enabled = True
    buttonEnum.Hint = ""
    buttonEnum.Caption = "Enumerate"
    buttonEnum.State = 0
    buttonEnum.Style = FBButtonStyle.kFBCheckbox
    buttonEnum.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonEnum.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(75,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(275,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region14","region14", x, y, w, h)

    t.SetControl("region14", region14)
    region14.Visible = True
    region14.ReadOnly = False
    region14.Enabled = True
    region14.Hint = ""
    region14.Caption = "Base number"
    region14.Style = FBTextStyle.kFBTextStyleNone
    region14.Justify = FBTextJustify.kFBTextJustifyCenter
    region14.WordWrap = False
    
    x = FBAddRegionParam(210,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(275,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(50,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editBaseNum","editBaseNum", x, y, w, h)

    t.SetControl("editBaseNum", editBaseNum)
    editBaseNum.Visible = True
    editBaseNum.ReadOnly = False
    editBaseNum.Enabled = True
    editBaseNum.Hint = ""
    editBaseNum.Text = "0"
    editBaseNum.PasswordMode = False
    
    x = FBAddRegionParam(75,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(305,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region16","region16", x, y, w, h)

    t.SetControl("region16", region16)
    region16.Visible = True
    region16.ReadOnly = False
    region16.Enabled = True
    region16.Hint = ""
    region16.Caption = "Step"
    region16.Style = FBTextStyle.kFBTextStyleNone
    region16.Justify = FBTextJustify.kFBTextJustifyCenter
    region16.WordWrap = False
    
    x = FBAddRegionParam(210,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(305,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(50,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editStepNum","editStepNum", x, y, w, h)

    t.SetControl("editStepNum", editStepNum)
    editStepNum.Visible = True
    editStepNum.ReadOnly = False
    editStepNum.Enabled = True
    editStepNum.Hint = ""
    editStepNum.Text = "1"
    editStepNum.PasswordMode = False
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(340,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(145,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonMask","buttonMask", x, y, w, h)

    t.SetControl("buttonMask", buttonMask)
    buttonMask.Visible = True
    buttonMask.ReadOnly = False
    buttonMask.Enabled = True
    buttonMask.Hint = ""
    buttonMask.Caption = "Replace by mask"
    buttonMask.State = 0
    buttonMask.Style = FBButtonStyle.kFBCheckbox
    buttonMask.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonMask.Look = FBButtonLook.kFBLookNormal
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(375,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(90,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region19","region19", x, y, w, h)

    t.SetControl("region19", region19)
    region19.Visible = True
    region19.ReadOnly = False
    region19.Enabled = True
    region19.Hint = ""
    region19.Caption = "Mask text"
    region19.Style = FBTextStyle.kFBTextStyleNone
    region19.Justify = FBTextJustify.kFBTextJustifyLeft
    region19.WordWrap = True
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(395,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(105,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editMask","editMask", x, y, w, h)

    t.SetControl("editMask", editMask)
    editMask.Visible = True
    editMask.ReadOnly = False
    editMask.Enabled = True
    editMask.Hint = ""
    editMask.Text = ""
    editMask.PasswordMode = False
    
    x = FBAddRegionParam(160,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(375,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(90,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region21","region21", x, y, w, h)

    t.SetControl("region21", region21)
    region21.Visible = True
    region21.ReadOnly = False
    region21.Enabled = True
    region21.Hint = ""
    region21.Caption = "Replace with"
    region21.Style = FBTextStyle.kFBTextStyleNone
    region21.Justify = FBTextJustify.kFBTextJustifyLeft
    region21.WordWrap = True
    
    x = FBAddRegionParam(160,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(395,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(105,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editReplace","editReplace", x, y, w, h)

    t.SetControl("editReplace", editReplace)
    editReplace.Visible = True
    editReplace.ReadOnly = False
    editReplace.Enabled = True
    editReplace.Hint = ""
    editReplace.Text = ""
    editReplace.PasswordMode = False
    
    x = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(445,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(130,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonRename","buttonRename", x, y, w, h)

    t.SetControl("buttonRename", buttonRename)
    buttonRename.Visible = True
    buttonRename.ReadOnly = False
    buttonRename.Enabled = True
    buttonRename.Hint = ""
    buttonRename.Caption = "Rename!"
    buttonRename.State = 0
    buttonRename.Style = FBButtonStyle.kFBPushButton
    buttonRename.Justify = FBTextJustify.kFBTextJustifyCenter
    buttonRename.Look = FBButtonLook.kFBLookNormal
    buttonRename.OnClick.Add(BtnCallbackbuttonRename)
    
def CreateTool():
    t = None
    #t = CreateUniqueTool("Renamer tool (by Neill3d)")
    try:
        t = FBCreateUniqueTool("Renamer tool (by Neill3d)")
    except NameError:
        t = CreateUniqueTool("Renamer tool (by Neill3d)")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 292
        t.StartSizeY = 514
        PopulateTool(t)
        ShowTool(t)
CreateTool()
