# clip
Clip - an MP3 clipper. Clip clips out a number of bytes from the middle of a file. It is targeted for clipping out sections of larger MP3 files. 

Background

Back in 2001, I wanted to clip out specific songs from 2-hour long MP3 streams of DJ Dust shows. Audacity or similar tools were just not capable of doing that with the typical memory available on workstations at that time. We didn´t find any command line tool doing what I needed either, so Jan Kroken helped me write the first version of Clip.

Since then, Clip has been overtaken by more versatile audio clipping tools like Mp3Splt, but I keep clip around for nostalgic reasons. Besides, I know exactly how it works.
How to use

Run clip [-b ] offset[K|M|s] length[K|M|s]|´end´ infile.mp3 outfile.mp3.
Clip will copy the middle of infile.mp3 and save it to outfile.mp3. Simple! 
