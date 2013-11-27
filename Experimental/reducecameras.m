% Reduce the number of cameras in a recvis structure
% Useful for testing replays with a subset of the cameras
% Usage: recvis=reducecameras(recvis,cameras)
%    cameras is a list of camera indices to keep
function recvis=reducecameras(recvis,cameras)
recvis.p.camera=recvis.p.camera(cameras);
recvis.p.layout.cpos=recvis.p.layout.cpos(cameras,:);
recvis.p.layout.cdir=recvis.p.layout.cdir(cameras,:);
for i=1:length(recvis.vis)
  recvis.vis(i).cframe=recvis.vis(i).cframe(cameras);
  recvis.vis(i).acquired=recvis.vis(i).acquired(cameras);
  recvis.vis(i).v=recvis.vis(i).v(cameras,:);
end
if isfield(recvis.p,'rays')
  recvis.p.rays.rays=recvis.p.rays{cameras};
  recvis.p.nzrayindices=recvis.p.nzrayindices{cameras};
  recvis.p.nzraymap=recvis.p.nzraymap{cameras};
  recvis.p.raylines=recvis.p.raylines{cameras,:};
end
recvis.note=sprintf('%s (Reduced to cameras %s)',recvis.note,sprintf('%d ',cameras));