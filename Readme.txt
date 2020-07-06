gkrellm-uguru for GKrellM2  

From  Tobias van Dyk.  
2005:
Because lm_sensors does not support the Winbond monitor chip (uGuru) used on some Abit motherboards here is a gkrellm plugin that provides some of the monitoring functions 

Note that Hans De Goede wrote an Uguru kernel module during 2006

See: 

https://github.com/billw2/billw2.github.io

http://gkrellm.srcbox.net/Plugins.html

To add to this blast from the past here is an image dump of the now sadly demised thinkblog.org of 14 July 2006 to 4 April 2006. I addition to the mention of my uguru plug-in this page captures a few other much more noteworhy events such as:

01 June 2006
Ubuntu 6.06 Released
06:12:36 :: [Linux] :: 104 words 
Ubuntu is the new, up-and-coming Linux distribution based on the rock-solid foundation of Debian/GNU Linux. See screenshots here.
You can order CDs of it for free, or download the ISOs via BitTorrent—legally, thankyouverymuch— and create your own discs (CDs or DVDs).
I’m looking toward Gentoo personally, but if you’re unfamiliar with Linux, this is reputed to be the best option, and easier to install than even Windows XP!
Happy installs!

19 June 2006
Bill Gates Stepping Down
22:53:18 :: [psychology, technology &c., Linux] :: 1366 words 
BillG YoungMost of you who follow this sort of thing know by now that Bill Gates is reversing his roles between Microsoft and the Bill & Melinda Gates Foundation; in his words, “Right now and for the next two years, my full-time job is here at Microsoft, and my part-time job remains the Foundation. Beginning in July 2008, I will switch that, to be full-time at the Foundation, while remaining involved with Microsoft as Chairman and an advisor on key development projects on a part-time basis.”
NeoWin.net has the full email, which is echoed in blockquote at the end of this article (if you’re on the main page, click “more…” at the bottom of the post).

04 April 2006
Strides Toward KDE/GNOME Compatibility
23:16:48 :: [Linux] :: 244 words 
In Geekdom, there are perennial debates that carry as much weight as religious discussion: Emacs versus vi, Linux versus BSD, and so forth. One of these debates concerns the desktop platforms of choice. I use and prefer KDE, but GNOME has long been a favorite among Linux users for its sleek interface.

and the uguru announcement:

14 March 2006
uGuru GKrellM2 Monitor
02:03:46 :: [Linux] :: 270 words 
If you own an Abit motherboard and are a Linux user, you are familiar with the frustration that the proprietary Abit-modified Winbond chip that they’ve dubbed “µGuru” (commonly uGuru, but pronounced and spelled with a Greek “mu,” “mu-Guru”) is unsupported under Linux. Hans de Goede created a program based on Olle Sandberg’s original program to interface with this chip.
Olle’s “oguru” program [link to actual program here] works. Unfortunately, it’s only a once-invoked, stdout-based commandline version.
Tobias van Dyk, on the other hand, created a plugin for GKrellM2 based on oguru that updates the CPU, system, and power supply temperature and shows the voltages of several key monitors by interfacing with the chip and displaying this information in GKrellM2. You can find this plugin here.
When compiling, if you’re on an x86_64 machine like I am, you’re going to need to update the line #include <asm/io.h> to say, #include <sys/io.h> as on Athlon 64 machines, calls to either asm-i386 or asm-x86_64 as such are deprecated (and asm alone isn’t defined at all!).
Note that in order to run this plugin without a segfault from GKrellM2, you’re going to need to run it as root. 

<br>
<p align="center">
<img src="Thinkblog.png" width="840" />  
<br>



