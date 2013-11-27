function errorvec=phierror(N1,N2,phi)

phi=rodrigues(phi);

errorvec=dot(phi*N1,N2)./(sqrt(sum(N1.^2)).*sqrt(sum(N2.^2)))-1;