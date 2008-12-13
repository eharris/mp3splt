#!/bin/sh
#we should have done '!/bin/bash', but it won't work on msys+mingw

#if the first argument of the script is not empty, then we consider a cross
#compilation; otherwise, consider a windows installation
we_dont_cross_compile=$1

#we move in the current script directory
script_dir=$(readlink -f $0) || exit 1
script_dir=${script_dir%\/*.sh}
cd $script_dir

. ../include_variables.sh

ARCH="i386"

if [[ -z $we_dont_cross_compile ]];then
  put_package "windows_installer"
  DLL_SUFFIX=""
else
  put_package "cross_windows_installer"
  DLL_SUFFIX="-0"
fi

#generate the '.nsi' installer script

WIN_INSTALLER_FILE="win32_installer.nsi"

TMP_GENERATED_FILES_FILE='.mp3splt_tmp_uninstall_files.txt'
TMP_CREATED_DIRECTORIES_FILE='.mp3splt_tmp_uninstall_directories.txt'
TMP_CHECK_SECTIONS_UNINSTALL_FILE='.mp3splt_tmp_check_sections_uninstall.txt'

echo '' > $TMP_GENERATED_FILES_FILE
echo '' > $TMP_CREATED_DIRECTORIES_FILE
echo '' > $TMP_CHECK_SECTIONS_UNINSTALL_FILE
 

CURRENT_OUT_PATH=""

function set_out_path()
{
  out_path=$1
  CURRENT_OUT_PATH=$out_path
  echo '    SetOutPath '$out_path >> $WIN_INSTALLER_FILE
}

#-generate function files and keeps the files in a temporary files for
#generating the uninstall section
#-requires as parameter a list of files
function copy_files()
{
  FILES=$@
  for file in $FILES;do
    echo '    File ${MP3SPLT_PATH}'"\\$file" >> $WIN_INSTALLER_FILE
    file_without_path=${file##*\\}
    echo '  Delete '"$CURRENT_OUT_PATH\\$file_without_path" >> $TMP_GENERATED_FILES_FILE
  done
}

#-function that creates a directory with NSI commands and keeps the
#directories in a temporary file for generating the uninstaller
function create_directory()
{
  DIR=$@
  echo '    CreateDirectory '$DIR >> $WIN_INSTALLER_FILE
  echo "  RmDir $DIR" >> $TMP_CREATED_DIRECTORIES_FILE
}

#-creates the start of a section
#-must receive 4 parameters : 
#  * the flags of the start of the section
#  * the section name
#  * the section identifier in the '.nsi' script
#  * the fourth parameter : "yes" if we create a directory in the section,
#    "no" otherwise
function start_section()
{
  section_name=$1
  section_id=$2
  we_create_directory=$3

  echo "
  Section \"$section_name\" $section_id

    DetailPrint \"\"
    DetailPrint \"Installing section "$section_id" :\"
    DetailPrint \"\"
" >> $WIN_INSTALLER_FILE

  #write if we install the section in the 'installed_sections.ini' file
  echo '
  SectionGetFlags ${'$section_id'} $0
  IntOp $1 $0 & ${SF_SELECTED}
  WriteINIStr $INSTDIR\installed_sections.ini '$section_id' "installed" $1' \
 >> $TMP_CHECK_SECTIONS_UNINSTALL_FILE

  #condition to uninstall the directories of this section only if the
  #section is found installed from the 'installed_sections.ini' file
  if [[ $we_create_directory = "yes" ]];then
    echo '
 ReadINIStr $0 $INSTDIR\installed_sections.ini '$section_id' "installed"
 IntCmp 0 $0 after_dirs_'$section_id >> $TMP_CREATED_DIRECTORIES_FILE
    echo '  DetailPrint ""' >> $TMP_CREATED_DIRECTORIES_FILE
    echo '  DetailPrint "Uninstalling dirs from section '$section_id' :"' >> $TMP_CREATED_DIRECTORIES_FILE
    echo '  DetailPrint ""' >> $TMP_CREATED_DIRECTORIES_FILE
  fi

  #condition to uninstall the files of this section only if the
  #section is found installed from the 'installed_sections.ini' file
  echo '
 ReadINIStr $0 $INSTDIR\installed_sections.ini '$section_id' "installed"
 IntCmp 0 $0 after_files_'$section_id >> $TMP_GENERATED_FILES_FILE
  echo '  DetailPrint ""' >> $TMP_GENERATED_FILES_FILE
  echo '  DetailPrint "Uninstalling files from section '$section_id' :"' >> $TMP_GENERATED_FILES_FILE
  echo '  DetailPrint ""' >> $TMP_GENERATED_FILES_FILE
}

#-creates the end of a section
#-must receive 2 parameters :
#  *the section identifier
#  *"yes" if we have created a directory in this section, and "no" otherwise
function end_section()
{
  section_id=$1
  we_created_directory=$2

  echo "
  SectionEnd
" >> $WIN_INSTALLER_FILE

 #end condition to uninstall directories only if the section is found
 #installed from the 'installed_sections.ini' file
 if [[ $we_created_directory = "yes" ]];then
    echo ' after_dirs_'$section_id':' >> $TMP_CREATED_DIRECTORIES_FILE
  fi

 #end condition to uninstall files only if the section is found
 #installed from the 'installed_sections.ini' file
  echo ' after_files_'$section_id':' >> $TMP_GENERATED_FILES_FILE
}

#generates the Delete file1.txt for all the installed files
# and after, RmDir dir for all the created directories
function generate_uninstall_files_dirs()
{
  #first, remove the files
  cat $TMP_GENERATED_FILES_FILE >> $WIN_INSTALLER_FILE
  #then, remove directories
  cat $TMP_CREATED_DIRECTORIES_FILE >> $WIN_INSTALLER_FILE
}

#main options
echo "!include MUI2.nsh

;program variables
!define VERSION \"$MP3SPLT_VERSION\"
!define PROGRAM_NAME \"mp3splt\"" > $WIN_INSTALLER_FILE

if [[ -z $we_dont_cross_compile ]];then
  echo "!define MP3SPLT_PATH \"c:/mp3splt_mingw`pwd`/../..\"" >> $WIN_INSTALLER_FILE
else 
  echo "!define MP3SPLT_PATH \"`pwd`/../..\"" >> $WIN_INSTALLER_FILE
fi

echo '
;name of the program
Name "mp3splt ${VERSION}"
;file to write
OutFile "../../mp3splt_'$MP3SPLT_VERSION'_'$ARCH'.exe"
;installation directory
InstallDir $PROGRAMFILES\mp3splt

BrandingText " "

;interface settings
!define MUI_ICON ${MP3SPLT_PATH}/newmp3splt/windows/mp3splt.ico
!define MUI_UNICON ${MP3SPLT_PATH}/newmp3splt/windows/mp3splt.ico

!define MUI_WELCOMEFINISHPAGE_BITMAP ${MP3SPLT_PATH}/newmp3splt/windows/mp3splt.bmp
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ${MP3SPLT_PATH}/newmp3splt/windows/mp3splt.bmp

!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN $INSTDIR\mp3splt.bat
!define MUI_FINISHPAGE_LINK "Mp3splt-project home page"
!define MUI_FINISHPAGE_LINK_LOCATION "http://mp3splt.sourceforge.net"

!define MUI_FINISHPAGE_SHOWREADME "http://mp3splt.sourceforge.net/mp3splt_page/documentation/man.html"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View online manual"

!define MUI_COMPONENTSPAGE_NODESC
ShowInstDetails "show"
ShowUninstDetails "show"
SetCompressor /SOLID "lzma"

;install pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ${MP3SPLT_PATH}\newmp3splt\COPYING
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;uninstall pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;interface languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"' >> $WIN_INSTALLER_FILE

#main section
MAIN_SECTION_FILES="
newmp3splt\src\mp3splt.exe
libmp3splt\src\.libs\libmp3splt${DLL_SUFFIX}.dll
libltdl3.dll
zlib1.dll
"

echo '
;main installation section
Section "mp3splt (with libmp3splt)" main_section
  DetailPrint ""
  DetailPrint "Installing the main section :"
  DetailPrint ""
' >> $WIN_INSTALLER_FILE

set_out_path '$INSTDIR'
copy_files $MAIN_SECTION_FILES

echo '
  FileOpen $9 mp3splt.bat w
  FileWrite $9 '\''cmd /K "cd $INSTDIR & .\mp3splt.exe"'\''
  FileClose $9

  WriteUninstaller "${PROGRAM_NAME}_uninst.exe"

  WriteRegStr HKLM "Software\${PROGRAM_NAME}\" "UninstallString" "$INSTDIR\${PROGRAM_NAME}_uninst.exe"

  ;add to Add/Remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "DisplayName" "mp3splt"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "UninstallString" "$INSTDIR\${PROGRAM_NAME}_uninst.exe"

SectionEnd' >> $WIN_INSTALLER_FILE

#mp3 plugin section
MP3_PLUGIN_FILES="
libmad-0.dll
libid3tag.dll
libmp3splt\plugins\.libs\libsplt_mp3${DLL_SUFFIX}.dll
"

echo '
;main plugins section
SubSection /e "Plugins" plugins_section' >> $WIN_INSTALLER_FILE

start_section "mp3 plugin" "mp3_plugin_section" "no"
set_out_path '$INSTDIR'
copy_files $MP3_PLUGIN_FILES
end_section "mp3_plugin_section" "no"

#ogg plugin section
OGG_PLUGIN_FILES="
libogg-0.dll
libvorbis-0.dll
libvorbisenc-2.dll
libvorbisfile-3.dll
libmp3splt\plugins\.libs\libsplt_ogg${DLL_SUFFIX}.dll
"

start_section "ogg vorbis plugin" "ogg_plugin_section" "no"
set_out_path '$INSTDIR'
copy_files $OGG_PLUGIN_FILES
end_section "ogg_plugin_section" "no"

echo 'SubSectionEnd' >> $WIN_INSTALLER_FILE

#mp3splt doc section
MP3SPLT_DOC_FILES="
newmp3splt\README
newmp3splt\doc\manual.html
newmp3splt\COPYING
newmp3splt\ChangeLog
newmp3splt\INSTALL
newmp3splt\NEWS
newmp3splt\TODO
newmp3splt\AUTHORS
"

echo '
;main documentation section
SubSection /e "Documentation" documentation_section' >> $WIN_INSTALLER_FILE

start_section "mp3splt documentation" "mp3splt_doc_section" "yes"
create_directory '$INSTDIR\mp3splt_doc'
set_out_path '$INSTDIR\mp3splt_doc'
copy_files $MP3SPLT_DOC_FILES
end_section "mp3splt_doc_section" "yes"

#libmp3splt doc section
LIBMP3SPLT_DOC_FILES="
libmp3splt\README
libmp3splt\COPYING
libmp3splt\ChangeLog
libmp3splt\INSTALL
libmp3splt\NEWS
libmp3splt\TODO
libmp3splt\AUTHORS
"

start_section "libmp3splt documentation" "libmp3splt_doc_section" "yes"
create_directory '$INSTDIR\libmp3splt_doc'
set_out_path '$INSTDIR\libmp3splt_doc'
copy_files $LIBMP3SPLT_DOC_FILES
end_section "libmp3splt_doc_section" "yes"

echo 'SubSectionEnd' >> $WIN_INSTALLER_FILE

#menu shortcuts section
echo '
;start Menu Shortcuts section
Section "Start Menu Shortcuts" menu_shortcuts_section

  DetailPrint ""
  DetailPrint "Installing the start menu shortcuts :"
  DetailPrint ""
' >> $WIN_INSTALLER_FILE

create_directory '$SMPROGRAMS\mp3splt'

echo '  CreateShortCut "$SMPROGRAMS\mp3splt\Mp3splt.lnk" "$INSTDIR\mp3splt.bat" "" ""
	CreateShortCut "$SMPROGRAMS\mp3splt\Uninstall.lnk" "$INSTDIR\mp3splt_uninst.exe" "" "$INSTDIR\mp3splt_uninst.exe" 0

  ;if mp3splt_doc_section is selected, add mp3splt_doc link
  SectionGetFlags ${mp3splt_doc_section} $0
  IntOp $1 $0 & ${SF_SELECTED}
  IntCmp 0 $1 after_doc_shortcut
  CreateShortCut "$SMPROGRAMS\mp3splt\mp3splt_doc.lnk" "$INSTDIR\mp3splt_doc" "" "$INSTDIR\mp3splt_doc" 
  after_doc_shortcut:

  ;if libmp3splt_doc_section is selected, add libmp3splt_doc link
  SectionGetFlags ${libmp3splt_doc_section} $0
  IntOp $1 $0 & ${SF_SELECTED}
  IntCmp 0 $1 after_lib_doc_shortcut
  CreateShortCut "$SMPROGRAMS\mp3splt\libmp3splt_doc.lnk" "$INSTDIR\libmp3splt_doc"	"" "$INSTDIR\libmp3splt_doc" 
  after_lib_doc_shortcut:

SectionEnd


;desktop shortcut
Section "Desktop Shortcut" desktop_shortcut_section

  DetailPrint ""
  DetailPrint "Installing the desktop shortcut :"
  DetailPrint ""
	CreateShortCut "$DESKTOP\mp3splt.lnk" "$INSTDIR\mp3splt.bat" "" ""  

SectionEnd' >> $WIN_INSTALLER_FILE

#hidden sections checking for uninstalling only installed sections
echo -n '
;write installed sections into .ini file
Section "-write installed sections into .ini file"' >> $WIN_INSTALLER_FILE

cat $TMP_CHECK_SECTIONS_UNINSTALL_FILE >> $WIN_INSTALLER_FILE

echo '
  SectionGetFlags ${menu_shortcuts_section} $0
  IntOp $1 $0 & ${SF_SELECTED}
  WriteINIStr $INSTDIR\installed_sections.ini menu_shortcuts_section "installed" $1

  SectionGetFlags ${desktop_shortcut_section} $0
  IntOp $1 $0 & ${SF_SELECTED}
  WriteINIStr $INSTDIR\installed_sections.ini desktop_shortcut_section "installed" $1' >> $WIN_INSTALLER_FILE

echo '
SectionEnd' >> $WIN_INSTALLER_FILE

#uninstallation section
echo '
;uninstallation section
Section "Uninstall"

  DetailPrint ""
  DetailPrint "Uninstalling the main files :"
  DetailPrint ""

  Delete $INSTDIR\${PROGRAM_NAME}_uninst.exe
  Delete $INSTDIR\mp3splt.bat' >> $WIN_INSTALLER_FILE

generate_uninstall_files_dirs

echo '
  ;menu shortcuts
  ReadINIStr $0 $INSTDIR\installed_sections.ini menu_shortcuts_section "installed"
  IntCmp 0 $0 after_menu_shortcuts_section
   DetailPrint ""
   DetailPrint "Uninstalling section menu_shortcuts_section :"
   DetailPrint ""

   ReadINIStr $0 $INSTDIR\installed_sections.ini mp3splt_doc_section "installed"
   IntCmp 0 $0 after_link_mp3splt_doc_section
    Delete $SMPROGRAMS\mp3splt\mp3splt_doc.lnk
   after_link_mp3splt_doc_section:

   ReadINIStr $0 $INSTDIR\installed_sections.ini mp3splt_doc_section "installed"
   IntCmp 0 $0 after_link_libmp3splt_doc_section
    Delete $SMPROGRAMS\mp3splt\libmp3splt_doc.lnk
   after_link_libmp3splt_doc_section:

   Delete $SMPROGRAMS\mp3splt\Mp3splt.lnk
   Delete $SMPROGRAMS\mp3splt\Uninstall.lnk
   RmDir $SMPROGRAMS\mp3splt
  after_menu_shortcuts_section:

  ;desktop shortcut section
  ReadINIStr $0 $INSTDIR\installed_sections.ini desktop_shortcut_section "installed"
  IntCmp 0 $0 after_desktop_shortcut_section
   DetailPrint ""
   DetailPrint "Uninstalling desktop shortcut :"
   DetailPrint ""
   Delete $DESKTOP\mp3splt.lnk
  after_desktop_shortcut_section:

  DetailPrint ""
  DetailPrint "Uninstalling remaining files & removing registry keys :"
  DetailPrint ""

  ;delete remaining ashes if possible
  Delete $INSTDIR\installed_sections.ini
  RmDir $INSTDIR

  ;delete registry
  DeleteRegKey HKLM Software\${PROGRAM_NAME}
  DeleteRegKey HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}

  DetailPrint ""

SectionEnd' >> $WIN_INSTALLER_FILE


#uninstall old program if found installed
echo '
;uninstall the old program if necessary
Function .onInit

  ;read only and select the main section
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${main_section} $0

  ;read from registry eventual uninstall string
  ReadRegStr $R0 HKLM "Software\${PROGRAM_NAME}\" "UninstallString"
  StrCmp $R0 "" done
 
  ;uninstall previous installation
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "${PROGRAM_NAME} is already installed. $\n$\nClick `OK` to start the uninstall program of the previous installation.$\n$\n" \
  IDOK uninst
  Abort

  ;run the uninstaller
  uninst:
    ClearErrors
    ExecWait '\''$R0 _?=$INSTDIR'\''
    IfErrors no_remove_uninstaller
    no_remove_uninstaller:
  done:

FunctionEnd
' >> $WIN_INSTALLER_FILE

rm -f $TMP_GENERATED_FILES_FILE
rm -f $TMP_CREATED_DIRECTORIES_FILE
rm -f $TMP_CHECK_SECTIONS_UNINSTALL_FILE

if [[ -z $we_dont_cross_compile ]];then
  ../../../nsis/makensis -V3 win32_installer.nsi || exit 1
else
  makensis -V3 win32_installer.nsi || exit 1
fi

#remove '.nsi' script
#rm -f $WIN_INSTALLER_FILE
