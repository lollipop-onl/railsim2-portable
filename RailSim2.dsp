# Microsoft Developer Studio Project File - Name="RailSim2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=RailSim2 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "RailSim2.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "RailSim2.mak" CFG="RailSim2 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "RailSim2 - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "RailSim2 - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RailSim2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"RailSim2\RailSim2.exe"

!ELSEIF  "$(CFG)" == "RailSim2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"RailSim2\RailSim2.exe" /pdbtype:sept
# SUBTRACT LINK32 /profile /incremental:no

!ENDIF 

# Begin Target

# Name "RailSim2 - Win32 Release"
# Name "RailSim2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "RS2 Source"

# PROP Default_Filter ""
# Begin Group "Misc Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\CCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\CPixelbit.cpp
# End Source File
# Begin Source File

SOURCE=.\CPixelbitStamp.cpp
# End Source File
# Begin Source File

SOURCE=.\CShadowVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\CStringTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\CVertexDump.cpp
# End Source File
# Begin Source File

SOURCE=.\CWaveArray.cpp
# End Source File
# End Group
# Begin Group "Interface Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCheckBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CDiaDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CDragContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\CEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CGroupBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\CListView.cpp
# End Source File
# Begin Source File

SOURCE=.\CMiniButton.cpp
# End Source File
# Begin Source File

SOURCE=.\CMultiStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\CPluginTree.cpp
# End Source File
# Begin Source File

SOURCE=.\CPopMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\CPushButton.cpp
# End Source File
# Begin Source File

SOURCE=.\CRadioButton.cpp
# End Source File
# Begin Source File

SOURCE=.\CScrollBarV.cpp
# End Source File
# Begin Source File

SOURCE=.\CSimpleDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CStaticCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CToggleIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\CTreeDirElement.cpp
# End Source File
# Begin Source File

SOURCE=.\CTreeElement.cpp
# End Source File
# Begin Source File

SOURCE=.\CTreeFileElement.cpp
# End Source File
# Begin Source File

SOURCE=.\CWindowCtrl.cpp
# End Source File
# End Group
# Begin Group "Mode Class Source"

# PROP Default_Filter ""
# Begin Group "RailwayMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CGirderSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CLineSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CPierSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CPoleSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailBuildMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailwayMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CTieSelectMode.cpp
# End Source File
# End Group
# Begin Group "TrainMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CDiaEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainSetMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainViewMode.cpp
# End Source File
# End Group
# Begin Group "StationMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CStationBuildMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CStationEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CStationSelectMode.cpp
# End Source File
# End Group
# Begin Group "StructMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CStructBuildMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CStructEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CStructSelectMode.cpp
# End Source File
# End Group
# Begin Group "SceneMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEnvEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CSceneEditMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CSimulationMode.cpp
# End Source File
# End Group
# Begin Group "SystemMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CConfigMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CExitMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CSkinSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CVideoMode.cpp
# End Source File
# End Group
# Begin Group "MiscMode Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\C3DPluginMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CGameMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CInterfaceMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CNeutralMode.cpp
# End Source File
# Begin Source File

SOURCE=.\CSceneryMode.cpp
# End Source File
# End Group
# End Group
# Begin Group "Plugin Class Source"

# PROP Default_Filter ""
# Begin Group "PluginParts Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCustomizer.cpp
# End Source File
# Begin Source File

SOURCE=.\CCustomizerMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\CCustomizerMover.cpp
# End Source File
# Begin Source File

SOURCE=.\CEffector.cpp
# End Source File
# Begin Source File

SOURCE=.\CExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\CLensFlare.cpp
# End Source File
# Begin Source File

SOURCE=.\CModelSwitch.cpp
# End Source File
# Begin Source File

SOURCE=.\CNamedObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CParticle.cpp
# End Source File
# Begin Source File

SOURCE=.\CSoundEffector.cpp
# End Source File
# Begin Source File

SOURCE=.\CTextureAnimation.cpp
# End Source File
# End Group
# Begin Group "PluginSet Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CRailwayPluginSet.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainGroupTemplate.cpp
# End Source File
# End Group
# Begin Group "RailwayPlugin Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CGirderPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CLinePlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CPierPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CPolePlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CProfilePlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiePlugin.cpp
# End Source File
# End Group
# Begin Group "ModelPlugin Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CModelPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CStationPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CStructPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CSurfacePlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainPlugin.cpp
# End Source File
# End Group
# Begin Group "Misc Plugin Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEnvPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\CSkinPlugin.cpp
# End Source File
# End Group
# End Group
# Begin Group "Instance Class Source"

