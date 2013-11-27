function errorvec=deltaerror(N1,N2,delta)

errorvec=delta'*N2./(sqrt(sum(N2.^2)).*(sqrt(sum(N2.^2))-sqrt(sum(N1.^2))))-1;
% error=sqrt(sum(errorvec.^2)/length(errorvec));