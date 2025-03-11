# Introduction
A sound visualisation project made in early 2023. I wanted to learn about how FFT works in order to do some analysis on the audio, such as extracting stems, spectral editing and detecting some features of the audio. I moved on with other projects and I did not achieve these goals, but I still learned some things about FFT and I could pick this idea up in the future. The code is public for archival purposes, maybe it's of use to someone.

The program is only in Romanian, as I made it for myself. (sorry)

# Usage
Open a WAV file with the program (drag the file onto it in Explorer) in order to play it. If you see flashing blue text it means that you didn't load a file.

A precompiled version of the program for Windows is available in the Releases section.

# Features
Displays an FFT spectogram which can be manipulated with the following keybinds:
* S: add offset
* W: decrease offset
* D: increase width
* A: decrease width
* F: toggle view mode (default is FFT, secondary is chunks, it's buggy)
* Space: pause
* Arrow keys: ...

# Compilation
No build system is provided. Link/include these libraries with the build system of your choice:
* SDL2-2.26.1
* SDL2_ttf-2.20.1
* fftw-3.3.5
* portaudio
* sndfile

# Contact
If someone would like to continue to work on this project or another one, please contact me at the e-mail address on my profile!

# Acknowledgments
* Lazy Foo's SDL2 tutorials: https://lazyfoo.net/tutorials/SDL/
* Titillium font designed by Accademia di Belle Arti di Urbino
* The ARSS for inspiring me to learn more about FFT: https://arss.sourceforge.net/