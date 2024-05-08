# fabscroll_v1_010
 esp32 p10 display + remote fetch text

for copenhagen fablab 2024

readme/notes for using the p10 led matrix scroller

Basic functionality:
Connect to local wifi
Fetch 2 remote .txt files
Display as scrolling text on led panel
Periodically check for updates to the textfiles
Display the new text if/when received

A default and a temporary text is used. The default text is for basic information/text that rarely changes. The temporary text runs on a builtin timer, meaning it expires automatically after eg. 24h.


Textfiles should just be plain ascii .txt files, optionally utf8 with bom. Only standard ascii characters should be used in the text files. The text currently uses the standard font from the Adafruit_GFX library,  glcdfont.