function im=acquire(p,n)
tic;
for k=1:n
  %  vis=rcvr(p,'stats',true);
  %im{k}=vis.im;
  if isfield(p.camera,'roi')
    im{k}=aremulti(1:6,'av10115-half',{p.camera.roi});
  else
    im{k}=aremulti(1:6,'av10115-half');
  end
end
el=toc;
fprintf('Read %d frames in %.1f seconds - frame rate = %.2f/s\n', n, el, n/el);
