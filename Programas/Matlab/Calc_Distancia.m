function  Distances  = Calc_Distancia(Rotula,Puntos_Moved_Red)

Fixed = Puntos_Moved_Red(:,:,2*Rotula-1); % 

% Para cada agujero, calcula la posición de la placa móvil según h y los
% ángulos alpha1 y alpha2.

Moved = Puntos_Moved_Red(:,:,2*Rotula); % 

% Cálculo de las distancias entre cada par de agujeros mediante el cálculo
% de la distancias entre dos puntos (módulo).
Distances = sqrt(sum((Moved'-Fixed').^2));
%end

