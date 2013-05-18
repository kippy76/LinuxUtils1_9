Linux Utils 1.6
(C) 2011 Christopher Walker

About this program...

I made this C++ program to provide a quick panel-accessed utility that would allow crude colour desktop colour temperature adjustments. I use my laptop a lot, and it has no monitor colour adjustments, so I made this utility because I couldn't find anything appropriate.

Since I was writing it, I thought I'd create a menu system and drop in lots of features I do daily, and included a simple file cleanup system. Version 1.5 clears out the following items :

>> Global recently used list
>> Multimedia recently used list
>> Bash command history
>> User trash can
>> System general cache
>> Thumbnail cache

Of course, you can add more, there's a little room I think.

The colour temperature adjustment screen has 4 sliders that allow manual adjustment of RGB and gamma settings. The program will automatically save the settings and load the settings without user intervention. There are also 4 presets, D93, D65, D50 and flat reset. These values are calibrated for my LCD screen, and you will need to amend the source code with settings that suit your hardware. There's plenty of comments in the source that should help you, and all global variables are clearly listed at the top for easy access.

One caveat, the program was a quick knock together to get the job done, and most of the code can certainly be improved, so feel free. Similarly, the sound module was cobbled together in 1 draft, so you can almost certainly improve on it!

One last note, to enable startup correction of colour, the program supports silent command-line options for applying colour and cleaning out the files. Execute the program via 'LinuxUtils1_5 -help' to see the options available.

I hope you find this program useful, and if you write some cool utilities to occupy the remaining 2 menu spaces, I'd love to receive a copy to see what you've done. Enjoy, and feel free to issue copies to your friends if they want it. The program has been issued under GPL GNU version 3 license, a copy of which is enclosed, so please ensure that if you supply a copy to someone, you send them the original archive that includes all the files and a copy of the license. Many thanks.

C Walker.
