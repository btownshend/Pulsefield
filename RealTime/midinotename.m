% Convert midi note numbers to names
function note=midinotename(num)
notes={'C','C#','D','D#','E','F','F#','G','G#','A','A#','B'};
octave=floor(num/12)-1;
nm=notes{mod(num,12)+1};
if octave<0
  note=sprintf('%s(%d)',nm,octave);
else
  note=sprintf('%s%d',nm,octave);
end
