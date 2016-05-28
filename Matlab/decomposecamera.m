% Decompose a camera view matrix to [eye,aim,up] vectors
function [eye,aim,up]=decomposecamera(c)
untrans=c; untrans(1:3,4)=0;
tmat=inv(untrans)*c;
eye=tmat(1:3,4);
aim=-c(3,1:3);
up=c(2,1:3);
