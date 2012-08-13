% Generate notes in a scale
% See http://8.brm.sk/Scales_Chords.pdf
function notes=makescale(scale,key,numnotes)
scales=getscales();
keys='C  C# Db D  D# Eb E  F  F# Gb G  G# Ab A  A# Bb B  B# Cb ';
keyv=[0  1  1  2  3  3  4  5  6  6  7  8  8  9  10 10 11 12 12]+60;
scpitches=[];
notes=[];
snum=find(strcmpi(scale,{scales.name}));
if isempty(snum)
  fprintf('Unknown scale: %s\n', scale);
  fprintf('Supported scales: %s\n', sprintf('%s ',scales{:,1}));
  return;
end
scpitches=scales(snum(1)).pattern;
key=upper([key,'  ']);
key=key(1:3);
kpos=strfind(keys,key);
if length(kpos)~=1
  fprintf('Unknown key: %s\n', key);
  fprintf('Support keys: %s\n', keys);
  return;
end
cnotes=keyv((kpos+2)/3)+scpitches;
notes=[];
for i=1:numnotes
  notes(i)=cnotes(mod(i-1,length(cnotes))+1)+12*floor((i-1)/length(cnotes));
end  
% Shift to put middle C as close as possible to center
notes=notes-round((median(notes)-60)/12)*12;
