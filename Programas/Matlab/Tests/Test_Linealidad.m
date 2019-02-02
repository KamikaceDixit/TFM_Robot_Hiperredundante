alpha=[30 0 0 0 0 0 0 0];
beta=[0 0 0 0 0 0 0 0];
angulo=0:0.5:30;
indice=1;
Datos=zeros(size(angulo,2),3);

for i=angulo
Distancias=Calc_Distancia(1,Calc_Posiciones(alpha,[i 0 0 0 0 0 0 0]));
Datos(indice,:)=Distancias([1 13 25]);
indice=indice+1;
end
Datos=Datos-50;

figure
hold on;
title("Variación Longitudes")
xlabel("Ángulo (deg)")
ylabel("Incremento Longitud (mm)")
plot(angulo,Datos(:,1));
plot(angulo,Datos(:,2));
plot(angulo,Datos(:,3));


Lineal_A=linspace(Datos(1,1),Datos(size(Datos,1),1),size(Datos,1))';
Lineal_B=linspace(Datos(1,2),Datos(size(Datos,1),2),size(Datos,1))';
Lineal_C=linspace(Datos(1,3),Datos(size(Datos,1),3),size(Datos,1))';
plot(angulo,Lineal_A);
plot(angulo,Lineal_B);
plot(angulo,Lineal_C);
legend("Cable 1", "Cable 13", "Cable 25") 


figure
hold on
title("Error de linealización")
xlabel("Ángulo (deg)")
ylabel("Error (mm)")
plot((Datos(:,1)-Lineal_A))
plot((Datos(:,2)-Lineal_B))
plot((Datos(:,3)-Lineal_C))
legend("Cable 1", "Cable 13", "Cable 25") 
