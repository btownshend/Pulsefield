% Generate notes in a scale
% See http://8.brm.sk/Scales_Chords.pdf
function notes=makescale(scale,key,octaves)
scales={'major',[0,2,4,5,7,9,11],
        'minor',[0,2,3,5,7,8,10],
        'blues',[0,3,5,6,7,10]};
keys='C  C# Db D  D# Eb E  F  F# Gb G  G# Ab A  A# Bb B  B# Cb ';
keyv=[0  1  1  2  3  3  4  5  6  6  7  8  8  9  10 10 11 12 12]+60;
scpitches=[];
notes=[];
for i=1:size(scales,1)
  if strcmp(scale,scales{i,1})
    scpitches=scales{i,2};
    break;
  end
end
if isempty(scpitches)
  fprintf('Unknown scale: %s\n', scale);
  fprintf('Supported scales: %s\n', sprintf('%s ',scales{:,1}));
  return;
end
key=upper([key,'  ']);
key=key(1:3);
kpos=strfind(keys,key);
if length(kpos)~=1
  fprintf('Unknown key: %s\n', key);
  fprintf('Support keys: %s\n', keys);
  return;
end
cnotes=keyv((kpos+2)/3)+scpitches;
nbelow=ceil((octaves-1)/2);
nabove=octaves-nbelow-1;
for i=nbelow:-1:1
  notes=[notes,cnotes-12*i];
end
for i=0:nabove
  notes=[notes,cnotes+12*i];
end