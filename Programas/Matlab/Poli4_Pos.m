function [Polinomio] = Poli4_Pos(t0,t1,x_0,v_0,v_1,a_0,a_1)

syms  t ;

A=[t0^4 t0^3 t0^2 t0 1 ;
   4*t0^3 3*t0^2 2*t0 1 0;
   4*t1^3 3*t1^2 2*t1 1 0
   12*t0^2 6*t0 2 0 0;
   12*t1^2 6*t1 2 0 0;];
%X=[a1; b1; c1; d1; e1];
A=sym(A); %Conversion a simbolico
B=[x_0 v_0 v_1 a_0 a_1];
detA=det(A);

A1=A; A1(:,1)=B;
A2=A; A2(:,2)=B;
A3=A; A3(:,3)=B;
A4=A; A4(:,4)=B;
A5=A; A5(:,5)=B;

a1=det(A1)/detA;
b1=det(A2)/detA;
c1=det(A3)/detA;
d1=det(A4)/detA;
e1=det(A5)/detA;

Polinomio=a1*t^4+b1*t^3+c1*t^2+d1*t + e1;
end


