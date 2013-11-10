% List recordings
function lsrecvis(varargin)
defaults=struct('minocc',1,'maxocc',9999,'minframes',1);
args=processargs(defaults,varargin);
files=dir([pfroot(),'/Recordings/*.mat']);
for i=1:length(files)
  f=files(i);
  zz=load([pfroot(),'/Recordings/',f.name],'note');
  if ~isfield(zz,'note')
    system(sprintf('mv %s/Recordings/%s %s/Recordings/BAD',pfroot(),f.name,pfroot()));
    fprintf('Moved %s to BAD\n', f.name);
  end
  if ~isfield(zz,'note')
    zz.note='(empty)';
  end
  if ~isfield(zz,'vis')
    zz.vis=[];
  end
  v=regexp(zz.note,'^.* of ([0-9]*) frames at .* with max occupancy of ([0-9]*)$','tokens');
  if length(v)<1 || length(v{1})~=2
    fprintf('Unable to parse note\n');
    if length(v)>=1
      disp(v{1});
    end
  else
    nframes=str2num(v{1}{1});
    occ=str2num(v{1}{2});
    if occ<args.minocc || occ>args.maxocc || nframes<args.minframes
      continue;
    end
  end
  fprintf('%s %s %4.1fMB %s\n', f.name, f.date, round(f.bytes/1024/1024), zz.note);
end