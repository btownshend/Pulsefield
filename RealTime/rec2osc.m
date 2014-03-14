% Replay a record session and capture the OSC
global recvis
%basename='20130901T015637';
%basename='20130826T235212';
%basename='20130831T080408';
basename='20131212T181759';
dir=basename(1:6);
recvis=load(['../../Recordings/',dir,'/',basename,'.mat']);
cmd=sprintf('../../OSC/recordOSC/recordOSC 7002 >../../OSCRecordings/%s.osc &',basename);
[s,r]=system(cmd);
if s~=0
  error(r);
end
replayrecvis=replay()


