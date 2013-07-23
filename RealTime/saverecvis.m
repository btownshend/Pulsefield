function saverecvis(recvis,note)
if nargin<1
  error('Usage:  saverecvis(recvis [,note])');
end
if nargin<2
  recvis.note=input('Enter comment for save file (or return to skip save): ','s');
else
  recvis.note=note;
end
if ~isempty(recvis.note)
  recname=sprintf('%s/Recordings/%s.mat',pfroot(),datestr(recvis.vis(1).when,30));
  fprintf('Saving recording in %s...', recname);
  if isfield(recvis.vis,'im')
    recvistmp=rmfield(recvis,'vis');
    recvistmp.vis=rmfield(recvis.vis,'im');
    save(recname,'-struct','recvistmp');
  else
    save(recname,'-struct','recvis');
  end
  fprintf('done\n');
end
