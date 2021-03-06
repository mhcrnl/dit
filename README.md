<p align="center"><a href="http://hisham.hm/dit"><img border="0" src="http://hisham.hm/dit/dit-white.jpg" alt="Dit"></a></p>

A console text editor for Unix systems that you already know how to use.

http://hisham.hm/dit

Quick reference
---------------

* Ctrl+Q or F10 - quit
* Ctrl+S - save
* Ctrl+X - cut
* Ctrl+C - copy
* Ctrl+V - paste
* Ctrl+Z or Ctrl+U - undo
* Shift-arrows or Alt-arrows - select
  * NOTE! Some terminals "capture" shift-arrow movement for other purposes (switching tabs, etc) and Dit never gets the keys, that's why Dit also tries to support Alt-arrows. Try both and see which one works. If Shift-arrows don't work I recommend you reconfigure your terminal (you can do this in Konsole setting "Previous Tab" and "Next Tab" to alt-left and alt-right, for example). RXVT-Unicode and Terminology are some terminals that work well out-of-the-box.
* Ctrl+F or F3 - find. Inside Find:
  * Ctrl+C - toggle case sensitive
  * Ctrl+W - toggle whole word
  * Ctrl+N - next
  * Ctrl+P - previous
  * Ctrl+R - replace
  * Enter - "confirm": exit Find staying where you are
  * Esc - "cancel": exit Find returning to where you started
    * This is useful for "peeking into another part of the file": just Ctrl+F, type something to look, and then Esc to go back to where you were.
* Ctrl+G - go to line
* Ctrl+B - back (to previous location, before last find, go-to-line, tab-switch, etc.)
  * You can press Ctrl+B multiple times to go back various levels.
* Tabs of open files:
  * Ctrl+J - previous tab
  * Ctrl+K - next tab
  * Ctrl+W - close tab
* Ctrl+N - word wrap paragraph
* Ctrl+T - toggle tab mode (Tabs, 2, 3, 4 or 8 spaces - it tries to autodetect based on file contents)

This documentation is incomplete... there are [more keys](https://github.com/hishamhm/dit/blob/master/bindings/default)! Try around!