# PROP Default_Filter ""
# Begin Group "RailwayInst Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CLine.cpp
# End Source File
# Begin Source File

SOURCE=.\CPier.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailConnector.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailWay.cpp
# End Source File
# End Group
# Begin Group "ModelInst Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CModelInst.cpp
# End Source File
# Begin Source File

SOURCE=.\CPartsInst.cpp
# End Source File
# Begin Source File

SOURCE=.\CScene.cpp
# End Source File
# Begin Source File

SOURCE=.\CStation.cpp
# End Source File
# Begin Source File

SOURCE=.\CStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrain.cpp
# End Source File
# End Group
# Begin Group "Misc Instance Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CDiaInst.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\CSaveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainGroup.cpp
# End Source File
# End Group
# End Group
# Begin Group "RailCurve Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CLineBuildCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailDetectCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailDumpCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailPlanCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CRailSplitCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\CTrainSetCurve.cpp
# End Source File
# End Group
# Begin Group "Misc Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Capture.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphicCover.cpp
# End Source File
# Begin Source File

SOURCE=.\HighTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Language.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\RailMap.cpp
# End Source File
# Begin Source File

SOURCE=.\RailSim2.cpp
# End Source File
# Begin Source File

SOURCE=.\RailSim2.rc
# End Source File
# Begin Source File

SOURCE=.\Script.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemCover.cpp
# End Source File
# Begin Source File

SOURCE=.\WakeUp.cpp
# End Source File
# End Group
# End Group
# Begin Group "UDX Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lib\anim.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\comm.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\debug.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\draw.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\editbox.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\effect.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\font.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\graphic.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\height_field.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\input.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\light.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\main.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\mesh.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\movie.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\music.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\object.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\offscreen.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\particle.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\sound.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\sprite.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\texture.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\vertex.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\view.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\view_ctrl.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\water_mesh.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\wave.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\wave_stream.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\lib\window.cpp
# SUBTRACT CPP /YX
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "RS2 Header"

# PROP Default_Filter ""
# Begin Group "Misc Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCamera.h
# End Source File
# Begin Source File

SOURCE=.\CCameraFocus.h
# End Source File
# Begin Source File

SOURCE=.\CCursor.h
# End Source File
# Begin Source File

SOURCE=.\CPixelbit.h
# End Source File
# Begin Source File

SOURCE=.\CShadowVolume.h
# End Source File
# Begin Source File

SOURCE=.\CStringTexture.h
# End Source File
# Begin Source File

SOURCE=.\CVertexDump.h
# End Source File
# Begin Source File

SOURCE=.\CWaveArray.h
# End Source File
# End Group
# Begin Group "Interface Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCheckBox.h
# End Source File
# Begin Source File

SOURCE=.\CDiaDialog.h
# End Source File
# Begin Source File

SOURCE=.\CDragContainer.h
# End Source File
# Begin Source File

SOURCE=.\CEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CGroupBox.h
# End Source File
# Begin Source File

SOURCE=.\CInterface.h
# End Source File
# Begin Source File

SOURCE=.\CListView.h
# End Source File
# Begin Source File

SOURCE=.\CMiniButton.h
# End Source File
# Begin Source File

SOURCE=.\CMultiStatic.h
# End Source File
# Begin Source File

SOURCE=.\CPluginTree.h
# End Source File
# Begin Source File

SOURCE=.\CPopMenu.h
# End Source File
# Begin Source File

SOURCE=.\CPushButton.h
# End Source File
# Begin Source File

SOURCE=.\CRadioButton.h
# End Source File
# Begin Source File

SOURCE=.\CScrollBarV.h
# End Source File
# Begin Source File

SOURCE=.\CSimpleDialog.h
# End Source File
# Begin Source File

SOURCE=.\CStaticCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CToggleIcon.h
# End Source File
# Begin Source File

SOURCE=.\CTrainListView.h
# End Source File
# Begin Source File

SOURCE=.\CTreeDirElement.h
# End Source File
# Begin Source File

SOURCE=.\CTreeElement.h
# End Source File
# Begin Source File

SOURCE=.\CTreeFileElement.h
# End Source File
# Begin Source File

SOURCE=.\CWindowCtrl.h
# End Source File
# End Group
# Begin Group "Mode Class Header"

# PROP Default_Filter ""
# Begin Group "RailwayMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CGirderSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CLineSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CPierSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CPoleSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CRailBuildMode.h
# End Source File
# Begin Source File

