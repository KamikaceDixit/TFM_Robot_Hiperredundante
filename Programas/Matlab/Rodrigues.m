function R=Rodrigues(k,alfa)
%k=k/norm(k);
alfa=deg2rad(alfa);

kx=k(1); ky=k(2); kz=k(3);
K=   [0 -kz ky;
      kz 0 -kx;
     -ky kx 0];
%R=cos(alfa)*eye(3)+(1-cos(alfa))*(k'*k)+sin(alfa)*K;
R=eye(3)+sin(alfa)*K+(1-cos(alfa))*K*K;
end