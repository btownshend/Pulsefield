dir='/Volumes/Media/2014/140215 Co-related Prototyping'
file='MVI_2868.MOV';
recvisfile='20140215T155831.mat';
oh=loadoh([dir,'/',file],datenum('2/15/14 16:06:06'),recvisfile);
oh.cameralate=12;
overlay(oh,recvis);
