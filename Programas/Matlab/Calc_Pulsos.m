function [Pulsos_Destino] = Calc_Pulsos(Puntos_Moved_Red)
%Par�metros de dise�o
R=50;   %mm Radio al punto medio de los agujeros gu�a de los cables.
H=50;   %mm Distancia entre placas de secci�n articulada.
Num_Agujeros=36; %Agujeros distribuidos uniformemente en el disco.
Num_Rotulas=8;
Paso_husillo=4;%mm
Pulsos_por_rev=44;
Posicion_eje_y=27.5;    % Ubicaci�n del eje y con respecto a la numeraci�n 
                        % de los agujeros, en este casos se encuentra entre 
                        % el agujero 27 y el 28.
                      

Longitudes_cables=zeros(1,Num_Agujeros);
Longitudes_cables_0=[50 350 0 150 250 0 100 400 0 200 300 0 50 350 0 150 250 0 100 400 0 200 300 0 50 350 0 150 250 0 100 400 0 200 300 0];
Geomatriz=zeros(Num_Rotulas,8,Num_Agujeros);
Reductora_Motor= [51,27,0,27,27, 0,51,27,0,27, 27,0,51,27,0, 27,27,0,51,27, 0,27,27,0,51, 27,0,27,27,0, 51,27,0,27,27, 0 ];		


Geomatriz(1:Num_Rotulas,1)=1:Num_Rotulas;   % Definici�n n�mero de r�tula.
% Definici�n de cables asociados a cada r�tula.
Geomatriz(1,2:4)=[1,13,25];
Geomatriz(2,2:4)=[7,19,31];
Geomatriz(3,2:4)=[4,16,28];
Geomatriz(4,2:4)=[10,22,34];
Geomatriz(5,2:4)=[5,17,29];
Geomatriz(6,2:4)=[11,23,35];
Geomatriz(7,2:4)=[2,14,26];
Geomatriz(8,2:4)=[8,20,32];


% Geomatriz (1       , 2      , 3      , 4      , 5 , 6 , 7 , 8 , 9 , 10) 
% Geomatriz (n�r�tula, cable_1, cable_2, cable_3, q0, q1, q2, d1, d2, d3)
% q1 y q2 van a ser direccion de maxima pendiente y angulo girado
% respectivamente.



% Generamos q0 %%No se que sentido tiene q0 ahora mismo REVISARRR
for i=1:Num_Rotulas
Geomatriz(i,5,1)=-(Num_Agujeros-Posicion_eje_y + Geomatriz(i,2,1))*10; 
% 36 cables, eje y centrado en 
% cable 27.5, sumado posicion del primer cable de cada r�tula, cables
% desfasados 10�.
end


%Puntos_Moved_Red = Calc_Posiciones(alpha,beta);
% C�lculo de las distancias en funci�n de los �ngulos de cada secci�n.
for i=1:Num_Rotulas
Geomatriz(i,8,:)=Calc_Distancia(i,Puntos_Moved_Red); % 
end

% Sumatorio de las distancias totales de todos los cables.
% Con esto conseguimos sumar la longitud de cada tr�o de cables teniendo en
% cuenta en qu� r�tula empiezan.
for i=8:-1:1    % Para cada r�tula empezando desde la pinza.
    for j=Geomatriz(i,2:4,1)   % Para cada cable en esa r�tula.
        for k=1:i   %Sumamos la longitud de todas las r�tulas.
            Longitudes_cables(j) = Longitudes_cables(j) + Geomatriz(k,8,j);
        end
    end
end

Desplazamiento_cables=Longitudes_cables-Longitudes_cables_0;
Pulsos_Destino=-round(Desplazamiento_cables*(Pulsos_por_rev/Paso_husillo).*Reductora_Motor);
clear alpha beta H i j k Longitudes_cables Longitudes_cables_0 Num_Agujeros Num_Rotulas Posicion_eje_y Puntos_Moved_Red R
end

