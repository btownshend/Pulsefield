% Make a set of movies from all files matching basename
function makemovies(basename)
[path,name,ext]=fileparts(basename);
d=dir(sprintf('%s/%s*.mat',path,name));
if isempty(d)
  error('%s not found\n',basename);
end
for i=1:length(d)
  data=load(sprintf('%s/%s',path,d(i).name));
  avi=sprintf('%s/%s',path,strrep(d(i).name,'.mat','.avi'));
  if exist(avi,'file')
    fprintf('%s already exists; skipping\n',avi);
    continue;
  end
  fprintf('Converting %s: %s\n', d(i).name, sprintf('%s ',data.frontend.args{:}));
  snapmovie(data.csnap,'maxrange',12,'height',480,'width',640,'filename',avi);
end
