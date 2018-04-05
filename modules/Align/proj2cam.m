function [cx,cy]=proj2cam(pmap,px,py)
cx=interp2(pmap.cx,pmap.cy,pmap.centroid(:,:,1),px,py);
cy=interp2(pmap.cx,pmap.cy,pmap.centroid(:,:,2),px,py);
