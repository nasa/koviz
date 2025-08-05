Purpose: How to deploy koviz on Windows

A. Build koviz using Qt Creator

B. Deploy koviz using windeployqt

   1. I think windeployqt comes with Qt
      See: /c/Qt/5.15.2/mingw81_64/bin/windeployqt
   2. Put the windeployqt application
      in the bash PATH
   3. In a gitbash terminal
        a. mkdir -p ~/deploy/koviz
        b. cp <koviz>/bin/koviz.exe ~/deploy/koviz
        c. windeployqt ~/deploy/koviz/koviz.exe
           This will put dependencies (e.g.
           all the *.dlls) in ~/deploy/koviz
        d. cp <koviz>/win/mpv/libmpv-2.dll ~/deploy/koviz

C. Create koviz-installer.exe using Inno Setup

   1. cp <koviz>/win/icon/koviz.ico ~/deploy/koviz
   2. cp <koviz>/win/koviz.iss ~/deploy
   3. cp <koviz>/LICENSE ~/deploy/koviz
   4. vi ~/deploy/koviz/koviz.iss
      Change deploy path to deploy.<xx>
          * Change LicenseFile 
          * Change SetupIconFile
          * Change Source path 
   5. Launch Inno Setup
      Double click Inno Setup icon
   6. Inno:File->Open ~/deploy/koviz.iss
   7. Select Inno->Build->Compile
   8. See ~/deploy/Output/koviz-installer.exe
