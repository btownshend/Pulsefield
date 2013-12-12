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
    for i=1:length(recvistmp.p.camera)
      recvistmp.p.camera(i).extcal= ...
          rmfield(recvistmp.camera(i).extcal,'image');
    end
    recvistmp.vis=rmfield(recvis.vis,'im');
    save(recname,'-struct','recvistmp');
  else
    if isfield(recvis.p.camera(1).extcal,'image')
      for i=1:length(recvis.p.camera)
        recvis.p.camera(i).extcal= ...
            rmfield(recvis.p.camera(i).extcal,'image');
      end
    end
%    if isfield(recvis.p,'rays')
%      recvis.p=rmfield(recvis.p,'rays');
%    end
    save(recname,'-struct','recvis');
  end
  fprintf('done\n');
end
