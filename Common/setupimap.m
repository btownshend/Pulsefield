%setupimap - setup mapping from physical coords to image coords for ray-tracing
function imap=setupimap(layout,isize)
imap=struct('isize',isize);
if length(imap.isize)==1
  % Equalize pixel count
  imap.isize(2)=imap.isize(1);
end
all=[layout.cpos;layout.lpos;layout.active];
imap.scale=isize ./max(max(all)-min(all));
imap.origin=min(all);
