function errvec=sterroptfn(rpts,lpts,deltarot)
delta=deltarot(1:3);
rot=deltarot(4:6);
phi=rodrigues(rot);
ptdif=rpts-(phi*lpts+repmat(delta,1,size(lpts,2)));
errvec=sqrt(sum(ptdif.^2));