SOURCE=.\CRailEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CRailSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CRailwayMode.h
# End Source File
# Begin Source File

SOURCE=.\CTieSelectMode.h
# End Source File
# End Group
# Begin Group "TrainMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CDiaEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CTrainEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CTrainSetMode.h
# End Source File
# Begin Source File

SOURCE=.\CTrainViewMode.h
# End Source File
# End Group
# Begin Group "StationMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CStationBuildMode.h
# End Source File
# Begin Source File

SOURCE=.\CStationEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CStationSelectMode.h
# End Source File
# End Group
# Begin Group "StructMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CStructBuildMode.h
# End Source File
# Begin Source File

SOURCE=.\CStructEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CStructSelectMode.h
# End Source File
# End Group
# Begin Group "SceneMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEnvEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CSceneEditMode.h
# End Source File
# Begin Source File

SOURCE=.\CSimulationMode.h
# End Source File
# End Group
# Begin Group "SystemMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CConfigMode.h
# End Source File
# Begin Source File

SOURCE=.\CExitMode.h
# End Source File
# Begin Source File

SOURCE=.\CFileMode.h
# End Source File
# Begin Source File

SOURCE=.\CSkinSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\CVideoMode.h
# End Source File
# End Group
# Begin Group "MiscMode Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\C3DPluginMode.h
# End Source File
# Begin Source File

SOURCE=.\CGameMode.h
# End Source File
# Begin Source File

SOURCE=.\CInterfaceMode.h
# End Source File
# Begin Source File

SOURCE=.\CNeutralMode.h
# End Source File
# Begin Source File

SOURCE=.\CSceneryMode.h
# End Source File
# End Group
# End Group
# Begin Group "Plugin Class Header"

# PROP Default_Filter ""
# Begin Group "PluginParts Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CCustomizer.h
# End Source File
# Begin Source File

SOURCE=.\CCustomizerMisc.h
# End Source File
# Begin Source File

SOURCE=.\CCustomizerMover.h
# End Source File
# Begin Source File

SOURCE=.\CEffector.h
# End Source File
# Begin Source File

SOURCE=.\CExpression.h
# End Source File
# Begin Source File

SOURCE=.\CLensFlare.h
# End Source File
# Begin Source File

SOURCE=.\CModelSwitch.h
# End Source File
# Begin Source File

SOURCE=.\CNamedObject.h
# End Source File
# Begin Source File

SOURCE=.\CParticle.h
# End Source File
# Begin Source File

SOURCE=.\CSoundEffector.h
# End Source File
# Begin Source File

SOURCE=.\CTextureAnimation.h
# End Source File
# End Group
# Begin Group "PluginSet Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CRailwayPluginSet.h
# End Source File
# Begin Source File

SOURCE=.\CTrainGroupTemplate.h
# End Source File
# End Group
# Begin Group "RailwayPlugin Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CGirderPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CLinePlugin.h
# End Source File
# Begin Source File

SOURCE=.\CPierPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CPolePlugin.h
# End Source File
# Begin Source File

SOURCE=.\CProfilePlugin.h
# End Source File
# Begin Source File

SOURCE=.\CRailPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CTiePlugin.h
# End Source File
# End Group
# Begin Group "ModelPlugin Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CModelPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CStationPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CStructPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CSurfacePlugin.h
# End Source File
# Begin Source File

SOURCE=.\CTrainPlugin.h
# End Source File
# End Group
# Begin Group "Misc Plugin Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEnvPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CPlugin.h
# End Source File
# Begin Source File

SOURCE=.\CSkinPlugin.h
# End Source File
# End Group
# End Group
# Begin Group "Instance Class Header"

# PROP Default_Filter ""
# Begin Group "RailwayInst Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CLine.h
# End Source File
# Begin Source File

SOURCE=.\CPier.h
# End Source File
# Begin Source File

SOURCE=.\CRailConnector.h
# End Source File
# Begin Source File

SOURCE=.\CRailLink.h
# End Source File
# Begin Source File

SOURCE=.\CRailWay.h
# End Source File
# End Group
# Begin Group "ModelInst Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CModelInst.h
# End Source File
# Begin Source File

SOURCE=.\CPartsInst.h
# End Source File
# Begin Source File

SOURCE=.\CScene.h
# End Source File
# Begin Source File

SOURCE=.\CStation.h
# End Source File
# Begin Source File

SOURCE=.\CStruct.h
# End Source File
# Begin Source File

