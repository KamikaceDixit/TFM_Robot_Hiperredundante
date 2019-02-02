function [Polinomio] = Poli1_Pos(t0,x_0,v_0)
syms t;

A=[t0 1 ;
   1 0];
%X=[a1; b1;];
B=[x_0 v_0];
detA=det(A);
A=sym(A);

A1=A;
A1(:,1)=B;
A2=A;
A2(:,2)=B;

a1=det(A1)/detA; 
b1=det(A2)/detA;

Polinomio=a1*t+b1;

end
