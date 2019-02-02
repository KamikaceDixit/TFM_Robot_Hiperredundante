function [Pulsos_Destino] = Calc_Pulsos(Puntos_Moved_Red)
%Parámetros de diseño
R=50;   %mm Radio al punto medio de los agujeros guía de los cables.
H=50;   %mm Distancia entre placas de sección articulada.
Num_Agujeros=36; %Agujeros distribuidos uniformemente en el disco.
Num_Rotulas=8;
Paso_husillo=4;%mm
Pulsos_por_rev=44;
Posicion_eje_y=27.5;    % Ubicación del eje y con respecto a la numeración 
                        % de los agujeros, en este casos se encuentra entre 
                        % el agujero 27 y el 28.
                      

Longitudes_cables=zeros(1,Num_Agujeros);
Longitudes_cables_0=[50 350 0 150 250 0 100 400 0 200 300 0 50 350 0 150 250 0 100 400 0 200 300 0 50 350 0 150 250 0 100 400 0 200 300 0];
Geomatriz=zeros(Num_Rotulas,8,Num_Agujeros);
Reductora_Motor= [51,27,0,27,27, 0,51,27,0,27, 27,0,51,27,0, 27,27,0,51,27, 0,27,27,0,51, 27,0,27,27,0, 51,27,0,27,27, 0 ];		


Geomatriz(1:Num_Rotulas,1)=1:Num_Rotulas;   % Definición número de rótula.
% Definición de cables asociados a cada rótula.
Geomatriz(1,2:4)=[1,13,25];
Geomatriz(2,2:4)=[7,19,31];
Geomatriz(3,2:4)=[4,16,28];
Geomatriz(4,2:4)=[10,22,34];
Geomatriz(5,2:4)=[5,17,29];
Geomatriz(6,2:4)=[11,23,35];
Geomatriz(7,2:4)=[2,14,26];
Geomatriz(8,2:4)=[8,20,32];


% Geomatriz (1       , 2      , 3      , 4      , 5 , 6 , 7 , 8 , 9 , 10) 
% Geomatriz (nºrótula, cable_1, cable_2, cable_3, q0, q1, q2, d1, d2, d3)
% q1 y q2 van a ser direccion de maxima pendiente y angulo girado
% respectivamente.



% Generamos q0 %%No se que sentido tiene q0 ahora mismo REVISARRR
for i=1:Num_Rotulas
Geomatriz(i,5,1)=-(Num_Agujeros-Posicion_eje_y + Geomatriz(i,2,1))*10; 
% 36 cables, eje y centrado en 
% cable 27.5, sumado posicion del primer cable de cada rótula, cables
% desfasados 10º.
end


%Puntos_Moved_Red = Calc_Posiciones(alpha,beta);
% Cálculo de las distancias en función de los ángulos de cada sección.
for i=1:Num_Rotulas
Geomatriz(i,8,:)=Calc_Distancia(i,Puntos_Moved_Red); % 
end

% Sumatorio de las distancias totales de todos los cables.
% Con esto conseguimos sumar la longitud de cada trío de cables teniendo en
% cuenta en qué rótula empiezan.
for i=8:-1:1    % Para cada rótula empezando desde la pinza.
    for j=Geomatriz(i,2:4,1)   % Para cada cable en esa rótula.
        for k=1:i   %Sumamos la longitud de todas las rótulas.
            Longitudes_cables(j) = Longitudes_cables(j) + Geomatriz(k,8,j);
        end
    end
end

Desplazamiento_cables=Longitudes_cables-Longitudes_cables_0;
Pulsos_Destino=-round(Desplazamiento_cables*(Pulsos_por_rev/Paso_husillo).*Reductora_Motor);
clear alpha beta H i j k Longitudes_cables Longitudes_cables_0 Num_Agujeros Num_Rotulas Posicion_eje_y Puntos_Moved_Red R
end