SOURCE=.\CTrain.h
# End Source File
# End Group
# Begin Group "Misc Instance Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CDiaInst.h
# End Source File
# Begin Source File

SOURCE=.\CRailBuilder.h
# End Source File
# Begin Source File

SOURCE=.\CSaveFile.h
# End Source File
# Begin Source File

SOURCE=.\CTrainGroup.h
# End Source File
# End Group
# End Group
# Begin Group "RailCurve Class Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CLineBuildCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailDetectCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailDumpCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailPlanCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailSplitCurve.h
# End Source File
# Begin Source File

SOURCE=.\CRailTraceCurve.h
# End Source File
# Begin Source File

SOURCE=.\CTrainSetBuffer.h
# End Source File
# Begin Source File

SOURCE=.\CTrainSetCurve.h
# End Source File
# End Group
# Begin Group "Misc Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Capture.h
# End Source File
# Begin Source File

SOURCE=.\Const.h
# End Source File
# Begin Source File

SOURCE=.\GraphicCover.h
# End Source File
# Begin Source File

SOURCE=.\HighTimer.h
# End Source File
# Begin Source File

SOURCE=.\Language.h
# End Source File
# Begin Source File

SOURCE=.\LanguageID.h
# End Source File
# Begin Source File

SOURCE=.\Macro.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
# End Source File
# Begin Source File

SOURCE=.\RailMap.h
# End Source File
# Begin Source File

SOURCE=.\RSPV.H
# End Source File
# Begin Source File

SOURCE=.\Script.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\SystemCover.h
# End Source File
# Begin Source File

SOURCE=.\ValueArea.h
# End Source File
# End Group
# End Group
# Begin Group "UDX Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lib\anim.h
# End Source File
# Begin Source File

SOURCE=.\lib\app.h
# End Source File
# Begin Source File

SOURCE=.\lib\bcc.h
# End Source File
# Begin Source File

SOURCE=.\lib\comm.h
# End Source File
# Begin Source File

SOURCE=.\lib\debug.h
# End Source File
# Begin Source File

SOURCE=.\lib\draw.h
# End Source File
# Begin Source File

SOURCE=.\lib\editbox.h
# End Source File
# Begin Source File

SOURCE=.\lib\effect.h
# End Source File
# Begin Source File

SOURCE=.\lib\font.h
# End Source File
# Begin Source File

SOURCE=.\lib\frame.h
# End Source File
# Begin Source File

SOURCE=.\lib\graphic.h
# End Source File
# Begin Source File

SOURCE=.\lib\headers.h
# End Source File
# Begin Source File

SOURCE=.\lib\height_field.h
# End Source File
# Begin Source File

SOURCE=.\lib\input.h
# End Source File
# Begin Source File

SOURCE=.\lib\libraries.h
# End Source File
# Begin Source File

SOURCE=.\lib\light.h
# End Source File
# Begin Source File

SOURCE=.\lib\mesh.h
# End Source File
# Begin Source File

SOURCE=.\lib\movie.h
# End Source File
# Begin Source File

SOURCE=.\lib\music.h
# End Source File
# Begin Source File

SOURCE=.\lib\mutex.h
# End Source File
# Begin Source File

SOURCE=.\lib\object.h
# End Source File
# Begin Source File

SOURCE=.\lib\offscreen.h
# End Source File
# Begin Source File

SOURCE=.\lib\particle.h
# End Source File
# Begin Source File

SOURCE=.\lib\render.h
# End Source File
# Begin Source File

SOURCE=.\lib\sound.h
# End Source File
# Begin Source File

SOURCE=.\lib\sprite.h
# End Source File
# Begin Source File

SOURCE=.\lib\sysvalue.h
# End Source File
# Begin Source File

SOURCE=.\lib\texture.h
# End Source File
# Begin Source File

SOURCE=.\lib\udx.h
# End Source File
# Begin Source File

SOURCE=.\lib\vertex.h
# End Source File
# Begin Source File

SOURCE=.\lib\view.h
# End Source File
# Begin Source File

SOURCE=.\lib\view_ctrl.h
# End Source File
# Begin Source File

SOURCE=.\lib\water_mesh.h
# End Source File
# Begin Source File

SOURCE=.\lib\wave.h
# End Source File
# Begin Source File

SOURCE=.\lib\wave_stream.h
# End Source File
# Begin Source File

SOURCE=.\lib\window.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Opening.bmp
# End Source File
# Begin Source File

SOURCE=.\res\RailSim2.ico
# End Source File
# End Group
# End Target
# End Project
