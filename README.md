# PIB-PA: Einführung in die Demoszene

<img width="1919" height="1079" alt="screenshot_login" src="https://github.com/user-attachments/assets/684ec2b7-d466-4e8d-9222-69d0584b56ba" />

<img width="1919" height="1079" alt="screenshot_terminal" src="https://github.com/user-attachments/assets/522776fa-4ce8-4d9d-8817-51f985c8a77c" />

<img width="1919" height="1079" alt="screenshot_karte_deutschland" src="https://github.com/user-attachments/assets/598869af-db2b-40b8-a22f-c14f8f49322f" />


## Beschreibung
Diese Software ist als Teil einer Projektarbeit im Rahmen des Kurses PIB-PA der htw saar entwickelt worden.
Sie beinhaltet eine selbst entwickelte Demo, die sich im Bereich der Demoszene bewegt.
Innerhalb der Demo wird ein Retro-Terminal sowie der Start einer Datei innerhalb dieses Terminals simuliert.
Die Demo besteht dabei aus den folgenden Komponenten:
- Ein Login-Screen, der die Authentifizierung eines Benutzers simuliert.
- Ein Terminal-Screen, der den Start von C++-Code innerhalb des Retro-Terminals simuliert.
- Eine Karten-Sequenz, in der die Ortung verschiedener vorgegebener Koordinaten simuliert wird.
Unterbrochen wird dieser Szenen-Ablauf durch den Kollaps und Wiederaufbau des Bildschirms. Dies passiert sowohl zwischen dem
Terminal-Screen und der Karten-Sequenz als auch nach Ablauf der Karten-Sequenz, worauf ein erneuter Durchgang der Demo folgt.

## Benötigte Software
Zum Ausführen dieser Demo wird Visual C++ 2013 Redistributable benötigt.
Dies kann unter https://www.microsoft.com/en-us/download/details.aspx?id=40784 heruntergeladen werden.
Dabei müssen die x86- und x64-Versionen installiert werden.

## Config-Datei
Die Demo beinhaltet eine Config-Datei, mit der die Bildschirmauflösung und Vollbildmodus konfiguriert werden können. Die Datei hat das folgende Format:

```ini
[Window]
Width=1920
Height=1080
Fullscreen=false
```

Die Werte können nach Bedarf angepasst werden. Das Programm muss dazu nicht neu kompiliert werden. Die Datei befindet sich im selben Verzeichnis wie die start.bat.

## Kompilieranleitung
Die Demo kommt standardmäßig mit einer kompilierten Version der Anwendung. 
Zur Kompilierung der Demo wird die MSYS2 MinGW64-Umgebung benötigt. Die folgenden Schritte sind erforderlich:
1.  Installieren Sie MSYS2 von https://www.msys2.org/
2.  Öffnen Sie die MSYS2 MinGW64-Shell.
3.  Aktualisieren Sie die Paketdatenbank und installieren Sie die benötigten Pakete:´
    - pacman -Syu
    - pacman -S --needed mingw-w64-x86_64-toolchain
    - pacman -S mingw-w64-x86_64-cmake
    - pacman -S mingw-w64-x86_64-glfw
    - pacman -S mingw-w64-x86_64-freetype
    - pacman -S mingw-w64-x86_64-glm
    - pacman -S mingw-w64-x86_64-sfml
4. Klonen Sie das Repository und wechseln Sie in das Verzeichnis "pa_demoszene"
5. Kompilierung des Projekts:
    - mkdir build
    - cd build
    - cmake -G "MinGW Makefiles" ..
    - mingw32-make
6. Wechseln Sie zurück in das Oberverzeichnis durch "cd .."
7. Nach der Kompilierung finden Sie die ausführbare Datei im Verzeichnis "pa_demoszene".
8. Starten der Anwendung entweder durch Ausführen der beiliegenden start.bat oder durch Eingabe des Befehls ./RetroTerminal in der MinGW64-Shell.
